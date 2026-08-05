/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2026 Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * runtime/sslib/anonymous.c
 * SurgeScript standard library (not quite, but there are common methods):
 * Anonymous objects (a.k.a. "Named Tuples" or "Immutable Anonymous Records")
 */

#include <stdbool.h>
#include <stdlib.h>
#include "../program_pool.h"
#include "../program.h"
#include "../object_manager.h"
#include "../heap.h"
#include "../../util/xxh.h"
#include "../../util/util.h"
#include "../../util/ssarray.h"

/* internal */
#define ANONYMOUS_OBJECT_NAME_PREFIX    "__Anonymous$"
#define ANONYMOUS_OBJECT_NAME_MAXLEN    31
#define METADATA_COUNT                  3    /* number of metadata entries */
#define METADATA_FIELD_LIST_SEPARATOR   '\n' /* never part of a field name (identifier) */
#define METADATA_ADDR_REMAINING_WRITES  ((surgescript_heapptr_t)0x0)
#define METADATA_ADDR_FIELD_COUNT       ((surgescript_heapptr_t)0x1)
#define METADATA_ADDR_FIELD_LIST        ((surgescript_heapptr_t)0x2)
#define FIELD_ADDRESS(i)                ((surgescript_heapptr_t)((i) + METADATA_COUNT)) /* i-th user field, i = 0, 1, 2... */

#undef U
#undef F
#define U(x)                            surgescript_program_operand_u(x)
#define F(x)                            surgescript_program_operand_f(x)
#define T0                              U(0)
#define T1                              U(1)
#define TEXT(text)                      U(surgescript_program_add_text(program, (text)))
#define SSASM(...)                      _SSASMX(__VA_ARGS__, _SSASM2, _SSASM1, _SSASM0)(__VA_ARGS__)

#define _SSASMX(_0, _1, _2, X, ...)     X
#define _SSASM2(op, a, b)               surgescript_program_add_line(program, (op), (a), (b))
#define _SSASM1(op, a)                  surgescript_program_add_line(program, (op), (a), U(0))
#define _SSASM0(op)                     surgescript_program_add_line(program, (op), U(0), U(0))

SS_STATIC_ASSERT(ANONYMOUS_OBJECT_NAME_MAXLEN <= SS_NAMEMAX);

static int count_vowels(const char* str);
static int count_non_vowels(const char* str);
static int count_field_names(const char** field_names);
static char* join_field_names(const char** field_names);
static char* generate_object_name(const char** field_names, char* out_object_name, size_t out_object_name_size);
static void create_main_state(surgescript_programpool_t* program_pool, const char* object_name);
static void create_file_getter(surgescript_programpool_t* program_pool, const char* object_name, const char* source_file);
static void create_constructor(surgescript_programpool_t* program_pool, const char* object_name, const char** field_names);
static void create_getter(surgescript_programpool_t* program_pool, const char* object_name, const char* field_name, int field_index);
static void create_native_methods(surgescript_programpool_t* program_pool, const char* object_name);

static surgescript_var_t* fun_init(surgescript_object_t* object, const surgescript_var_t* param[], int num_params);
static surgescript_var_t* fun_equals(surgescript_object_t* object, const surgescript_var_t* param[], int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t* param[], int num_params);




/*
 * surgescript_register_anonymous_object()
 * Create an object class for an anonymous object (this does not spawn anything)
 * Parameter field_names is a NULL-terminated array of strings
 * Output parameter out_object_name gives the name of the object
 * Returns true on success
 */
bool surgescript_register_anonymous_object(struct surgescript_programpool_t* program_pool, const char** field_names, const char* source_file, char* out_object_name, size_t out_object_name_size)
{
    char object_name[ANONYMOUS_OBJECT_NAME_MAXLEN + 1] = "";

    /* validate: must have 1 or more fields */
    if(field_names == NULL || *field_names == NULL)
        return false;

    /* generate a name for the anonymous object */
    generate_object_name(field_names, object_name, sizeof object_name);
    ssassert(*object_name != '\0');

    /* create the object class */
    if(!surgescript_programpool_exists(program_pool, object_name, "state:main")) {

        create_main_state(program_pool, object_name);
        create_file_getter(program_pool, object_name, source_file);
        create_constructor(program_pool, object_name, field_names);
        create_native_methods(program_pool, object_name);

        /* the order of the fields matters! */
        int field_index = 0;
        for(const char** it = field_names; *it != NULL; it++) {
            const char* field_name = *it;

            /* Why do we create only getters? Immutability! */
            create_getter(program_pool, object_name, field_name, field_index++);
        }

    }

    /* export the object name */
    ssassert(strlen(object_name) < out_object_name_size);
    surgescript_util_strncpy(out_object_name, object_name, out_object_name_size);

    /* done! */
    return true;
}




/* -------------------------------------------------------------------------- */

/* count the vowels in an ASCII string */
int count_vowels(const char* str)
{
    static const int table[128] = { ['a'] = 1, ['e'] = 1, ['i'] = 1, ['o'] = 1, ['u'] = 1, ['A'] = 1, ['E'] = 1, ['I'] = 1, ['O'] = 1, ['U'] = 1 };
    int count = 0;

    while(*str)
        count += table[*str++ & 127];

    return count;
}

/* count the non-vowels in an ASCII string */
int count_non_vowels(const char* str)
{
    return strlen(str) - count_vowels(str);
}

/* count the number of non-null elements of a NULL-terminated array of strings */
int count_field_names(const char** field_names)
{
    int field_count = 0;

    for(const char** it = field_names; *it != NULL; it++)
        field_count++;

    return field_count;
}

/* join field names as in field_1 <separator> field_2 <separator> field_3 <separator> ...
   you must ssfree() the result. field_names is a NULL-terminated array of strings */
char* join_field_names(const char** field_names)
{
    SSARRAY(char, buffer);
    ssarray_init(buffer);

    for(const char** it = field_names; *it != NULL; it++) {
        const char* field_name = *it;
        ssassert(*field_name != '\0'); /* validity check */
        for(const char* p = field_name; *p != '\0'; p++)
            ssarray_push(buffer, *p);
        ssarray_push(buffer, METADATA_FIELD_LIST_SEPARATOR);
    }
    ssarray_push(buffer, '\0');

    /* technically we could just return the buffer, but
       I prefer to call ssarray_release() for consistency */
    char* result = ssstrdup(buffer);
    ssarray_release(buffer);
    return result;
}

/* generate a name for the anonymous object */
char* generate_object_name(const char** field_names, char* out_object_name, size_t out_object_name_size)
{
    char object_name[ANONYMOUS_OBJECT_NAME_MAXLEN + 1] = ANONYMOUS_OBJECT_NAME_PREFIX "0000000000000000";
    xxhash_t ha = 0, hb = 0, seed = 0xCAFEC0DE * (xxhash_t)generate_object_name;

    /* we need fields to compute the hashes */
    ssassert(field_names && *field_names);

    /* preprocessing */
    int field_count = count_field_names(field_names);
    char* field_list = join_field_names(field_names);
    int field_list_length = strlen(field_list);

    /* compute first 32-bit hash */
    ha = XXH(field_list, field_list_length, seed);

    /* compute second 32-bit hash */
    uint32_t mix = 1u;
    for(const char** it = field_names; *it != NULL; it++)
        mix *= 3u + count_non_vowels(*it);
    uint32_t tmp[] = { mix, field_list_length, field_count };
    hb = XXH(tmp, sizeof tmp, seed);

    /* write both hashes to the suffix of the object name */
    const char prefix[] = ANONYMOUS_OBJECT_NAME_PREFIX;
    const size_t suffix_position = sizeof prefix - 1;
    char* suffix = object_name + suffix_position;
    snprintf(suffix, 16+1, "%08x%08x", (uint32_t)ha, (uint32_t)hb); /* probably unique */
    SS_STATIC_ASSERT(sizeof prefix + 16 < sizeof object_name);

    /* release */
    ssfree(field_list);

    /* done! */
    ssassert(strlen(object_name) < out_object_name_size);
    surgescript_util_strncpy(out_object_name, object_name, out_object_name_size);
    return out_object_name;
}

/* main state */
void create_main_state(surgescript_programpool_t* program_pool, const char* object_name)
{
    const char* program_name = "state:main";
    surgescript_program_t* program = surgescript_program_create(0);

    /* very fast way to return null */
    SSASM(SSOP_MOVN, T0);
    SSASM(SSOP_RET);

    surgescript_programpool_put(program_pool, object_name, program_name, program);
}

/* get___file() method */
void create_file_getter(surgescript_programpool_t* program_pool, const char* object_name, const char* source_file)
{
    const char* program_name = "get___file";
    surgescript_program_t* program = surgescript_program_create(0);

    /* very fast way to return source_file */
    SSASM(SSOP_MOVS, T0, TEXT(source_file));
    SSASM(SSOP_RET);

    surgescript_programpool_put(program_pool, object_name, program_name, program);

    /*

    This implementation is not entirely correct: different anonymous objects with
    the same shape (fields names and order) may be defined in different files.
    Thus, they will be mapped to the same class of objects, but this.__file will
    return only one of the files. A possible solution is to make the source file
    an instance member rather than a class member, leading to increased memory
    usage. However, as I write this, I do not see how this property can be useful
    in practice. Such extra complexity is hardly worth the trouble.

    */
}

/* get_<field_name>() method */
void create_getter(surgescript_programpool_t* program_pool, const char* object_name, const char* field_name, int field_index)
{
    char* program_name = surgescript_util_accessorfun("get", field_name);
    surgescript_program_t* program = surgescript_program_create(0);

    /* very fast way to return heap[field_address] */
    surgescript_heapptr_t field_address = FIELD_ADDRESS(field_index);
    SSASM(SSOP_PEEK, T0, U(field_address));
    SSASM(SSOP_RET);

    surgescript_programpool_put(program_pool, object_name, program_name, program);
    ssfree(program_name);
}

/* __ssconstructor() method */
void create_constructor(surgescript_programpool_t* program_pool, const char* object_name, const char** field_names)
{
    const char* program_name = "__ssconstructor";
    surgescript_program_t* program = surgescript_program_create(0);

    /* This implementation assumes that field_count memory cells are allocated
       CONTIGUOUSLY in the heap, starting at a predefined base address.
       Thus, mem[base + field_index] gives the right pointer to any field.
       Internal variables of objects make use of the same assumption;
       see emit_vardecl() in asm.c for details */

    /* preprocessing */
    int field_count = count_field_names(field_names);
    char* field_list = join_field_names(field_names);

    /* allocate memory for the metadata */
    for(int i = 0; i < METADATA_COUNT; i++)
        SSASM(SSOP_ALLOC);

    /* set up metadata */
    SSASM(SSOP_MOVS, T1, TEXT(field_list));
    SSASM(SSOP_MOVF, T0, F(field_count));
    SSASM(SSOP_POKE, T0, U(METADATA_ADDR_REMAINING_WRITES)); /* heap[METADATA_ADDR_REMAINING_WRITES] = field_count */
    SSASM(SSOP_POKE, T0, U(METADATA_ADDR_FIELD_COUNT)); /* heap[METADATA_ADDR_FIELD_COUNT] = field_count */
    SSASM(SSOP_POKE, T1, U(METADATA_ADDR_FIELD_LIST)); /* heap[METADATA_ADDR_FIELD_LIST] = field_list */

    /* end */
    SSASM(SSOP_MOVN, T0);
    SSASM(SSOP_RET); /* return null */

    /* done */
    ssfree(field_list);
    surgescript_programpool_put(program_pool, object_name, program_name, program);
}

/* register native methods */
void create_native_methods(surgescript_programpool_t* program_pool, const char* object_name)
{
    surgescript_programpool_put(program_pool, object_name, "__init", surgescript_program_create_native(1, fun_init));
    surgescript_programpool_put(program_pool, object_name, "equals", surgescript_program_create_native(1, fun_equals));
    surgescript_programpool_put(program_pool, object_name, "toString", surgescript_program_create_native(0, fun_tostring));
}

/* __init(value) method */
surgescript_var_t* fun_init(surgescript_object_t* object, const surgescript_var_t* param[], int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);

    /* ensure that this anonymous object remains immutable
       in case anyone calls this function manually */
    surgescript_var_t* var_remaining_writes = surgescript_heap_at(heap, METADATA_ADDR_REMAINING_WRITES);
    int remaining_writes = surgescript_var_get_number(var_remaining_writes);
    if(remaining_writes <= 0)
        return NULL;

    /* in the i-th call to this method, we'll initialize the
       i-th field of this anonymous object (i = 0, 1, 2,...)
       Note that the order of the fields matters! */
    int field_count = surgescript_var_get_number(surgescript_heap_at(heap, METADATA_ADDR_FIELD_COUNT));
    int field_index = field_count - remaining_writes;

    /* allocate memory for the field */
    surgescript_heapptr_t field_address = surgescript_heap_malloc(heap);
    ssassert(field_address == FIELD_ADDRESS(field_index));

    /* initialize the field */
    const surgescript_var_t* field_data = param[0];
    surgescript_var_t* dest = surgescript_heap_at(heap, field_address);
    surgescript_var_copy(dest, field_data);

    /* update remaining_writes */
    surgescript_var_set_number(var_remaining_writes, --remaining_writes);
    return NULL;
}

/* equals() method */
surgescript_var_t* fun_equals(surgescript_object_t* object, const surgescript_var_t* param[], int num_params)
{
    /* initialize the return value to false */
    surgescript_var_t* result = surgescript_var_set_bool(surgescript_var_create(), false);

    /* early exit? */
    if(!surgescript_var_is_objecthandle(param[0]))
        return result;

    /* preprocess */
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t other_handle = surgescript_var_get_objecthandle(param[0]);
    const surgescript_object_t* other = surgescript_objectmanager_get(manager, other_handle);
    const surgescript_object_t* self = object;

    /* if the classes differ, then the objects are different */
    if(surgescript_object_class_id(self) != surgescript_object_class_id(other))
        return result;

#if 1
    /* this shouldn't happen */
    ssassert(0 == strcmp(surgescript_object_name(self), surgescript_object_name(other)));
#endif

    /* now we know that both objects are anonymous */
    const surgescript_heap_t* self_heap = surgescript_object_heap(self);
    const surgescript_heap_t* other_heap = surgescript_object_heap(other);
    const surgescript_var_t* self_field_count_var = surgescript_heap_at(self_heap, METADATA_ADDR_FIELD_COUNT);
    const surgescript_var_t* other_field_count_var = surgescript_heap_at(other_heap, METADATA_ADDR_FIELD_COUNT);
    int self_field_count = surgescript_var_get_number(self_field_count_var);
    int other_field_count = surgescript_var_get_number(other_field_count_var);

    /* if the classes are the same, then the number of fields must be the same */
    ssassert(self_field_count == other_field_count);
    int field_count = self_field_count;

    /* let's compare each field individually */
    for(int field_index = 0; field_index < field_count; field_index++) {
        const surgescript_var_t* self_field_data = surgescript_heap_at(self_heap, FIELD_ADDRESS(field_index));
        const surgescript_var_t* other_field_data = surgescript_heap_at(other_heap, FIELD_ADDRESS(field_index));

        /* primitive types can be compared easily */
        if(!surgescript_var_is_objecthandle(self_field_data) && !surgescript_var_is_objecthandle(other_field_data)) {
            if(!surgescript_var_sametype(self_field_data, other_field_data) || 0 != surgescript_var_compare(self_field_data, other_field_data))
                return result;
            else
                continue;
        }

        /* we compare objects by calling .equals() */
        const surgescript_var_t* callee_var = surgescript_var_is_objecthandle(self_field_data) ? self_field_data : other_field_data;
        const surgescript_var_t* param_var = callee_var == other_field_data ? self_field_data : other_field_data;
        const surgescript_var_t* param[] = { param_var };
        surgescript_objecthandle_t callee_handle = surgescript_var_get_objecthandle(callee_var);
        surgescript_object_t* callee = surgescript_objectmanager_get(manager, callee_handle);

        /* sanity check */
        if(surgescript_object_function_arity(callee, "equals") != 1)
            return result;

        /* test for field equality */
        surgescript_object_call_function(callee, "equals", param, 1, result);
        if(surgescript_var_get_bool(result) == false) { /* here we convert the return value of equals() to a boolean */
            /* we set result = false again just in case caller.equals() doesn't return a bool - as it should! */
            return surgescript_var_set_bool(result, false);
        }
    }

    /* Now we know that the objects have the same fields, which were declared in
       the same order, and that those fields have the same content. Thus, the
       objects test true for equality. This is analogous to C#'s Anonymous Types */
    return surgescript_var_set_bool(result, true);
}

/* toString() method */
surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t* param[], int num_params)
{
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    const surgescript_heap_t* heap = surgescript_object_heap(object);
    const surgescript_var_t* field_list_var = surgescript_heap_at(heap, METADATA_ADDR_FIELD_LIST);
    const char* field_list = surgescript_var_fast_get_string(field_list_var);
    surgescript_var_t* result = surgescript_var_create();
    SSARRAY(char, sb); /* string builder */
    int field_index = 0;

    /* initialization */
    ssarray_init_ex(sb, 64);

    /* prefix */
    ssarray_push(sb, '{');
    ssarray_push(sb, ' ');

    /* for each field */
    for(const char* p = field_list; *p != '\0'; p++) {

        /* copy field name */
        if(*p != METADATA_FIELD_LIST_SEPARATOR) {
            ssarray_push(sb, *p);
            continue;
        }

        /* write equal sign */
        ssarray_push(sb, ' ');
        ssarray_push(sb, '=');
        ssarray_push(sb, ' ');

        /* write the data of the field */
        const surgescript_var_t* field_data = surgescript_heap_at(heap, FIELD_ADDRESS(field_index));
        char* field_stringified_data = surgescript_var_get_string(field_data, manager);
        {
            if(surgescript_var_is_string(field_data)) {
                /* surround the string with double quotes;
                   add backslashes if necessary */
                ssarray_push(sb, '"');
                for(const char* q = field_stringified_data; *q != '\0'; q++) {
                    if(*q == '"' || *q == '\\')
                        ssarray_push(sb, '\\');
                    ssarray_push(sb, *q);
                }
                ssarray_push(sb, '"');
            }
            else {
                /* copy the stringified field to sb */
                for(const char* q = field_stringified_data; *q != '\0'; q++)
                    ssarray_push(sb, *q);
            }
        }
        ssfree(field_stringified_data);
        field_index++;

        /* write a comma, but not a trailing comma */
        if(p[1] != '\0') {
            ssarray_push(sb, ',');
            ssarray_push(sb, ' ');
        }

    }

    /* suffix */
    ssarray_push(sb, ' ');
    ssarray_push(sb, '}');
    ssarray_push(sb, '\0');

    /* done! */
    surgescript_var_set_string(result, sb);
    ssarray_release(sb);
    return result;
}