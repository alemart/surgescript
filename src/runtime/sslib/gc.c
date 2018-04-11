/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017-2018  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/sslib/gc.c
 * SurgeScript standard library: Garbage Collector controller
 */

#include "../vm.h"
#include "../object.h"
#include "../../util/util.h"

/* constants */
static const float DEFAULT_GC_INTERVAL = 1.0f; /* will run GC.collect() every DEFAULT_GC_INTERVAL seconds (by default) */

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_collect(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setinterval(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getinterval(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getobjectcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static const surgescript_heapptr_t INTERVAL_ADDR = 0;
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
    surgescript_vm_bind(vm, "__GC", "getInterval", fun_getinterval, 0);
    surgescript_vm_bind(vm, "__GC", "setInterval", fun_setinterval, 1);
    surgescript_vm_bind(vm, "__GC", "getObjectCount", fun_getobjectcount, 0);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    float now = surgescript_util_gettickcount() * 0.001f;

    ssassert(INTERVAL_ADDR == surgescript_heap_malloc(heap));
    ssassert(LASTCOLLECT_ADDR == surgescript_heap_malloc(heap));

    surgescript_var_set_number(surgescript_heap_at(heap, INTERVAL_ADDR), DEFAULT_GC_INTERVAL);
    surgescript_var_set_number(surgescript_heap_at(heap, LASTCOLLECT_ADDR), now);

    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_heap_t* heap = surgescript_object_heap(object);
    float interval = surgescript_var_get_number(surgescript_heap_at(heap, INTERVAL_ADDR));
    float last_collect = surgescript_var_get_number(surgescript_heap_at(heap, LASTCOLLECT_ADDR));
    float now = surgescript_util_gettickcount() * 0.001f;

    surgescript_objectmanager_garbagecheck(manager);
    if(now - last_collect >= interval) {
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

/* get the GC interval (in seconds) */
surgescript_var_t* fun_getinterval(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, INTERVAL_ADDR));
}

/* set the GC interval (in seconds) */
surgescript_var_t* fun_setinterval(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    float new_interval = ssmax(0.0f, surgescript_var_get_number(param[0]));
    surgescript_var_set_number(surgescript_heap_at(heap, INTERVAL_ADDR), new_interval);
    return NULL;
}

/* returns the (last) number of garbage-collected objects */
surgescript_var_t* fun_getobjectcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    int count = surgescript_objectmanager_garbagecount(manager);
    return surgescript_var_set_number(surgescript_var_create(), count);
}