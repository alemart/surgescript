/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2022 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/sslib/dictionary.c
 * SurgeScript standard library: Dictionary data structure
 */

#include <string.h>
#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../object_manager.h"
#include "../tag_system.h"
#include "../../util/ssarray.h"
#include "../../util/util.h"

/* private stuff */

/* Dictionary */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_clear(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_delete(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_has(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_keys(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_iterator(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* DictionaryIterator */
static surgescript_var_t* fun_it_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_next(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_hasnext(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* DictionaryEntry: useful for iterators */
static surgescript_var_t* fun_entry_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_entry_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_entry_getkey(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_entry_getvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_entry_setvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_entry_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* BSTNode: native implementation of a Binary Search Tree in SurgeScript */
static surgescript_var_t* fun_bst_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_getkey(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_getvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_getleft(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_getright(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_setvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_count(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_find(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_insert(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_bst_remove(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* addresses */
static const surgescript_heapptr_t BST_KEY = 0; /* address of the 'key' field in the BST */
static const surgescript_heapptr_t BST_VALUE = 1; /* and so on */
static const surgescript_heapptr_t BST_LEFT = 2;
static const surgescript_heapptr_t BST_RIGHT = 3;
static const surgescript_heapptr_t IT_ENTRYREF = 0;
static const surgescript_heapptr_t IT_STACKSIZE = 1;
static const surgescript_heapptr_t IT_STACKBASE = 2;
static const surgescript_heapptr_t ENTRY_BSTREF = 0;
static const surgescript_heapptr_t DICT_BSTROOT = 0;

/* utilities */
static surgescript_var_t* sanitize_key(surgescript_var_t* ssvar, const surgescript_objectmanager_t* manager);
static surgescript_objecthandle_t new_bst_node(const surgescript_object_t* parent, const surgescript_var_t* key, const surgescript_var_t* value);
static int bst_count(const surgescript_objectmanager_t* manager, const surgescript_object_t* object);
static surgescript_var_t* bst_remove(surgescript_object_t* object, const char* param_key, int depth);
static surgescript_var_t* bst_removeroot(surgescript_object_t* object);
static surgescript_var_t* dictentry(surgescript_object_t* entry, const surgescript_object_t* bst);

/*
 * surgescript_sslib_register_dictionary()
 * Register methods
 */
void surgescript_sslib_register_dictionary(surgescript_vm_t* vm)
{
    /* tags */
    surgescript_tagsystem_t* tag_system = surgescript_vm_tagsystem(vm);
    surgescript_tagsystem_add_tag(tag_system, "Dictionary", "iterable");
    surgescript_tagsystem_add_tag(tag_system, "DictionaryIterator", "iterator");

    /* methods */
    surgescript_vm_bind(vm, "Dictionary", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Dictionary", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Dictionary", "get_count", fun_getcount, 0);
    surgescript_vm_bind(vm, "Dictionary", "get", fun_get, 1);
    surgescript_vm_bind(vm, "Dictionary", "set", fun_set, 2);
    surgescript_vm_bind(vm, "Dictionary", "clear", fun_clear, 0);
    surgescript_vm_bind(vm, "Dictionary", "delete", fun_delete, 1);
    surgescript_vm_bind(vm, "Dictionary", "has", fun_has, 1);
    surgescript_vm_bind(vm, "Dictionary", "keys", fun_keys, 0);
    surgescript_vm_bind(vm, "Dictionary", "iterator", fun_iterator, 0);
    surgescript_vm_bind(vm, "Dictionary", "toString", fun_tostring, 0);

    surgescript_vm_bind(vm, "DictionaryIterator", "constructor", fun_it_constructor, 0);
    surgescript_vm_bind(vm, "DictionaryIterator", "state:main", fun_it_main, 0);
    surgescript_vm_bind(vm, "DictionaryIterator", "next", fun_it_next, 0);
    surgescript_vm_bind(vm, "DictionaryIterator", "hasNext", fun_it_hasnext, 0);
    surgescript_vm_bind(vm, "DictionaryIterator", "toString", fun_it_tostring, 0);

    surgescript_vm_bind(vm, "DictionaryEntry", "constructor", fun_entry_constructor, 0);
    surgescript_vm_bind(vm, "DictionaryEntry", "state:main", fun_entry_main, 0);
    surgescript_vm_bind(vm, "DictionaryEntry", "get_key", fun_entry_getkey, 0);
    surgescript_vm_bind(vm, "DictionaryEntry", "get_value", fun_entry_getvalue, 0);
    surgescript_vm_bind(vm, "DictionaryEntry", "set_value", fun_entry_setvalue, 1);
    surgescript_vm_bind(vm, "DictionaryEntry", "toString", fun_entry_tostring, 0);

    surgescript_vm_bind(vm, "BSTNode", "constructor", fun_bst_constructor, 0);
    surgescript_vm_bind(vm, "BSTNode", "state:main", fun_bst_main, 0);
    surgescript_vm_bind(vm, "BSTNode", "get_key", fun_bst_getkey, 0);
    surgescript_vm_bind(vm, "BSTNode", "get_value", fun_bst_getvalue, 0);
    surgescript_vm_bind(vm, "BSTNode", "set_value", fun_bst_setvalue, 1);
    surgescript_vm_bind(vm, "BSTNode", "get_left", fun_bst_getleft, 0);
    surgescript_vm_bind(vm, "BSTNode", "get_right", fun_bst_getright, 0);
    surgescript_vm_bind(vm, "BSTNode", "count", fun_bst_count, 0);
    surgescript_vm_bind(vm, "BSTNode", "find", fun_bst_find, 1);
    surgescript_vm_bind(vm, "BSTNode", "insert", fun_bst_insert, 2);
    surgescript_vm_bind(vm, "BSTNode", "remove", fun_bst_remove, 1);
}



/* --- Dictionary --- */

/* A Dictionary is just a facade that implements a Binary Search Tree (BSTNodes) */

/* constructor(): initialize the Dictionary */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);

    ssassert(DICT_BSTROOT == surgescript_heap_malloc(heap));
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, DICT_BSTROOT), null_handle);

    return NULL;
}

/* main state: do nothing */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

/* getCount(): how many entries does this Dictionary have? */
surgescript_var_t* fun_getcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t bst = surgescript_var_get_objecthandle(surgescript_heap_at(heap, DICT_BSTROOT));

    if(surgescript_objectmanager_exists(manager, bst)) {
        surgescript_object_t* node = surgescript_objectmanager_get(manager, bst);
        return fun_bst_count(node, NULL, 0);
    }
    else
        return surgescript_var_set_number(surgescript_var_create(), 0);
}

/* get(key): gets an entry from the Dictionary */
surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_var_t* get = NULL;
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t bst = surgescript_var_get_objecthandle(surgescript_heap_at(heap, DICT_BSTROOT));

    if(surgescript_objectmanager_exists(manager, bst)) {
        surgescript_object_t* node = surgescript_objectmanager_get(manager, bst);
        surgescript_var_t* key = sanitize_key(surgescript_var_clone(param[0]), manager);
        const surgescript_var_t* p[] = { key };
        surgescript_var_t* result = fun_bst_find(node, p, 1);
        surgescript_objecthandle_t result_handle = surgescript_var_get_objecthandle(result);

        if(surgescript_objectmanager_exists(manager, result_handle)) {
            surgescript_object_t* result_object = surgescript_objectmanager_get(manager, result_handle);
            get = fun_bst_getvalue(result_object, NULL, 0);
        }

        surgescript_var_destroy(result);
        surgescript_var_destroy(key);
    }

    return get;
}

/* set(key, value): sets a new entry */
surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_var_t* root = surgescript_heap_at(heap, DICT_BSTROOT);
    surgescript_objecthandle_t bst = surgescript_var_get_objecthandle(root);
    surgescript_var_t* key = sanitize_key(surgescript_var_clone(param[0]), manager); /* keys must be sanitized */
    const surgescript_var_t* value = param[1];

    if(surgescript_objectmanager_exists(manager, bst)) { /* if there is a root... */
        surgescript_object_t* node = surgescript_objectmanager_get(manager, bst);
        const surgescript_var_t* p[] = { key, value };
        surgescript_var_t* result = fun_bst_insert(node, p, 2);
        surgescript_var_destroy(result);
    }
    else /* if not, create one */
        surgescript_var_set_objecthandle(root, new_bst_node(object, key, value));

    surgescript_var_destroy(key);
    return NULL;
}

/* clear(): clears the whole Dictionary, so that no entries are stored */
surgescript_var_t* fun_clear(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);
    surgescript_var_t* root = surgescript_heap_at(heap, DICT_BSTROOT);
    surgescript_objecthandle_t bst = surgescript_var_get_objecthandle(root);

    if(surgescript_objectmanager_exists(manager, bst)) {
        surgescript_object_t* node = surgescript_objectmanager_get(manager, bst);
        surgescript_object_kill(node);
    }

    surgescript_var_set_objecthandle(root, null_handle);
    return NULL;
}

/* delete(key): deletes a key from the Dictionary */
surgescript_var_t* fun_delete(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_var_t* root = surgescript_heap_at(heap, DICT_BSTROOT);
    surgescript_objecthandle_t bst = surgescript_var_get_objecthandle(root);

    if(surgescript_objectmanager_exists(manager, bst)) {
        surgescript_object_t* node = surgescript_objectmanager_get(manager, bst);
        surgescript_var_t* key = sanitize_key(surgescript_var_clone(param[0]), manager);
        const surgescript_var_t* p[] = { key };

        surgescript_var_t* new_root = fun_bst_remove(node, p, 1);
        if(new_root != NULL) {
            surgescript_var_copy(root, new_root);
            surgescript_var_destroy(new_root);
        }

        surgescript_var_destroy(key);
    }

    return NULL;
}

/* has(key): does this dictionary have an entry with the given key? */
surgescript_var_t* fun_has(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    bool has = false;
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t bst = surgescript_var_get_objecthandle(surgescript_heap_at(heap, DICT_BSTROOT));

    if(surgescript_objectmanager_exists(manager, bst)) {
        surgescript_object_t* node = surgescript_objectmanager_get(manager, bst);
        surgescript_var_t* key = sanitize_key(surgescript_var_clone(param[0]), manager);
        const surgescript_var_t* p[] = { key };
        surgescript_var_t* result = fun_bst_find(node, p, 1);
        surgescript_objecthandle_t result_handle = surgescript_var_get_objecthandle(result);

        has = surgescript_objectmanager_exists(manager, result_handle);

        surgescript_var_destroy(result);
        surgescript_var_destroy(key);
    }

    return surgescript_var_set_bool(surgescript_var_create(), has);
}

/* iterator(): spawns an iterator of this Dictionary */
surgescript_var_t* fun_iterator(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* The DictionaryIterator will set up itself */
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t it_handle = surgescript_objectmanager_spawn(manager, surgescript_object_handle(object), "DictionaryIterator", NULL);
    return surgescript_var_set_objecthandle(surgescript_var_create(), it_handle);
}

/* toString(): converts to string */
surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_var_t* stringified_dictionary = surgescript_var_create();
    surgescript_object_t* iterator = NULL;
    SSARRAY(char, sb); /* string builder */
    static int depth = 0;
    bool can_descend = (++depth < 16); /* handle circular links */

    /* helper macros */
    #define WRITE_ELEMENT(element, write_as_quoted_string) \
        do { \
            char* value = surgescript_var_get_string((element), can_descend ? manager : NULL); \
            if(write_as_quoted_string) { \
                ssarray_push(sb, '"'); \
                for(const char* p = value; *p; p++) { \
                    switch(*p) { \
                        case '\n': ssarray_push(sb, '\\'); ssarray_push(sb, 'n'); break; \
                        case '\r': ssarray_push(sb, '\\'); ssarray_push(sb, 'r'); break; \
                        case '\t': ssarray_push(sb, '\\'); ssarray_push(sb, 't'); break; \
                        case '\f': ssarray_push(sb, '\\'); ssarray_push(sb, 'f'); break; \
                        case '\v': ssarray_push(sb, '\\'); ssarray_push(sb, 'v'); break; \
                        case '\b': ssarray_push(sb, '\\'); ssarray_push(sb, 'b'); break; \
                        case '\"': ssarray_push(sb, '\\'); ssarray_push(sb, '"'); break; \
                        default: ssarray_push(sb, *p); break; \
                    } \
                } \
                ssarray_push(sb, '"'); \
            } \
            else { \
                for(const char* p = value; *p; p++) \
                    ssarray_push(sb, *p); \
            } \
            ssfree(value); \
        } while(0) \

    /* start sb */
    ssarray_init(sb);
    ssarray_push(sb, '{');

    /* iterate through the Dictionary */
    do {
        surgescript_object_t* entry;
        surgescript_var_t* tmp = surgescript_var_create();
        surgescript_object_call_function(object, "iterator", NULL, 0, tmp),
        iterator = surgescript_objectmanager_get(manager, surgescript_var_get_objecthandle(tmp));
        while(surgescript_object_call_function(iterator, "hasNext", NULL, 0, tmp), surgescript_var_get_bool(tmp)) {
            /* get entry */
            surgescript_object_call_function(iterator, "next", NULL, 0, tmp);
            entry = surgescript_objectmanager_get(manager, surgescript_var_get_objecthandle(tmp));

            /* add whitespace */
            ssarray_push(sb, ' ');

            /* write key */
            surgescript_object_call_function(entry, "get_key", NULL, 0, tmp);
            WRITE_ELEMENT(tmp, true);
            ssarray_push(sb, ':');
            ssarray_push(sb, ' ');

            /* write value */
            surgescript_object_call_function(entry, "get_value", NULL, 0, tmp);
            if(surgescript_var_is_objecthandle(tmp)) {
                surgescript_objecthandle_t handle = surgescript_var_get_objecthandle(tmp);
                surgescript_object_t* object = surgescript_objectmanager_get(manager, handle);
                if(strcmp(surgescript_object_name(object), "Array") != 0 && strcmp(surgescript_object_name(object), "Dictionary") != 0) {
                    if(can_descend)
                        surgescript_object_call_function(object, "toString", NULL, 0, tmp);
                    WRITE_ELEMENT(tmp, strcmp(surgescript_var_fast_get_string(tmp), "[object]"));
                }
                else
                    WRITE_ELEMENT(tmp, false);
            }
            else
                WRITE_ELEMENT(tmp, surgescript_var_is_string(tmp));

            /* add separator */
            if(!(surgescript_object_call_function(iterator, "hasNext", NULL, 0, tmp), surgescript_var_get_bool(tmp))) {
                ssarray_push(sb, ' ');
                break;
            }
            else
                ssarray_push(sb, ',');
        }
        surgescript_var_destroy(tmp);
    } while(0);

    /* convert sb to string */
    ssarray_push(sb, '}');
    ssarray_push(sb, '\0');
    surgescript_var_set_string(stringified_dictionary, sb);
    ssarray_release(sb);
    --depth;

    /* done! */
    return stringified_dictionary;
}

/* keys(): returns an array containing the keys of the dictionary */
surgescript_var_t* fun_keys(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t array_handle = surgescript_objectmanager_spawn_array(manager);
    surgescript_object_t* array = surgescript_objectmanager_get(manager, array_handle);
    surgescript_object_t* iterator = NULL;
    surgescript_var_t* tmp = surgescript_var_create();
    const surgescript_var_t* p[] = { tmp };

    /* iterate through the Dictionary */
    surgescript_object_call_function(object, "iterator", NULL, 0, tmp),
    iterator = surgescript_objectmanager_get(manager, surgescript_var_get_objecthandle(tmp));
    while(surgescript_object_call_function(iterator, "hasNext", NULL, 0, tmp), surgescript_var_get_bool(tmp)) {
        surgescript_object_t* entry = NULL;
        surgescript_object_call_function(iterator, "next", NULL, 0, tmp);
        entry = surgescript_objectmanager_get(manager, surgescript_var_get_objecthandle(tmp));
        surgescript_object_call_function(entry, "get_key", NULL, 0, tmp);
        surgescript_object_call_function(array, "push", p, 1, NULL);
    }

    /* done! */
    return surgescript_var_set_objecthandle(tmp, array_handle);
}




/* --- DictionaryIterator --- */

/* constructor(): DictionaryIterator must be spawned by Dictionary */
surgescript_var_t* fun_it_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t parent_handle = surgescript_object_parent(object);
    surgescript_object_t* parent = surgescript_objectmanager_get(manager, parent_handle);
    surgescript_heap_t* parent_heap = surgescript_object_heap(parent);
    surgescript_objecthandle_t bst = surgescript_var_get_objecthandle(surgescript_heap_at(parent_heap, DICT_BSTROOT));
    surgescript_objecthandle_t this_handle = surgescript_object_handle(object);
    surgescript_objecthandle_t entry_handle = surgescript_objectmanager_spawn(manager, this_handle, "DictionaryEntry", NULL);
    const char* parent_name = surgescript_object_name(parent);

    ssassert(IT_ENTRYREF == surgescript_heap_malloc(heap));
    ssassert(IT_STACKSIZE == surgescript_heap_malloc(heap)); /* this can't represent 2^24+1 ~ 16.77 M */
    ssassert(IT_STACKBASE == surgescript_heap_malloc(heap));

    surgescript_var_set_objecthandle(surgescript_heap_at(heap, IT_ENTRYREF), entry_handle);
    if(surgescript_objectmanager_exists(manager, bst) && 0 == strcmp(parent_name, "Dictionary")) {
        surgescript_var_set_number(surgescript_heap_at(heap, IT_STACKSIZE), 1.0);
        surgescript_var_set_objecthandle(surgescript_heap_at(heap, IT_STACKBASE), bst);
    }
    else {
        surgescript_var_set_number(surgescript_heap_at(heap, IT_STACKSIZE), 0.0);
        surgescript_var_set_objecthandle(surgescript_heap_at(heap, IT_STACKBASE), surgescript_objectmanager_null(manager));
    }

    return NULL;
}

/* "main" state */
surgescript_var_t* fun_it_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* next(): advances the iterator and returns the item previously pointed to by the iterator */
surgescript_var_t* fun_it_next(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_var_t* stacksize = surgescript_heap_at(heap, IT_STACKSIZE);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);

    if(surgescript_var_get_number(stacksize) > 0) {
        surgescript_var_t* stacktop = surgescript_heap_at(heap, IT_STACKBASE + (surgescript_var_get_number(stacksize) - 1));
        surgescript_object_t* node = surgescript_objectmanager_get(manager, surgescript_var_get_objecthandle(stacktop));
        surgescript_heap_t* node_heap = surgescript_object_heap(node);
        surgescript_objecthandle_t left_handle, right_handle;
        surgescript_var_t* new_top;
        surgescript_heapptr_t top_ptr;
        surgescript_objecthandle_t entry_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, IT_ENTRYREF));
        surgescript_object_t* entry = surgescript_objectmanager_get(manager, entry_handle);

        /* pop stacktop */
        surgescript_var_set_number(stacksize, surgescript_var_get_number(stacksize) - 1);

        /* push right child */
        right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(node_heap, BST_RIGHT));
        if(surgescript_objectmanager_exists(manager, right_handle)) {
            top_ptr = IT_STACKBASE + surgescript_var_get_number(stacksize);
            new_top = surgescript_heap_at(heap, top_ptr);
            surgescript_var_set_objecthandle(new_top, right_handle);
            surgescript_var_set_number(stacksize, surgescript_var_get_number(stacksize) + 1);
        }

        /* push left child */
        left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(node_heap, BST_LEFT));
        if(surgescript_objectmanager_exists(manager, left_handle)) {
            top_ptr = IT_STACKBASE + surgescript_var_get_number(stacksize);
            if(!surgescript_heap_validaddress(heap, top_ptr))
                ssassert(top_ptr == surgescript_heap_malloc(heap));
            new_top = surgescript_heap_at(heap, top_ptr);
            surgescript_var_set_objecthandle(new_top, left_handle);
            surgescript_var_set_number(stacksize, surgescript_var_get_number(stacksize) + 1);
        }

        /* return previously pointed item */
        /*return fun_bst_getkey(node, NULL, 0);*/
        return dictentry(entry, node);
    }

    return NULL;
}

/* hasNext(): returns true if the iterator has NOT reached the end of the collection */
surgescript_var_t* fun_it_hasnext(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_var_t* stacksize = surgescript_heap_at(heap, IT_STACKSIZE);
    return surgescript_var_set_bool(surgescript_var_create(), surgescript_var_get_number(stacksize) > 0);
}

/* toString(): converts to string */
surgescript_var_t* fun_it_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_string(surgescript_var_create(), "[DictionaryIterator]");
}


/* --- DictionaryEntry functions --- */
surgescript_var_t* fun_entry_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);
    ssassert(ENTRY_BSTREF == surgescript_heap_malloc(heap));
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, ENTRY_BSTREF), null_handle);
    return NULL;
}

surgescript_var_t* fun_entry_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

surgescript_var_t* fun_entry_getkey(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t entry_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, ENTRY_BSTREF));
    surgescript_object_t* entry = surgescript_objectmanager_get(manager, entry_handle);

    return fun_bst_getkey(entry, NULL, 0);
}

surgescript_var_t* fun_entry_getvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t entry_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, ENTRY_BSTREF));
    surgescript_object_t* entry = surgescript_objectmanager_get(manager, entry_handle);

    return fun_bst_getvalue(entry, NULL, 0);
}

surgescript_var_t* fun_entry_setvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t entry_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, ENTRY_BSTREF));
    surgescript_object_t* entry = surgescript_objectmanager_get(manager, entry_handle);
    const surgescript_var_t* p[] = { param[0] };

    return fun_bst_setvalue(entry, p, 1);
}

surgescript_var_t* fun_entry_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_string(surgescript_var_create(), "[DictionaryEntry]");
}


/* --- BSTNode functions --- */

/* constructor: initialize data */
surgescript_var_t* fun_bst_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);

    ssassert(BST_KEY == surgescript_heap_malloc(heap));
    ssassert(BST_VALUE == surgescript_heap_malloc(heap));
    ssassert(BST_LEFT == surgescript_heap_malloc(heap));
    ssassert(BST_RIGHT == surgescript_heap_malloc(heap));

    surgescript_var_set_string(surgescript_heap_at(heap, BST_KEY), "[undefined]");
    surgescript_var_set_null(surgescript_heap_at(heap, BST_VALUE));
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_LEFT), null_handle);
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_RIGHT), null_handle);

    return NULL;
}

/* main state */
surgescript_var_t* fun_bst_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_set_active(object, false); /* optimization; we shouldn't spend time updating this object */
    return NULL;
}

/* get the key */
surgescript_var_t* fun_bst_getkey(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, BST_KEY));
}

/* get the value */
surgescript_var_t* fun_bst_getvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, BST_VALUE));
}

/* get the left node */
surgescript_var_t* fun_bst_getleft(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, BST_LEFT));
}

/* get the right node */
surgescript_var_t* fun_bst_getright(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, BST_RIGHT));
}

/* set the value; param[0] can be of any type */
surgescript_var_t* fun_bst_setvalue(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_var_copy(surgescript_heap_at(heap, BST_VALUE), param[0]);
    return NULL;
}

/* count the number of nodes */
surgescript_var_t* fun_bst_count(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    return surgescript_var_set_number(surgescript_var_create(), bst_count(manager, object));
}

/* finds a node having a given key, param[0] */
surgescript_var_t* fun_bst_find(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_LEFT));
    surgescript_objecthandle_t right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_RIGHT));
    const char* key = surgescript_var_fast_get_string(surgescript_heap_at(heap, BST_KEY));
    const char* search_key = surgescript_var_fast_get_string(param[0]);
    int cmp = strcmp(search_key, key);

    if(cmp == 0) /* found it */
        return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
    else if(cmp < 0 && surgescript_objectmanager_exists(manager, left_handle)) /* maybe it's on the left */
        return fun_bst_find(surgescript_objectmanager_get(manager, left_handle), param, num_params);
    else if(cmp > 0 && surgescript_objectmanager_exists(manager, right_handle)) /* maybe it's on the right */
        return fun_bst_find(surgescript_objectmanager_get(manager, right_handle), param, num_params);
    else /* key not found */
        return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_objectmanager_null(manager));
}

/* inserts a (key, value) pair into the BST and returns the new BSTNode; key = param[0] must be a string, value = param[1] can be of any type */
surgescript_var_t* fun_bst_insert(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_LEFT));
    surgescript_objecthandle_t right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_RIGHT));
    const char* key = surgescript_var_fast_get_string(surgescript_heap_at(heap, BST_KEY));
    const char* new_key = surgescript_var_fast_get_string(param[0]); /* thus param[0] must be a string */
    int cmp = strcmp(new_key, key);

    if(cmp == 0) {
        /* the key was already in the BST */
        surgescript_var_copy(surgescript_heap_at(heap, BST_VALUE), param[1]);
        return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
    }
    else if(cmp < 0) {
        if(!surgescript_objectmanager_exists(manager, left_handle)) {
            /* insert on the left */
            surgescript_objecthandle_t new_node = new_bst_node(object, param[0], param[1]);
            surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_LEFT), new_node);
            return surgescript_var_set_objecthandle(surgescript_var_create(), new_node);
        }
        else
            return fun_bst_insert(surgescript_objectmanager_get(manager, left_handle), param, num_params);
    }
    else {
        if(!surgescript_objectmanager_exists(manager, right_handle)) {
            /* insert on the right */
            surgescript_objecthandle_t new_node = new_bst_node(object, param[0], param[1]);
            surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_RIGHT), new_node);
            return surgescript_var_set_objecthandle(surgescript_var_create(), new_node);
        }
        else
            return fun_bst_insert(surgescript_objectmanager_get(manager, right_handle), param, num_params);
    }
}

/* removes an entry (key = param[0]) from the BST, and returns the root of the modified BST */
surgescript_var_t* fun_bst_remove(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    char* key = surgescript_var_get_string(param[0], manager);
    surgescript_var_t* new_root = bst_remove(object, key, 0);
    ssfree(key);
    return new_root;
}



/* --- Utilities --- */

/* transforms ssvar into a string; returns ssvar */
surgescript_var_t* sanitize_key(surgescript_var_t* ssvar, const surgescript_objectmanager_t* manager)
{
    char* buf = surgescript_var_get_string(ssvar, manager);
    surgescript_var_set_string(ssvar, buf);
    ssfree(buf);
    return ssvar;
}

/* spawns a new BSTNode */
surgescript_objecthandle_t new_bst_node(const surgescript_object_t* parent, const surgescript_var_t* key, const surgescript_var_t* value)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(parent);
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);
    surgescript_objecthandle_t new_node = surgescript_objectmanager_spawn(manager, surgescript_object_handle(parent), "BSTNode", NULL);
    surgescript_object_t* new_obj = surgescript_objectmanager_get(manager, new_node);
    surgescript_heap_t* heap = surgescript_object_heap(new_obj);

    surgescript_var_copy(surgescript_heap_at(heap, BST_KEY), key); /* key must be a string */
    surgescript_var_copy(surgescript_heap_at(heap, BST_VALUE), value); /* value can be of any type */
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_LEFT), null_handle);
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, BST_RIGHT), null_handle);

    return new_node;
}

/* counts the number of nodes in a BST */
int bst_count(const surgescript_objectmanager_t* manager, const surgescript_object_t* object)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objecthandle_t left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_LEFT));
    surgescript_objecthandle_t right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_RIGHT));
    int count = 1;

    if(surgescript_objectmanager_exists(manager, left_handle))
        count += bst_count(manager, surgescript_objectmanager_get(manager, left_handle));
    if(surgescript_objectmanager_exists(manager, right_handle))
        count += bst_count(manager, surgescript_objectmanager_get(manager, right_handle));

    return count;
}

/* removes the root of the BST and returns a handle to the new root. Performs any required modifications. */
surgescript_var_t* bst_removeroot(surgescript_object_t* object)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_LEFT));

    if(!surgescript_objectmanager_exists(manager, left_handle)) {
        surgescript_objecthandle_t right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_RIGHT));
        surgescript_object_kill(object);
        return surgescript_var_set_objecthandle(surgescript_var_create(), right_handle);
    }
    else {
        surgescript_object_t* node = object;
        surgescript_heap_t* node_heap = heap;
        surgescript_objecthandle_t child_handle = left_handle;
        surgescript_object_t* child = surgescript_objectmanager_get(manager, child_handle);
        surgescript_heap_t* child_heap = surgescript_object_heap(child);
        surgescript_objecthandle_t grand_child = surgescript_var_get_objecthandle(surgescript_heap_at(child_heap, BST_RIGHT));

        /* the right-most guy (that is to the left of the root) will be the new root */
        while(surgescript_objectmanager_exists(manager, grand_child)) {
            node = child;
            node_heap = surgescript_object_heap(node);
            child_handle = grand_child;
            child = surgescript_objectmanager_get(manager, child_handle);
            child_heap = surgescript_object_heap(child);
            grand_child = surgescript_var_get_objecthandle(surgescript_heap_at(child_heap, BST_RIGHT));
        }

        if(node != object) {
            surgescript_var_copy(surgescript_heap_at(node_heap, BST_RIGHT), surgescript_heap_at(child_heap, BST_LEFT));
            surgescript_var_copy(surgescript_heap_at(child_heap, BST_LEFT), surgescript_heap_at(heap, BST_LEFT));
            surgescript_var_copy(surgescript_heap_at(child_heap, BST_RIGHT), surgescript_heap_at(heap, BST_RIGHT));
        }
        else
            surgescript_var_copy(surgescript_heap_at(child_heap, BST_RIGHT), surgescript_heap_at(heap, BST_RIGHT));

        surgescript_object_kill(object);
        return surgescript_var_set_objecthandle(surgescript_var_create(), child_handle);
    }
}



/* removes a node from the BST; returns the root of the modified BST or NULL if the key isn't found */
surgescript_var_t* bst_remove(surgescript_object_t* object, const char* param_key, int depth)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t left_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_LEFT));
    surgescript_objecthandle_t right_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, BST_RIGHT));
    const char* key = surgescript_var_fast_get_string(surgescript_heap_at(heap, BST_KEY));
    int cmp = strcmp(param_key, key);

    /*printf("bst_remove(%s, %s) d=%d e cmp=%d\n", param_key, key, depth, cmp);*/

    if(cmp == 0) {
        surgescript_var_t* new_root = bst_removeroot(object);
        return new_root;
    }
    else if((cmp < 0 && surgescript_objectmanager_exists(manager, left_handle)) || (cmp > 0 && surgescript_objectmanager_exists(manager, right_handle))) {
        surgescript_objecthandle_t child_handle = (cmp < 0) ? left_handle : right_handle;
        surgescript_object_t* child = surgescript_objectmanager_get(manager, child_handle);
        surgescript_heap_t* child_heap = surgescript_object_heap(child);
        const char* child_key = surgescript_var_fast_get_string(surgescript_heap_at(child_heap, BST_KEY));

        if(0 == strcmp(param_key, child_key)) {
            /*surgescript_var_t* new_root = bst_remove(child, param_key, depth + 1);*/
            surgescript_var_t* new_root = bst_removeroot(child);
            surgescript_var_copy(surgescript_heap_at(heap, (cmp < 0) ? BST_LEFT : BST_RIGHT), new_root);
            surgescript_var_destroy(new_root);
        }
        else {
            surgescript_var_t* new_root = bst_remove(child, param_key, depth + 1);
            if(new_root != NULL)
                surgescript_var_destroy(new_root);
        }

        return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object)); /* return myself */
    }
    else
        return NULL; /* key not found */
}

/* given a DictionaryEntry, set its BST Node Reference and return a new variable with the handle to the entry */
surgescript_var_t* dictentry(surgescript_object_t* entry, const surgescript_object_t* bst)
{
    surgescript_heap_t* heap = surgescript_object_heap(entry);
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, ENTRY_BSTREF), surgescript_object_handle(bst));
    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(entry));
}
