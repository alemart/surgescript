/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/number.c
 * SurgeScript standard library: routines for the Number object
 */

#include <stdio.h>
#include <string.h>
#include "../vm.h"
#include "../object.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_valueof(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_concat(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_number()
 * Register methods
 */
void surgescript_sslib_register_number(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Number", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Number", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Number", "valueOf", fun_valueof, 1);
    surgescript_vm_bind(vm, "Number", "toString", fun_tostring, 1);
    surgescript_vm_bind(vm, "Number", "concat", fun_concat, 2);
}



/* my functions */

/* returns my primitive */
surgescript_var_t* fun_valueof(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), surgescript_var_get_number(param[0]));
}

/* converts to string */
surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* buf = surgescript_var_get_string(param[0]);
    surgescript_var_t* ret = surgescript_var_set_string(surgescript_var_create(), buf);
    ssfree(buf);
    return ret;
}

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

/* converts to and concatenates two strings */
surgescript_var_t* fun_concat(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_var_t* ret = surgescript_var_create();
    char* str1 = surgescript_var_get_string(param[0]);
    char* str2 = surgescript_var_get_string(param[1]);
    char* str = ssmalloc((1 + strlen(str1) + strlen(str2)) * sizeof(*str));
    surgescript_var_set_string(ret, strcat(strcpy(str, str1), str2));
    ssfree(str);
    ssfree(str2);
    ssfree(str1);
    return ret;
}