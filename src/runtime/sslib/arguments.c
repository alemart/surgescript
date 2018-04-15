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
 * runtime/sslib/arguments.c
 * SurgeScript standard library: command line arguments
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
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getlength(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getdata(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_iterator(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_option(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_hasoption(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* misc */
static void populate_data_array(surgescript_object_t* array, struct surgescript_vmargs_t* args);
static surgescript_object_t* get_data_array(surgescript_object_t* object);
static const surgescript_heapptr_t DATA_ARRAY = 0;

/*
 * surgescript_sslib_register_arguments()
 * Register methods
 */
void surgescript_sslib_register_arguments(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Arguments", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Arguments", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Arguments", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Arguments", "toString", fun_tostring, 0);
    surgescript_vm_bind(vm, "Arguments", "get__data", fun_getdata, 0);
    surgescript_vm_bind(vm, "Arguments", "get", fun_get, 1);
    surgescript_vm_bind(vm, "Arguments", "getLength", fun_getlength, 0);
    surgescript_vm_bind(vm, "Arguments", "iterator", fun_iterator, 0);
    surgescript_vm_bind(vm, "Arguments", "option", fun_option, 1);
    surgescript_vm_bind(vm, "Arguments", "hasOption", fun_hasoption, 1);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t this_handle = surgescript_object_handle(object);
    surgescript_objecthandle_t data_handle = surgescript_objectmanager_spawn(manager, this_handle, "Array", NULL);

    ssassert(DATA_ARRAY == surgescript_heap_malloc(heap));
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, DATA_ARRAY), data_handle);
    populate_data_array(surgescript_objectmanager_get(manager, data_handle), surgescript_objectmanager_vmargs(manager));

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

/* converts to string */
surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_var_t* value = surgescript_var_create();
    surgescript_object_t* data_array = get_data_array(object);
    surgescript_object_call_function(data_array, "toString", NULL, 0, value);
    return value;
}

/* gets the i-th argument, 0 <= i < length */
surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_var_t* p[] = { param[0] };
    surgescript_var_t* value = surgescript_var_create();
    surgescript_object_t* data_array = get_data_array(object);
    surgescript_object_call_function(data_array, "get", p, 1, value);
    return value;
}

/* how many arguments? */
surgescript_var_t* fun_getlength(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_var_t* value = surgescript_var_create();
    surgescript_object_t* data_array = get_data_array(object);
    surgescript_object_call_function(data_array, "getLength", NULL, 0, value);
    return value;
}

/* gets the data array */
surgescript_var_t* fun_getdata(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, DATA_ARRAY));
}

/* returns an iterator */
surgescript_var_t* fun_iterator(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_var_t* value = surgescript_var_create();
    surgescript_object_t* data_array = get_data_array(object);
    surgescript_object_call_function(data_array, "iterator", NULL, 0, value);
    return value;
}

/* retrieves the value of a command-line option */
surgescript_var_t* fun_option(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_var_t* p[] = { param[0] };
    surgescript_var_t* value = surgescript_var_create();
    surgescript_object_t* data_array = get_data_array(object);
    surgescript_object_call_function(data_array, "indexOf", p, 1, value); /* searches the option */

    if(surgescript_var_get_number(value) >= 0) {
        /* the required option has been found */
        surgescript_var_t* index = surgescript_var_create();
        const surgescript_var_t* q[] = { index };
        surgescript_var_set_number(index, 1 + surgescript_var_get_number(value));
        surgescript_object_call_function(data_array, "get", q, 1, value);
        surgescript_var_destroy(index);
    }
    else {
        /* option not found */
        surgescript_var_set_null(value);
    }

    return value;
}

/* checks if a command-line option has been specified */
surgescript_var_t* fun_hasoption(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const surgescript_var_t* p[] = { param[0] };
    surgescript_var_t* value = surgescript_var_create();
    surgescript_object_t* data_array = get_data_array(object);
    surgescript_object_call_function(data_array, "indexOf", p, 1, value);
    bool has_option = (surgescript_var_get_number(value) >= 0);
    return surgescript_var_set_bool(value, has_option);
}



/* auxiliary methods */

/* populate the data array with the VM args (arguments from the command-line) */
void populate_data_array(surgescript_object_t* array, struct surgescript_vmargs_t* args)
{
    surgescript_var_t* str = surgescript_var_create();
    const surgescript_var_t* params[] = { str };
    const char** data = *((const char***)args);

    for(const char** it = data; *it != NULL; it++) {
        surgescript_var_set_string(str, *it);
        surgescript_object_call_function(array, "push", params, 1, NULL);
    }

    surgescript_var_destroy(str);
}

surgescript_object_t* get_data_array(surgescript_object_t* object)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t data_handle = surgescript_var_get_objecthandle(surgescript_heap_at(heap, DATA_ARRAY));
    return surgescript_objectmanager_get(manager, data_handle);
}