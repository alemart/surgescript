/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object.c
 * SurgeScript standard library: common routines for all objects
 */

#include <stdio.h>
#include "../vm.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_app(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_parent(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_changestate(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_name(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_stdlib_register_object()
 * Register common methods to all objects
 */
void surgescript_stdlib_register_object(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Object", "app", fun_app, 0);
    surgescript_vm_bind(vm, "Object", "parent", fun_parent, 0);
    surgescript_vm_bind(vm, "Object", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Object", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Object", "changeState", fun_changestate, 1);
    surgescript_vm_bind(vm, "Object", "name", fun_name, 0);
    surgescript_vm_bind(vm, "Object", "toString", fun_tostring, 0);
}



/* my functions */

/* returns a handle to the root object */
surgescript_var_t* fun_app(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objectmanager_handle_t root = surgescript_objectmanager_root(manager);
    return surgescript_var_set_objecthandle(surgescript_var_create(), root);
}

/* returns a handle to the parent object */
surgescript_var_t* fun_parent(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_handle_t parent = surgescript_object_parent(object);
    return surgescript_var_set_objecthandle(surgescript_var_create(), parent);
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
    char buf[128];
    snprintf(buf, sizeof(buf), "[object \"%s\"]", surgescript_object_name(object));
    return surgescript_var_set_string(surgescript_var_create(), buf);
}

/* what's the name of this object? */
surgescript_var_t* fun_name(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_string(surgescript_var_create(), surgescript_object_name(object));
}

/* changes the state of this object */
surgescript_var_t* fun_changestate(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* state = surgescript_var_get_string(param[0]);
    surgescript_object_set_state(object, state);
    ssfree(state);
    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
}