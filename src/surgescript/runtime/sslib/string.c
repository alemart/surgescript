/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2024 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/sslib/string.c
 * SurgeScript standard library: routines for the String object
 */

#include <string.h>
#include <ctype.h>
#include "../vm.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/ssarray.h"
#include "../../util/util.h"
#include "../../third_party/utf8.h"

/* private stuff */
static surgescript_var_t* fun_valueof(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_equals(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_call(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getlength(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_indexof(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_substr(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_concat(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_replace(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tolowercase(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_touppercase(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_isnullorempty(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_string()
 * Register methods
 */
void surgescript_sslib_register_string(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "String", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "String", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "String", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "String", "valueOf", fun_valueof, 1);
    surgescript_vm_bind(vm, "String", "toString", fun_tostring, 1);
    surgescript_vm_bind(vm, "String", "equals", fun_equals, 2);
    surgescript_vm_bind(vm, "String", "call", fun_call, 1);
    surgescript_vm_bind(vm, "String", "get_length", fun_getlength, 1);
    surgescript_vm_bind(vm, "String", "get", fun_get, 2);
    surgescript_vm_bind(vm, "String", "set", fun_set, 3);
    surgescript_vm_bind(vm, "String", "indexOf", fun_indexof, 2);
    surgescript_vm_bind(vm, "String", "substr", fun_substr, 3);
    surgescript_vm_bind(vm, "String", "concat", fun_concat, 2);
    surgescript_vm_bind(vm, "String", "replace", fun_replace, 3);
    surgescript_vm_bind(vm, "String", "toLowerCase", fun_tolowercase, 1);
    surgescript_vm_bind(vm, "String", "toUpperCase", fun_touppercase, 1);
    surgescript_vm_bind(vm, "String", "isNullOrEmpty", fun_isnullorempty, 1);
}



/* my functions */

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_set_active(object, false); /* we don't need to spend time updating this object */
    return NULL;
}

/* destroy */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing, as system objects cannot be destroyed */
    return NULL;
}

/* spawn */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing; you can't spawn children on this object */
    return NULL;
}

/* returns my primitive */
surgescript_var_t* fun_valueof(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* str = surgescript_var_fast_get_string(param[0]); /* param[0] can be assumed to be a string, for sure */
    surgescript_var_t* ret = surgescript_var_set_string(surgescript_var_create(), str);
    return ret;
}

/* converts to string */
surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* str = surgescript_var_fast_get_string(param[0]);
    surgescript_var_t* ret = surgescript_var_set_string(surgescript_var_create(), str);
    return ret;
}

/* equals() method */
surgescript_var_t* fun_equals(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    if(surgescript_var_sametype(param[0], param[1])) {
        const char* a = surgescript_var_fast_get_string(param[0]); /* can be assumed to be a string, for sure */
        const char* b = surgescript_var_fast_get_string(param[1]); /* it's a string, since param[0] is a string */
        return surgescript_var_set_bool(surgescript_var_create(), strcmp(a, b) == 0);
    }
    else
        return surgescript_var_set_bool(surgescript_var_create(), false);
}

/* call: type conversion */
surgescript_var_t* fun_call(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    char* str = surgescript_var_get_string(param[0], manager);
    surgescript_var_t* ret = surgescript_var_set_string(surgescript_var_create(), str);
    ssfree(str);
    return ret;
}

/* length of the string */
surgescript_var_t* fun_getlength(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* str = surgescript_var_fast_get_string(param[0]);
    return surgescript_var_set_number(surgescript_var_create(), u8_strlen(str));
}

/* character at */
surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* str = surgescript_var_fast_get_string(param[0]);
    int index = (int)surgescript_var_get_number(param[1]);
    char chr[7] = { 0 };

    if(index >= 0 && index < u8_strlen(str)) {
        size_t offset = u8_offset(str, index);
        size_t seq_len = u8_seqlen(str + offset);
        for(int i = 0; i < sizeof(chr) - 1 && seq_len--; i++)
            chr[i] = str[offset + i];
    }

    return surgescript_var_set_string(surgescript_var_create(), chr);
}

/* set a character */
surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* strings are immutable in SurgeScript */
    return surgescript_var_clone(param[0]);
}

/* finds the first occurence of param[1] in the string param[0] */
surgescript_var_t* fun_indexof(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    const char* haystack = surgescript_var_fast_get_string(param[0]);
    char* needle = surgescript_var_get_string(param[1], manager);
    char* occurrence = strstr(haystack, needle);
    int indexof = occurrence ? u8_charnum((char*)haystack, occurrence - haystack) : -1; /* all SurgeScript strings are UTF-8 encoded */
    ssfree(needle);
    return surgescript_var_set_number(surgescript_var_create(), indexof);
}

/* returns a substring beginning at param[1] having length param[2] */
surgescript_var_t* fun_substr(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* str = surgescript_var_fast_get_string(param[0]), *begin, *end;
    int start = surgescript_var_get_number(param[1]);
    int length = surgescript_var_get_number(param[2]);
    surgescript_var_t* var = surgescript_var_create();
    size_t utf8len = u8_strlen(str);
    char* substr;

    /* sanity check */
    if(start < 0)
        start = (int)utf8len - (-start % (int)utf8len);
    else if(start > (int)utf8len)
        start = (int)utf8len;
    length = ssclamp(length, 0, (int)utf8len - start);

    /* extract the substring */
    begin = str + u8_offset((char*)str, start);
    end = str + u8_offset((char*)str, start + length);
    ssassert(end >= begin);
    substr = ssmalloc((2 + end - begin) * sizeof(*substr));
    surgescript_util_strncpy(substr, begin, 1 + end - begin);

    /* done! */
    surgescript_var_set_string(var, substr);
    ssfree(substr);
    return var;
}

/* concatenates two strings */
surgescript_var_t* fun_concat(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_var_t* ret = surgescript_var_create();
    char* str[] = { NULL, NULL };
    char* buf = NULL;

    str[0] = surgescript_var_get_string(param[0], manager);
    str[1] = surgescript_var_get_string(param[1], manager);

    buf = ssmalloc((1 + strlen(str[0]) + strlen(str[1])) * sizeof(*buf));
    surgescript_var_set_string(ret, strcat(strcpy(buf, str[0]), str[1]));
    ssfree(buf);

    ssfree(str[1]);
    ssfree(str[0]);

    return ret;
}

/* replaces param[1] by param[2] in param[0] */
surgescript_var_t* fun_replace(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    const char* subject = surgescript_var_fast_get_string(param[0]);
    char* search = surgescript_var_get_string(param[1], manager);
    char* replace = surgescript_var_get_string(param[2], manager);
    int search_len = strlen(search);
    const char *loc, *p, *q;
    surgescript_var_t* result = surgescript_var_create();
    SSARRAY(char, sb); /* string builder */
    ssarray_init(sb);

    p = subject;
    if(search_len > 0) {
        while((loc = strstr(p, search)) != NULL) {
            while(p != loc && *p)
                ssarray_push(sb, *p++);
            for(q = replace; *q;)
                ssarray_push(sb, *q++);
            p += search_len;
        }
    }
    while(*p)
        ssarray_push(sb, *p++);
    ssarray_push(sb, '\0');

    surgescript_var_set_string(result, sb);
    ssarray_release(sb);
    ssfree(replace);
    ssfree(search);

    return result;
}

/* convert string to lower case characters */
surgescript_var_t* fun_tolowercase(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* src = surgescript_var_fast_get_string(param[0]), *p;
    char* dst = ssmalloc((1 + strlen(src)) * sizeof(*dst)), *q;
    surgescript_var_t* result = surgescript_var_create();

    for(p = src, q = dst; *p;)
        *q++ = tolower(*p++);
    *q = '\0';

    surgescript_var_set_string(result, dst);
    ssfree(dst);
    return result;
}

/* convert string to upper case characters */
surgescript_var_t* fun_touppercase(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* src = surgescript_var_fast_get_string(param[0]), *p;
    char* dst = ssmalloc((1 + strlen(src)) * sizeof(*dst)), *q;
    surgescript_var_t* result = surgescript_var_create();

    for(p = src, q = dst; *p;)
        *q++ = toupper(*p++);
    *q = '\0';

    surgescript_var_set_string(result, dst);
    ssfree(dst);
    return result;
}

/* returns true if the string is null or empty */
surgescript_var_t* fun_isnullorempty(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_bool(surgescript_var_create(),
        surgescript_var_is_null(param[0]) ||
        (surgescript_var_is_string(param[0]) && ('\0' == *(surgescript_var_fast_get_string(param[0]))))
    );
}
