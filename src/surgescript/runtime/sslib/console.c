/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2018 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/sslib/console.c
 * SurgeScript standard library: Console
 */

#include <stdio.h>
#include <string.h>
#include "../vm.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_print(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_write(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_readline(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_console()
 * Register methods
 */
void surgescript_sslib_register_console(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Console", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Console", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Console", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Console", "print", fun_print, 1);
    surgescript_vm_bind(vm, "Console", "write", fun_write, 1);
    surgescript_vm_bind(vm, "Console", "readline", fun_readline, 0);
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

/* print a line to stdout */
surgescript_var_t* fun_print(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    char* str = surgescript_var_get_string(param[0], manager);
    puts(str);
    ssfree(str);
    return NULL;
}

/* write a string to stdout */
surgescript_var_t* fun_write(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    char* str = surgescript_var_get_string(param[0], manager);
    fputs(str, stdout);
    fflush(stdout);
    ssfree(str);
    return NULL;
}

/* read a line from stdin */
surgescript_var_t* fun_readline(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char str[1024] = "";
    char* result = fgets(str, sizeof(str) / sizeof(char), stdin);

    if(result != NULL && !ferror(stdin)) {
        if(!feof(stdin)) {
            int len = strlen(result);
            if(len > 0)
                result[len - 1] = 0;
        }
        return surgescript_var_set_string(surgescript_var_create(), result);
    }

    return NULL;
}