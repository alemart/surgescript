/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2018  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/sslib/session.c
 * SurgeScript standard library: session variables
 */

#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getdata(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_isset(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_unset(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_clear(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* misc */
static surgescript_object_t* get_data_dict(surgescript_object_t* object);
static const surgescript_heapptr_t DATA_DICT = 0;

/*
 * surgescript_sslib_register_session()
 * Register methods
 */
void surgescript_sslib_register_session(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Session", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Session", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Session", "destroy", fun_destroy, 0); /* overloads Object's destroy() */
    surgescript_vm_bind(vm, "Session", "getData", fun_getdata, 0);
    surgescript_vm_bind(vm, "Session", "get", fun_get, 1);
    surgescript_vm_bind(vm, "Session", "set", fun_set, 2);
    surgescript_vm_bind(vm, "Session", "isset", fun_isset, 1);
    surgescript_vm_bind(vm, "Session", "unset", fun_unset, 1);
    surgescript_vm_bind(vm, "Session", "clear", fun_clear, 0);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t this_handle = surgescript_object_handle(object);
    surgescript_objecthandle_t data_handle = surgescript_objectmanager_spawn(manager, this_handle, "Dictionary", NULL);

    ssassert(DATA_DICT == surgescript_heap_malloc(heap));
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, DATA_DICT), data_handle);

    return NULL;
}

/* do nothing */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

/* do nothing */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

/* gets the data dictionary */
surgescript_var_t* fun_getdata(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, DATA_DICT));
}

/* gets the specified session variable */
surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_var_t* p[] = { param[0] };
    surgescript_var_t* value = surgescript_var_create();
    surgescript_object_t* data_dict = get_data_dict(object);
    surgescript_object_call_function(data_dict, "get", p, 1, value);
    return value;
}

/* modifies the specified session variable */
surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_var_t* p[] = { param[0], param[1] };
    surgescript_var_t* value = surgescript_var_create();
    surgescript_object_t* data_dict = get_data_dict(object);
    surgescript_object_call_function(data_dict, "set", p, 2, value);
    return value;
}

/* checks if a session variable exists */
surgescript_var_t* fun_isset(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_var_t* p[] = { param[0] };
    surgescript_var_t* value = surgescript_var_create();
    surgescript_object_t* data_dict = get_data_dict(object);
    surgescript_object_call_function(data_dict, "has", p, 1, value);
    return value;
}

/* removes a session variable */
surgescript_var_t* fun_unset(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_var_t* p[] = { param[0] };
    surgescript_object_t* data_dict = get_data_dict(object);
    surgescript_object_call_function(data_dict, "delete", p, 1, NULL);
    return NULL;
}

/* clears the session variables */
surgescript_var_t* fun_clear(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_t* data_dict = get_data_dict(object);
    surgescript_object_call_function(data_dict, "clear", NULL, 0, NULL);
    return NULL;
}







/* auxiliary methods */

surgescript_object_t* get_data_dict(surgescript_object_t* object)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t data_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, DATA_DICT));
    return surgescript_objectmanager_get(manager, data_handle);
}