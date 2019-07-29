/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2019 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/sslib/object.c
 * SurgeScript standard library: common routines for all objects
 */

#include <ctype.h>
#include <string.h>
#include "../vm.h"
#include "../object.h"
#include "../object_manager.h"
#include "../program_pool.h"
#include "../tag_system.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_name(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_parent(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_child(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_children(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_childcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_sibling(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_equals(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_hasfunction(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_findobject(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_findobjects(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_findobjectwithtag(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_findobjectswithtag(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_hastag(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_timeout(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_functions(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_timespent(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_memspent(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_childlist(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getactive(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setactive(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_invoke(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_arity(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_file(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_assert(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* utilities */
static void add_to_descendants_array(surgescript_objecthandle_t handle, void* arr);
static void add_to_function_array(const char* fun_name, void* arr);
static bool is_visible_function(const char* fun_name);
static bool can_spawn_object(const char* object_name, surgescript_objectmanager_t* manager);


/*
 * surgescript_sslib_register_object()
 * Register common methods to all objects
 */
void surgescript_sslib_register_object(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Object", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Object", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Object", "get_parent", fun_parent, 0);
    surgescript_vm_bind(vm, "Object", "child", fun_child, 1);
    surgescript_vm_bind(vm, "Object", "children", fun_children, 1);
    surgescript_vm_bind(vm, "Object", "get_childCount", fun_childcount, 0);
    surgescript_vm_bind(vm, "Object", "findObject", fun_findobject, 1);
    surgescript_vm_bind(vm, "Object", "findObjects", fun_findobjects, 1);
    surgescript_vm_bind(vm, "Object", "findObjectWithTag", fun_findobjectwithtag, 1);
    surgescript_vm_bind(vm, "Object", "findObjectsWithTag", fun_findobjectswithtag, 1);
    surgescript_vm_bind(vm, "Object", "sibling", fun_sibling, 1);
    surgescript_vm_bind(vm, "Object", "toString", fun_tostring, 0);
    surgescript_vm_bind(vm, "Object", "equals", fun_equals, 1);
    surgescript_vm_bind(vm, "Object", "hasFunction", fun_hasfunction, 1);
    surgescript_vm_bind(vm, "Object", "hasTag", fun_hastag, 1);
    surgescript_vm_bind(vm, "Object", "__timeout", fun_timeout, 1);
    surgescript_vm_bind(vm, "Object", "__invoke", fun_invoke, 2);
    surgescript_vm_bind(vm, "Object", "__arity", fun_arity, 1);
    surgescript_vm_bind(vm, "Object", "__assert", fun_assert, 3);
    surgescript_vm_bind(vm, "Object", "get___name", fun_name, 0);
    surgescript_vm_bind(vm, "Object", "get___active", fun_getactive, 0);
    surgescript_vm_bind(vm, "Object", "set___active", fun_setactive, 1);
    surgescript_vm_bind(vm, "Object", "get___functions", fun_functions, 0);
    surgescript_vm_bind(vm, "Object", "get___children", fun_childlist, 0);
    surgescript_vm_bind(vm, "Object", "get___timespent", fun_timespent, 0);
    surgescript_vm_bind(vm, "Object", "get___memspent", fun_memspent, 0);
    surgescript_vm_bind(vm, "Object", "get___file", fun_file, 0);
}



/* my functions */

/* returns a handle to the parent object */
surgescript_var_t* fun_parent(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objecthandle_t parent = surgescript_object_parent(object);
    return surgescript_var_set_objecthandle(surgescript_var_create(), parent);
}

/* returns a handle to a child named param[0] (or a handle to null if not found)
 * it also accepts a child ID (integer between 0 and childCount - 1, inclusive) */
surgescript_var_t* fun_child(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    if(surgescript_var_is_number(param[0])) {
        int id = (int)surgescript_var_get_number(param[0]);
        surgescript_objecthandle_t child = surgescript_object_nth_child(object, id);
        return surgescript_var_set_objecthandle(surgescript_var_create(), child);
    }
    else {
        const char* name = surgescript_var_fast_get_string(param[0]);
        surgescript_objecthandle_t child = surgescript_object_child(object, name);
        return surgescript_var_set_objecthandle(surgescript_var_create(), child);
    }
}

/* number of children */
surgescript_var_t* fun_childcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), surgescript_object_child_count(object));
}

/* returns a handle to a sibling named param[0] (or null) */
surgescript_var_t* fun_sibling(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* name = surgescript_var_fast_get_string(param[0]);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t parent_handle = surgescript_object_parent(object);
    surgescript_object_t* parent = surgescript_objectmanager_get(manager, parent_handle);
    surgescript_objecthandle_t sibling = surgescript_object_child(parent, name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), sibling);
}

/* spawns a new array containing handles to all direct children named param[0] */
surgescript_var_t* fun_children(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* name = surgescript_var_fast_get_string(param[0]);
    surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    surgescript_objecthandle_t array_handle = surgescript_objectmanager_spawn_array(object_manager);
    surgescript_object_t* array = surgescript_objectmanager_get(object_manager, array_handle);
    surgescript_object_children(object, name, array, add_to_descendants_array);
    return surgescript_var_set_objecthandle(surgescript_var_create(), array_handle);
}

/* finds a descendant (child, or grand-child, or grand-grand-child, and so on) named param[0] */
surgescript_var_t* fun_findobject(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* name = surgescript_var_fast_get_string(param[0]);
    surgescript_objecthandle_t child = surgescript_object_find_child(object, name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), child);
}

/* finds all descendants named param[0], returning a new array */
surgescript_var_t* fun_findobjects(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* name = surgescript_var_fast_get_string(param[0]);
    surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    surgescript_objecthandle_t array_handle = surgescript_objectmanager_spawn_array(object_manager);
    surgescript_object_t* array = surgescript_objectmanager_get(object_manager, array_handle);
    surgescript_object_find_children(object, name, array, add_to_descendants_array);
    return surgescript_var_set_objecthandle(surgescript_var_create(), array_handle);
}

/* finds a descendant tagged param[0] */
surgescript_var_t* fun_findobjectwithtag(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* tag_name = surgescript_var_fast_get_string(param[0]);
    surgescript_objecthandle_t child = surgescript_object_find_tagged_child(object, tag_name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), child);
}

/* finds all descendants tagged param[0], returning a new array */
surgescript_var_t* fun_findobjectswithtag(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* tag_name = surgescript_var_fast_get_string(param[0]);
    surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    surgescript_objecthandle_t array_handle = surgescript_objectmanager_spawn_array(object_manager);
    surgescript_object_t* array = surgescript_objectmanager_get(object_manager, array_handle);
    surgescript_object_find_tagged_children(object, tag_name, array, add_to_descendants_array);
    return surgescript_var_set_objecthandle(surgescript_var_create(), array_handle);
}

/* spawns a child */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* child_name = surgescript_var_fast_get_string(param[0]);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    if(can_spawn_object(child_name, manager)) {
        surgescript_objecthandle_t me = surgescript_object_handle(object);
        surgescript_objecthandle_t child = surgescript_objectmanager_spawn(manager, me, child_name, NULL);
        return surgescript_var_set_objecthandle(surgescript_var_create(), child);
    }
    else {
        const char* object_name = surgescript_object_name(object);
        ssfatal("Runtime Error: object \"%s\" can't spawn \"%s\".", object_name, child_name);
        return NULL;
    }
}

/* destroys the object */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_kill(object);
    return NULL;
}

/* toString() method */
surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* object_name = surgescript_object_name(object);
    char* str = ssmalloc((strlen(object_name) + 3) * sizeof(*str));
    surgescript_var_t* value = surgescript_var_create();

    strcat(strcat(strcpy(str, "["), object_name), "]");
    surgescript_var_set_string(value, str);
    ssfree(str);

    return value;
}

/* equals() method */
surgescript_var_t* fun_equals(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    if(surgescript_var_is_objecthandle(param[0])) {
        surgescript_objecthandle_t me = surgescript_object_handle(object);
        surgescript_objecthandle_t other = surgescript_var_get_objecthandle(param[0]);
        return surgescript_var_set_bool(surgescript_var_create(), me == other);
    }
    else
        return surgescript_var_set_bool(surgescript_var_create(), false);
}

/* what's the name of this object? */
surgescript_var_t* fun_name(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_string(surgescript_var_create(), surgescript_object_name(object));
}

/* does this object have a member function called param[0] ? */
surgescript_var_t* fun_hasfunction(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    const char* object_name = surgescript_object_name(object);
    char* program_name = surgescript_var_get_string(param[0], object_manager);
    bool exists = surgescript_programpool_exists(surgescript_objectmanager_programpool(object_manager), object_name, program_name);
    ssfree(program_name);
    return surgescript_var_set_bool(surgescript_var_create(), exists);
}

/* is this object tagged param[0] ? */
surgescript_var_t* fun_hastag(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    const char* object_name = surgescript_object_name(object);
    const char* tag_name = surgescript_var_fast_get_string(param[0]);
    bool tagged = surgescript_tagsystem_has_tag(surgescript_objectmanager_tagsystem(object_manager), object_name, tag_name);
    return surgescript_var_set_bool(surgescript_var_create(), tagged);
}

/* returns true iff the object has been on the same state for param[0] seconds or more */
surgescript_var_t* fun_timeout(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    double elapsed = surgescript_object_elapsed_time(object);
    double threshold = surgescript_var_get_number(param[0]);
    return surgescript_var_set_bool(surgescript_var_create(), elapsed >= threshold);
}

/* returns an Array containing the list of functions of this object */
surgescript_var_t* fun_functions(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    surgescript_programpool_t* program_pool = surgescript_objectmanager_programpool(object_manager);
    surgescript_objecthandle_t array_handle = surgescript_objectmanager_spawn_array(object_manager);
    surgescript_object_t* array = surgescript_objectmanager_get(object_manager, array_handle);

    surgescript_programpool_foreach_ex(program_pool, surgescript_object_name(object), array, add_to_function_array);
    surgescript_programpool_foreach_ex(program_pool, "Object", array, add_to_function_array);

    return surgescript_var_set_objecthandle(surgescript_var_create(), array_handle);
}

/* approximate time spent on this object, in seconds */
surgescript_var_t* fun_timespent(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), surgescript_object_timespent(object));
}

/* approximate memory used by this object (in VM space), in bytes */
surgescript_var_t* fun_memspent(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_number(surgescript_var_create(), surgescript_object_memspent(object));
}

/* returns an Array containing handles to my children */
surgescript_var_t* fun_childlist(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    surgescript_objecthandle_t array_handle = surgescript_objectmanager_spawn_array(object_manager);
    surgescript_object_t* array = surgescript_objectmanager_get(object_manager, array_handle);
    surgescript_var_t* tmp = surgescript_var_create();
    const surgescript_var_t* p[] = { tmp };
    int child_count = surgescript_object_child_count(object);

    for(int i = 0; i < child_count; i++) {
        surgescript_var_set_objecthandle(tmp, surgescript_object_nth_child(object, i));
        surgescript_object_call_function(array, "push", p, 1, NULL);
    }

    return surgescript_var_set_objecthandle(tmp, array_handle);
}

/* is this object active? */
surgescript_var_t* fun_getactive(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_bool(surgescript_var_create(), surgescript_object_is_active(object));
}

/* activates or deactivates this object (set active to true or false) */
surgescript_var_t* fun_setactive(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_set_active(object, surgescript_var_get_bool(param[0]));
    return NULL;
}

/* returns the source file of this object */
surgescript_var_t* fun_file(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* subclass responsibility */
    return NULL;
}

/* invokes a function, given by string param[0], with parameters specified in Array param[1] */
surgescript_var_t* fun_invoke(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_programpool_t* pool = surgescript_objectmanager_programpool(manager);
    const char* object_name = surgescript_object_name(object);
    char* program_name = surgescript_var_get_string(param[0], manager);
    surgescript_program_t* program = surgescript_programpool_get(pool, object_name, program_name);
    surgescript_var_t* ret = surgescript_var_create();

    /* Does the invoked function exist? */
    if(program != NULL) {
        /* Does the second argument (parameters array) exist? */
        surgescript_objecthandle_t params_handle = surgescript_var_get_objecthandle(param[1]);
        if(surgescript_objectmanager_exists(manager, params_handle)) {
            /* Is the second argument an Array? */
            surgescript_object_t* params_array = surgescript_objectmanager_get(manager, params_handle);
            if(strcmp(surgescript_object_name(params_array), "Array") == 0) {
                /* Does it have the same arity as the length of the input array? */
                int arity = surgescript_program_arity(program);
                surgescript_object_call_function(params_array, "get_length", NULL, 0, ret);
                if((int)surgescript_var_get_number(ret) == arity) {
                    /* Obtain the parameters */
                    surgescript_var_t** params_native_array = ssmalloc(arity * sizeof(*params_native_array));
                    for(int i = 0; i < arity; i++) {
                        const surgescript_var_t* p[] = { surgescript_var_set_number(ret, i) };
                        surgescript_object_call_function(params_array, "get", p, 1, ret);
                        params_native_array[i] = surgescript_var_clone(ret);
                    }

                    /* Invoke the desired function */
                    surgescript_object_call_function(object, program_name, (const surgescript_var_t**)params_native_array, arity, ret);

                    /* Release the parameters */
                    while(arity--)
                        surgescript_var_destroy(params_native_array[arity]);
                    ssfree(params_native_array);
                }
                else {
                    surgescript_var_set_null(ret);
                    ssfatal("Runtime Error: can't invoke \"%s.%s()\" - function requires %d argument%s.", object_name, program_name, arity, arity != 1 ? "s" : "");
                }
            }
            else
                ssfatal("Runtime Error: can't invoke \"%s.%s()\" - missing parameters array.", object_name, program_name);
        }
        else
            ssfatal("Runtime Error: can't invoke \"%s.%s()\" - missing parameters array.", object_name, program_name);
    }
    else
        ssfatal("Runtime Error: can't invoke \"%s.%s()\" - function doesn't exist.", object_name, program_name);

    ssfree(program_name);
    return ret;
}

/* the number of arguments of function name param[0], if it is defined (or 0 if not defined) */
surgescript_var_t* fun_arity(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_programpool_t* pool = surgescript_objectmanager_programpool(manager);
    const char* object_name = surgescript_object_name(object);
    const char* program_name = surgescript_var_fast_get_string(param[0]);

    surgescript_program_t* program = surgescript_programpool_get(pool, object_name, program_name);
    int arity = program != NULL ? surgescript_program_arity(program) : 0;
    
    return surgescript_var_set_number(surgescript_var_create(), arity);
}

/* basic assertion */
surgescript_var_t* fun_assert(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* object_name = surgescript_object_name(object);
    bool assertion = surgescript_var_get_bool(param[0]);
    const char* file = surgescript_var_fast_get_string(param[1]);
    int line = surgescript_var_get_number(param[2]);

    if(!assertion)
        ssfatal("Assertion failed! Object: \"%s\". Location: %s:%d.", object_name, file, line);

    return NULL;
}



/* --- misc --- */

/* auxiliary to fun_findobjects() and to fun_findobjectswithtag() */
void add_to_descendants_array(surgescript_objecthandle_t handle, void* arr)
{
    surgescript_object_t* array = (surgescript_object_t*)arr;
    surgescript_var_t* obj = surgescript_var_set_objecthandle(surgescript_var_create(), handle);
    const surgescript_var_t* param[] = { obj };
    surgescript_object_call_function(array, "push", param, 1, NULL);
    surgescript_var_destroy(obj);
}

/* auxiliary to fun_functions() */
void add_to_function_array(const char* fun_name, void* arr)
{
    if(is_visible_function(fun_name)) {
        surgescript_object_t* array = (surgescript_object_t*)arr;
        surgescript_var_t* fun = surgescript_var_set_string(surgescript_var_create(), fun_name);
        surgescript_var_t* ret = surgescript_var_create();
        const surgescript_var_t* param[] = { fun };

        /* if(array.indexOf(funName) < 0) array.push(funName); */
        surgescript_object_call_function(array, "indexOf", param, 1, ret);
        if(surgescript_var_get_number(ret) < 0)
            surgescript_object_call_function(array, "push", param, 1, NULL);
        
        surgescript_var_destroy(ret);
        surgescript_var_destroy(fun);
    }
}

/* is fun_name publicly visible or not? */
bool is_visible_function(const char* fun_name)
{
    return strncmp(fun_name, "state:", 6) && strcmp(fun_name, "__ssconstructor");
}

/* can the desired object be spawned? */
bool can_spawn_object(const char* object_name, surgescript_objectmanager_t* manager)
{
    return !(
        strcmp(object_name, "System") == 0 ||
        strcmp(object_name, "Application") == 0
    );
}
