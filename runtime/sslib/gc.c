/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/sslib/gc.c
 * SurgeScript standard library: Garbage Collector controller
 */

#include "../vm.h"
#include "../object.h"
#include "../../util/util.h"

/* constants */
static const float DEFAULT_GC_FREQUENCY = 1.0f; /* will run GC.collect() every DEFAULT_GC_FREQUENCY seconds (by default) */

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_collect(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setfrequency(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getfrequency(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static const surgescript_heapptr_t FREQUENCY_ADDR = 0;
static const surgescript_heapptr_t LASTCOLLECT_ADDR = 1;

/*
 * surgescript_sslib_register_gc()
 * Register methods
 */
void surgescript_sslib_register_gc(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "__GC", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "__GC", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "__GC", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "__GC", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "__GC", "collect", fun_collect, 0);
    surgescript_vm_bind(vm, "__GC", "getFrequency", fun_getfrequency, 0);
    surgescript_vm_bind(vm, "__GC", "setFrequency", fun_setfrequency, 1);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);

    ssassert(FREQUENCY_ADDR == surgescript_heap_malloc(heap));
    ssassert(LASTCOLLECT_ADDR == surgescript_heap_malloc(heap));

    surgescript_var_set_number(surgescript_heap_at(heap, FREQUENCY_ADDR), DEFAULT_GC_FREQUENCY);
    surgescript_var_set_number(surgescript_heap_at(heap, LASTCOLLECT_ADDR), 0.0f);

    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_heap_t* heap = surgescript_object_heap(object);
    float frequency = surgescript_var_get_number(surgescript_heap_at(heap, FREQUENCY_ADDR));
    float last_collect = surgescript_var_get_number(surgescript_heap_at(heap, LASTCOLLECT_ADDR));
    float now = surgescript_util_gettickcount();

    surgescript_objectmanager_garbagecheck(manager);
    if(now - last_collect >= frequency) {
        surgescript_object_call_function(object, "collect", NULL, 0, NULL);
        surgescript_var_set_number(surgescript_heap_at(heap, LASTCOLLECT_ADDR), now);
    }

    return NULL;
}

/* spawn */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* can't spawn anything on the GC! */
    return NULL;
}

/* destroy */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* can't destroy the GC! */
    return NULL;
}

/* calls the Garbage Collector */
surgescript_var_t* fun_collect(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objectmanager_garbagecollect(manager);
    return NULL;
}

/* get the GC frequency (in seconds) */
surgescript_var_t* fun_getfrequency(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, FREQUENCY_ADDR));
}

/* set the GC frequency (in seconds) */
surgescript_var_t* fun_setfrequency(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    float new_frequency = ssmax(0.0f, surgescript_var_get_number(param[0]));
    surgescript_var_set_number(surgescript_heap_at(heap, FREQUENCY_ADDR), new_frequency);
    return NULL;
}