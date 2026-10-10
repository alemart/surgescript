/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2026 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/sslib/signal.c
 * SurgeScript standard library: Signal objects
 */

#include <string.h>
#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../object_manager.h"
#include "../signal_system.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getname(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getemitter(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getcontext(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_init(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* utilities */
typedef struct privatesignaldata_t {
    surgescript_signalcode_t signal_code;
    surgescript_objecthandle_t emitter;
    surgescript_objecthandle_t context;
} privatesignaldata_t;

static const privatesignaldata_t EMPTY_SIGNAL = { 0 };


/*
 * surgescript_sslib_register_signal()
 * Register methods
 */
void surgescript_sslib_register_signal(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Signal", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Signal", "destructor", fun_destructor, 0);
    surgescript_vm_bind(vm, "Signal", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Signal", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Signal", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Signal", "get_name", fun_getname, 0);
    surgescript_vm_bind(vm, "Signal", "get_emitter", fun_getemitter, 0);
    surgescript_vm_bind(vm, "Signal", "get_context", fun_getcontext, 0);
    surgescript_vm_bind(vm, "Signal", "__init", fun_init, 3);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    ssassert(NULL == surgescript_object_userdata(object));
    return NULL;
}

/* destructor */
surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    privatesignaldata_t* data = (privatesignaldata_t*)surgescript_object_userdata(object);

    if(data != NULL)
        ssfree(data);

    return NULL;
}

/* initialization */
surgescript_var_t* fun_init(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* skip if already initialized */
    if(NULL != surgescript_object_userdata(object))
        return NULL;

    /* read parameters */
    int64_t signal_code_i64 = surgescript_var_get_rawbits(param[0]);
    surgescript_objecthandle_t emitter = surgescript_var_get_objecthandle(param[1]);
    surgescript_objecthandle_t context = surgescript_var_get_objecthandle(param[2]);

    /* allocate fields */
    privatesignaldata_t* data = ssmalloc(sizeof *data);
    *data = (privatesignaldata_t){
        .signal_code = (surgescript_signalcode_t)signal_code_i64,
        .emitter = emitter,
        .context = context
    };
    SS_STATIC_ASSERT(sizeof(signal_code_i64) > sizeof(data->signal_code)); /* just to make sure */

    /* store data */
    surgescript_object_set_userdata(object, data);

    /* keep a reference to the signal context to prevent garbage collection */
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_heapptr_t heap_addr = surgescript_heap_malloc(heap);
    surgescript_var_t* heap_var = surgescript_heap_at(heap, heap_addr);
    surgescript_var_set_objecthandle(heap_var, context);

    /* done */
    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* destroy */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing; signal objects cannot be destroyed */
    return NULL;
}

/* spawn */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing; you can't spawn children on this object */
    return NULL;
}

/* the name of the signal */
surgescript_var_t* fun_getname(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const privatesignaldata_t* data = surgescript_object_userdata(object);

    /* return a default value if not initialized, or if empty-initialized */
    if(data == NULL || 0 == memcmp(data, &EMPTY_SIGNAL, sizeof *data))
        return surgescript_var_set_string(surgescript_var_create(), "");

    /* return the name of the signal */
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    const surgescript_signalsystem_t* signal_system = surgescript_objectmanager_signalsystem(manager);
    const char* signal_name = surgescript_signalsystem_signal_name(signal_system, data->signal_code);
    return surgescript_var_set_string(surgescript_var_create(), signal_name);
}

/* the object that emitted this signal */
surgescript_var_t* fun_getemitter(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const privatesignaldata_t* data = surgescript_object_userdata(object);

    /* return a default value if not initialized */
    if(data == NULL)
        return surgescript_var_set_null(surgescript_var_create());

    /* return the stored value */
    return surgescript_var_set_objecthandle(surgescript_var_create(), data->emitter);
}

/* the context object, or null */
surgescript_var_t* fun_getcontext(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const privatesignaldata_t* data = surgescript_object_userdata(object);

    /* return a default value if not initialized */
    if(data == NULL)
        return surgescript_var_set_null(surgescript_var_create());

    /* return the stored value */
    return surgescript_var_set_objecthandle(surgescript_var_create(), data->context);
}
