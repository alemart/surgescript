/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/string.c
 * SurgeScript standard library: routines for the String object
 */

#include <stdio.h>
#include <string.h>
#include "../vm.h"
#include "../object.h"
#include "../../util/util.h"
#include "../../util/utf8.h"

/* private stuff */
static surgescript_var_t* fun_valueof(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_call(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tonumber(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_length(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_indexof(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_substr(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_concat(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_string()
 * Register methods
 */
void surgescript_sslib_register_string(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "String", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "String", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "String", "valueOf", fun_valueof, 1);
    surgescript_vm_bind(vm, "String", "toString", fun_tostring, 1);
    surgescript_vm_bind(vm, "String", "call", fun_call, 1);
    surgescript_vm_bind(vm, "String", "toNumber", fun_tonumber, 1);
    surgescript_vm_bind(vm, "String", "length", fun_length, 1);
    surgescript_vm_bind(vm, "String", "get", fun_get, 2);
    surgescript_vm_bind(vm, "String", "set", fun_set, 3);
    surgescript_vm_bind(vm, "String", "indexOf", fun_indexof, 2);
    surgescript_vm_bind(vm, "String", "substr", fun_substr, 3);
    surgescript_vm_bind(vm, "String", "concat", fun_concat, 2);
}



/* my functions */

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* destroy */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing, as system objects cannot be destroyed */
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

/* call: type conversion */
surgescript_var_t* fun_call(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* str = surgescript_var_get_string(param[0]);
    surgescript_var_t* ret = surgescript_var_set_string(surgescript_var_create(), str);
    ssfree(str);
    return ret;
}

/* converts to number */
surgescript_var_t* fun_tonumber(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), surgescript_var_get_number(param[0]));
}

/* length of the string */
surgescript_var_t* fun_length(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* str = surgescript_var_fast_get_string(param[0]);
    size_t count = 0;
    countCodePoints((uint8_t*)str, &count);
    return surgescript_var_set_number(surgescript_var_create(), count);
}

/* character at */
surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char chr[5] = { 0, 0, 0, 0, 0 };
    const char* str = surgescript_var_fast_get_string(param[0]);
    size_t index = surgescript_var_get_number(param[1]);
    uint32_t widechar = codePointAt((uint8_t*)str, index);

    if(widechar) /* valid utf-8 string (up to index) */
        u8_wc_toutf8(chr, widechar); /* this does not put the ending '\0' */
    
    return surgescript_var_set_string(surgescript_var_create(), chr);
}

/* set a character */
surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* strings are primitive values in SurgeScript */
    /* this is an invalid operation; do nothing */
    return surgescript_var_clone(param[2]);
}

/* finds the first occurence of param[1] in the string param[0] */
surgescript_var_t* fun_indexof(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* haystack = surgescript_var_fast_get_string(param[0]);
    char* needle = surgescript_var_get_string(param[1]);
    char* occurrence = strstr(haystack, needle);
    int indexof = occurrence ? u8_charnum((char*)haystack, occurrence - haystack) : -1; /* all SurgeScript strings are UTF-8 encoded */
    ssfree(needle);
    return surgescript_var_set_number(surgescript_var_create(), indexof);
}

/* returns a substring beginning at param[1] having length param[2] */
surgescript_var_t* fun_substr(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* substr;
    const char* str = surgescript_var_fast_get_string(param[0]), *begin, *end;
    int start = surgescript_var_get_number(param[1]);
    int length = surgescript_var_get_number(param[2]);
    surgescript_var_t* var = surgescript_var_create();
    size_t utf8len = 0;

    /* sanity check */
    countCodePoints((uint8_t*)str, &utf8len);
    start = ssclamp(start, 0, (int)utf8len);
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
    surgescript_var_t* ret = surgescript_var_create();
    const char* str1 = surgescript_var_fast_get_string(param[0]); /* param[0] can be assumed to be a string */
    char* str2 = surgescript_var_get_string(param[1]);
    char* str = ssmalloc((1 + strlen(str1) + strlen(str2)) * sizeof(*str));
    surgescript_var_set_string(ret, strcat(strcpy(str, str1), str2));
    ssfree(str);
    ssfree(str2);
    return ret;
}