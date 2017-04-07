/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/string.c
 * SurgeScript standard library: routines for the String object
 */

#include <stdio.h>
#include <stdbool.h>
#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_valueof(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tonumber(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_string()
 * Register methods
 */
void surgescript_sslib_register_string(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "String", "valueOf", fun_valueof, 1);
    surgescript_vm_bind(vm, "String", "toString", fun_tostring, 1);
    surgescript_vm_bind(vm, "String", "toNumber", fun_tonumber, 1);
    surgescript_vm_bind(vm, "String", "state:main", fun_main, 0);
}



/* my functions */

/* returns my primitive */
surgescript_var_t* fun_valueof(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* buf = surgescript_var_get_string(param[0]);
    surgescript_var_t* ret = surgescript_var_set_string(surgescript_var_create(), buf);
    ssfree(buf);
    return ret;
}

/* converts to string */
surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* buf = surgescript_var_get_string(param[0]);
    surgescript_var_t* ret = surgescript_var_set_string(surgescript_var_create(), buf);
    ssfree(buf);
    return ret;
}

/* converts to number */
surgescript_var_t* fun_tonumber(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), surgescript_var_get_number(param[0]));
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}