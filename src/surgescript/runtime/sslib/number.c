/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2023 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/sslib/number.c
 * SurgeScript standard library: routines for the Number object
 */

#include <math.h>
#include <float.h>
#include "../vm.h"
#include "../object.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_valueof(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_equals(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_call(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_isfinite(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_isnan(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_isinteger(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_number()
 * Register methods
 */
void surgescript_sslib_register_number(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Number", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Number", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Number", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Number", "valueOf", fun_valueof, 1);
    surgescript_vm_bind(vm, "Number", "toString", fun_tostring, 1);
    surgescript_vm_bind(vm, "Number", "equals", fun_equals, 2);
    surgescript_vm_bind(vm, "Number", "call", fun_call, 1);
    surgescript_vm_bind(vm, "Number", "get", fun_get, 2);
    surgescript_vm_bind(vm, "Number", "set", fun_set, 3);
    surgescript_vm_bind(vm, "Number", "isFinite", fun_isfinite, 1);
    surgescript_vm_bind(vm, "Number", "isNaN", fun_isnan, 1);
    surgescript_vm_bind(vm, "Number", "isInteger", fun_isinteger, 1);
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
    char* buf = surgescript_var_get_string(param[0], surgescript_object_manager(object));
    surgescript_var_t* ret = surgescript_var_set_string(surgescript_var_create(), buf);
    ssfree(buf);
    return ret;
}

/* equals() method */
surgescript_var_t* fun_equals(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* tip for users: use Math.approximately() instead */
    if(surgescript_var_sametype(param[0], param[1])) {
        double a = surgescript_var_get_number(param[0]);
        double b = surgescript_var_get_number(param[1]);
        double ma = fabs(a), mb = fabs(b);
        return surgescript_var_set_bool(surgescript_var_create(), (a == b) || fabs(a - b) <= ssmax(ma, mb) * FLT_EPSILON);
    }
    else
        return surgescript_var_set_bool(surgescript_var_create(), false);
}

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

/* call: type conversion */
surgescript_var_t* fun_call(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return fun_valueof(object, param, num_params);
}

/* get */
surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

/* set */
surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* numbers are primitive values in SurgeScript */
    /* this is an invalid operation; do nothing */
    return surgescript_var_clone(param[2]);
}

/* is the number finite? */
surgescript_var_t* fun_isfinite(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double x = surgescript_var_get_number(param[0]);
    return surgescript_var_set_bool(surgescript_var_create(), isfinite(x));
}

/* is the value NaN? */
surgescript_var_t* fun_isnan(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double x = surgescript_var_get_number(param[0]);
    return surgescript_var_set_bool(surgescript_var_create(), isnan(x));
}

/* is the number an integer? */
surgescript_var_t* fun_isinteger(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double x = surgescript_var_get_number(param[0]);
    return surgescript_var_set_bool(surgescript_var_create(), isfinite(x) && x == ceil(x));
}