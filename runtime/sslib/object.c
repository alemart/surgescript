/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017-2018  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/sslib/object.c
 * SurgeScript standard library: common routines for all objects
 */

#include <ctype.h>
#include <string.h>
#include "../vm.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_name(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_parent(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_child(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_sibling(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_equals(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_hasfun(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_findchild(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_hastag(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_timeout(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_functions(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* utilities */
static void add_to_function_array(const char* fun_name, void* arr);


/*
 * surgescript_sslib_register_object()
 * Register common methods to all objects
 */
void surgescript_sslib_register_object(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Object", "get__name", fun_name, 0);
    surgescript_vm_bind(vm, "Object", "get__functions", fun_functions, 0);
    surgescript_vm_bind(vm, "Object", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Object", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Object", "getParent", fun_parent, 0);
    surgescript_vm_bind(vm, "Object", "child", fun_child, 1);
    surgescript_vm_bind(vm, "Object", "findChild", fun_findchild, 1);
    surgescript_vm_bind(vm, "Object", "sibling", fun_sibling, 1);
    surgescript_vm_bind(vm, "Object", "toString", fun_tostring, 0);
    surgescript_vm_bind(vm, "Object", "equals", fun_equals, 1);
    surgescript_vm_bind(vm, "Object", "hasFunction", fun_hasfun, 1);
    surgescript_vm_bind(vm, "Object", "hasTag", fun_hastag, 1);
    surgescript_vm_bind(vm, "Object", "timeout", fun_timeout, 1);
}



/* my functions */

/* returns a handle to the parent object */
surgescript_var_t* fun_parent(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objecthandle_t parent = surgescript_object_parent(object);
    return surgescript_var_set_objecthandle(surgescript_var_create(), parent);
}

/* returns a handle to a child named param[0] (or a handle to null if not found) */
surgescript_var_t* fun_child(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* name = surgescript_var_fast_get_string(param[0]);
    surgescript_objecthandle_t child = surgescript_object_child(object, name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), child);
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

/* finds a child (or grand-child, or grand-grand-child, and so on) named param[0] */
surgescript_var_t* fun_findchild(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* name = surgescript_var_fast_get_string(param[0]);
    surgescript_objecthandle_t child = surgescript_object_find_child(object, name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), child);
}

/* spawns a child */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* name = surgescript_var_fast_get_string(param[0]);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t me = surgescript_object_handle(object);
    surgescript_objecthandle_t child = surgescript_objectmanager_spawn(manager, me, name, NULL);
    return surgescript_var_set_objecthandle(surgescript_var_create(), child);
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
    return surgescript_var_set_string(surgescript_var_create(), "[object]");
}

/* equals() method */
surgescript_var_t* fun_equals(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    if(surgescript_var_typecode(param[0]) == surgescript_var_type2code("object")) {
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
surgescript_var_t* fun_hasfun(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_objectmanager_t* object_manager = surgescript_object_manager(object);
    const char* object_name = surgescript_object_name(object);
    char* program_name = surgescript_var_get_string(param[0]);
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
    float elapsed = surgescript_object_elapsed_time(object);
    float threshold = surgescript_var_get_number(param[0]);
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

/* auxiliary to fun_functions() */
void add_to_function_array(const char* fun_name, void* arr)
{
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