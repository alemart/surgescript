/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2024 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/sslib/gc.c
 * SurgeScript standard library: Garbage Collector controller
 */

#include "../vm.h"
#include "../object.h"
#include "../object_manager.h"
#include "../heap.h"
#include "../../util/util.h"

/* constants */
#if 0
static const double DEFAULT_GC_INTERVAL = 1.0; /* will run GC.collect() every DEFAULT_GC_INTERVAL seconds (by default) */
#else
static const double DEFAULT_GC_INTERVAL = 0.0; /* will run GC.collect() continuously (as fast as possible) */
#endif

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
    surgescript_vm_bind(vm, "__GC", "get_interval", fun_getinterval, 0);
    surgescript_vm_bind(vm, "__GC", "set_interval", fun_setinterval, 1);
    surgescript_vm_bind(vm, "__GC", "get_objectCount", fun_getobjectcount, 0);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    double now = surgescript_util_gettickcount() * 0.001;

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
    double interval = surgescript_var_get_number(surgescript_heap_at(heap, INTERVAL_ADDR));
    double last_collect = surgescript_var_get_number(surgescript_heap_at(heap, LASTCOLLECT_ADDR));
    double now = surgescript_util_gettickcount() * 0.001;

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
    double new_interval = ssmax(0.0, surgescript_var_get_number(param[0]));
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
