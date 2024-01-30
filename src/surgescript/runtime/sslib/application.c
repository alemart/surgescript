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
 * runtime/sslib/application.c
 * SurgeScript standard library: routines for the Application object
 */

#include <stdio.h>
#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_exit(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_crash(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getargs(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_application()
 * Register methods
 */
void surgescript_sslib_register_application(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Application", "exit", fun_exit, 0);
    surgescript_vm_bind(vm, "Application", "crash", fun_crash, 1);
    surgescript_vm_bind(vm, "Application", "destroy", fun_destroy, 0); /* overloads Object's destroy() */
    surgescript_vm_bind(vm, "Application", "get_args", fun_getargs, 0);
}



/* my functions */

/* exits the app */
surgescript_var_t* fun_exit(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* this will destroy the root object and stop the VM */
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root_handle = surgescript_objectmanager_root(manager);
    surgescript_object_t* root = surgescript_objectmanager_get(manager, root_handle);
    surgescript_object_call_function(root, "exit", NULL, 0, NULL);
    surgescript_object_kill(object);
    return NULL;
}

/* destroys the app */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* this is the same as exiting the app */
    return fun_exit(object, param, num_params);
}

/* crashes the app with a message */
surgescript_var_t* fun_crash(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    char* text = surgescript_var_get_string(param[0], manager);
    ssfatal("Script Error: %s", text); /* change ssfatal to something else? */
    ssfree(text);
    return fun_exit(object, NULL, 0);
}

/* command line arguments */
surgescript_var_t* fun_getargs(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t this_handle = surgescript_object_handle(object);
    surgescript_objecthandle_t args_handle = surgescript_object_child(object, "Arguments");
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);

    /* lazy spawn */
    if(args_handle == null_handle)
        args_handle = surgescript_objectmanager_spawn(manager, this_handle, "Arguments", NULL);

    /* done! */
    return surgescript_var_set_objecthandle(surgescript_var_create(), args_handle);
}