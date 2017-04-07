/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object.c
 * SurgeScript standard library: common routines for all objects
 */

#include "../vm.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_name(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_parent(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_child(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_hasfun(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_findchild(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_object()
 * Register common methods to all objects
 */
void surgescript_sslib_register_object(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Object", "name", fun_name, 0);
    surgescript_vm_bind(vm, "Object", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Object", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Object", "parent", fun_parent, 0);
    surgescript_vm_bind(vm, "Object", "child", fun_child, 1);
    surgescript_vm_bind(vm, "Object", "findChild", fun_findchild, 1);
    surgescript_vm_bind(vm, "Object", "toString", fun_tostring, 0);
    surgescript_vm_bind(vm, "Object", "hasMemberFunction", fun_hasfun, 1);
    //surgescript_vm_bind(vm, "Object", "note", fun_note, 1); // TODO
}



/* my functions */

/* returns a handle to the parent object */
surgescript_var_t* fun_parent(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_handle_t parent = surgescript_object_parent(object);
    return surgescript_var_set_objecthandle(surgescript_var_create(), parent);
}

/* returns a handle to a child named param[0] (or a handle to null if not found) */
surgescript_var_t* fun_child(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* name = surgescript_var_get_string(param[0]);
    surgescript_objectmanager_handle_t child = surgescript_object_child(object, name);
    ssfree(name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), child);
}

/* finds a child (or grand-child, or grand-grand-child, and so on) named param[0] */
surgescript_var_t* fun_findchild(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* name = surgescript_var_get_string(param[0]);
    surgescript_objectmanager_handle_t child = surgescript_object_find_child(object, name);
    ssfree(name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), child);
}

/* spawns a child */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* name = surgescript_var_get_string(param[0]);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objectmanager_handle_t me = surgescript_object_handle(object);
    surgescript_objectmanager_handle_t child = surgescript_objectmanager_spawn(manager, me, name, NULL);
    ssfree(name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), child);
}

/* destroys the object that calls this */
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