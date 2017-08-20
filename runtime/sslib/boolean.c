/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/boolean.c
 * SurgeScript standard library: routines for the Boolean object
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "../vm.h"
#include "../object.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_valueof(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_call(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_plus(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_boolean()
 * Register methods
 */
void surgescript_sslib_register_boolean(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Boolean", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Boolean", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Boolean", "valueOf", fun_valueof, 1);
    surgescript_vm_bind(vm, "Boolean", "toString", fun_tostring, 1);
    surgescript_vm_bind(vm, "Boolean", "call", fun_call, 1);
    surgescript_vm_bind(vm, "Boolean", "plus", fun_plus, 2);
    surgescript_vm_bind(vm, "Boolean", "get", fun_get, 2);
    surgescript_vm_bind(vm, "Boolean", "set", fun_set, 3);
}



/* my functions */

/* returns my primitive */
surgescript_var_t* fun_valueof(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_bool(surgescript_var_create(), surgescript_var_get_bool(param[0]));
}

/* converts to string */
surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_string(surgescript_var_create(), surgescript_var_get_bool(param[0]) ? "true" : "false");
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

/* call: type conversion */
surgescript_var_t* fun_call(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return fun_valueof(object, param, num_params);
}

/* plus function: adds two booleans (or two somethings) */
surgescript_var_t* fun_plus(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    int code[] = {
        surgescript_var_type2code("number"),
        surgescript_var_type2code("boolean"),
        surgescript_var_type2code(NULL)
    };

    if(( /* if the second operand is either a number, a boolean or a null ... */
        !surgescript_var_typecheck(param[1], code[0]) ||
        !surgescript_var_typecheck(param[1], code[1]) ||
        !surgescript_var_typecheck(param[1], code[2])
    ) && !surgescript_var_typecheck(param[0], code[1])) { /* the first operand is assumed to be a boolean */
        float x = surgescript_var_get_number(param[0]);
        float y = surgescript_var_get_number(param[1]);
        return surgescript_var_set_number(surgescript_var_create(), x + y);
    }
    else {
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
}

/* get */
surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

/* set */
surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* booleans are primitive values in SurgeScript */
    /* this is an invalid operation; do nothing */
    return surgescript_var_clone(param[2]);
}