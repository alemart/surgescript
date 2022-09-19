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
 * runtime/sslib/surgescript.c
 * SurgeScript standard library: SurgeScript object
 */

#include "../vm.h"
#include "../object.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getversion(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/*
 * surgescript_sslib_register_surgescript()
 * Register methods
 */
void surgescript_sslib_register_surgescript(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "SurgeScript", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "SurgeScript", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "SurgeScript", "get_version", fun_getversion, 0);
    surgescript_vm_bind(vm, "SurgeScript", "state:main", fun_main, 0);
}



/* my functions */

/* destroy function */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* spawn function */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing; you can't spawn children on this object */
    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_set_active(object, false); /* we don't need to spend time updating this object */
    return NULL;
}
 
/* SurgeScript version */
surgescript_var_t* fun_getversion(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_string(surgescript_var_create(), surgescript_util_version());
}