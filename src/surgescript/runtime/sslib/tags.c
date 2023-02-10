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
 * runtime/sslib/tags.c
 * SurgeScript standard library: Tag System
 */

#include "../vm.h"
#include "../object.h"
#include "../object_manager.h"
#include "../tag_system.h"
#include "../../util/util.h"

/* API */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_list(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_select(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tagsof(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* private stuff */
static void array_push(const char* string, void* arr);

/*
 * surgescript_sslib_register_tagsystem()
 * Register methods
 */
void surgescript_sslib_register_tagsystem(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "__TagSystem", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "__TagSystem", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "__TagSystem", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "__TagSystem", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "__TagSystem", "list", fun_list, 0);
    surgescript_vm_bind(vm, "__TagSystem", "select", fun_select, 1);
    surgescript_vm_bind(vm, "__TagSystem", "tagsOf", fun_tagsof, 1);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_set_active(object, false); /* we don't need to spend time updating this object */
    return NULL;
}

/* spawn */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* can't spawn anything here! */
    return NULL;
}

/* destroy */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* can't destroy this object! */
    return NULL;
}

/* list(): returns an array of strings with all the tags that have been registered */
surgescript_var_t* fun_list(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    surgescript_objecthandle_t array_handle = surgescript_objectmanager_spawn_array(object_manager);
    surgescript_object_t* array = surgescript_objectmanager_get(object_manager, array_handle);
    surgescript_tagsystem_t* tag_system = surgescript_objectmanager_tagsystem(object_manager);

    surgescript_tagsystem_foreach_tag(tag_system, array, array_push);

    return surgescript_var_set_objecthandle(surgescript_var_create(), array_handle);
}

/* select(tag): returns an array of strings with all the names of the objects tagged tag */
surgescript_var_t* fun_select(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    surgescript_objecthandle_t array_handle = surgescript_objectmanager_spawn_array(object_manager);
    surgescript_object_t* array = surgescript_objectmanager_get(object_manager, array_handle);
    surgescript_tagsystem_t* tag_system = surgescript_objectmanager_tagsystem(object_manager);
    char* tag_name = surgescript_var_get_string(param[0], object_manager);

    surgescript_tagsystem_foreach_tagged_object(tag_system, tag_name, array, array_push);

    ssfree(tag_name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), array_handle);
}

/* tagsOf(objectName): returns an array of strings with all the tags of the objects named objectName */
surgescript_var_t* fun_tagsof(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    surgescript_objecthandle_t array_handle = surgescript_objectmanager_spawn_array(object_manager);
    surgescript_object_t* array = surgescript_objectmanager_get(object_manager, array_handle);
    surgescript_tagsystem_t* tag_system = surgescript_objectmanager_tagsystem(object_manager);
    char* object_name = surgescript_var_get_string(param[0], object_manager);

    surgescript_tagsystem_foreach_tag_of_object(tag_system, object_name, array, array_push);

    ssfree(object_name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), array_handle);
}



/* --- private stuff --- */

/* add a string to a SurgeScript array */
void array_push(const char* string, void* arr)
{
    surgescript_object_t* array = (surgescript_object_t*)arr;
    surgescript_var_t* tmp = surgescript_var_create();
    const surgescript_var_t* param[] = { tmp };

    surgescript_var_set_string(tmp, string);
    surgescript_object_call_function(array, "push", param, 1, NULL);

    surgescript_var_destroy(tmp);
}
