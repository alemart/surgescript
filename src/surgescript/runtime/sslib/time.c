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
 * runtime/sslib/time.c
 * SurgeScript standard library: Time routines
 */

#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_gettime(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getdelta(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getnow(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* utilities */
static const surgescript_heapptr_t TIME_ADDR = 0;
static const surgescript_heapptr_t DELTA_ADDR = 1;
static const surgescript_heapptr_t START_ADDR = 2;


/*
 * surgescript_sslib_register_time()
 * Register methods
 */
void surgescript_sslib_register_time(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Time", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Time", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Time", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Time", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Time", "get_time", fun_gettime, 0);
    surgescript_vm_bind(vm, "Time", "get_delta", fun_getdelta, 0);
    surgescript_vm_bind(vm, "Time", "get_now", fun_getnow, 0);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);

    ssassert(TIME_ADDR == surgescript_heap_malloc(heap));
    ssassert(DELTA_ADDR == surgescript_heap_malloc(heap));
    ssassert(START_ADDR == surgescript_heap_malloc(heap));

    surgescript_var_set_number(surgescript_heap_at(heap, TIME_ADDR), 0.0);
    surgescript_var_set_number(surgescript_heap_at(heap, DELTA_ADDR), 0.01667);
    surgescript_var_set_number(surgescript_heap_at(heap, START_ADDR), surgescript_util_gettickcount() * 0.001);

    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    double start_time = surgescript_var_get_number(surgescript_heap_at(heap, START_ADDR));
    double new_time = surgescript_util_gettickcount() * 0.001 - start_time;
    double old_time = surgescript_var_get_number(surgescript_heap_at(heap, TIME_ADDR));

    /* update the timers */
    surgescript_var_set_number(surgescript_heap_at(heap, TIME_ADDR), new_time);
    surgescript_var_set_number(surgescript_heap_at(heap, DELTA_ADDR), new_time - old_time);

    return NULL;
}

/* destroy */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing, as system objects cannot be destroyed */
    return NULL;
}

/* spawn */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing; you can't spawn children on this object */
    return NULL;
}

/* the time (in seconds) at the beginning of this frame */
surgescript_var_t* fun_gettime(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, TIME_ADDR));
}

/* the time (in seconds) taken to complete the last frame */
surgescript_var_t* fun_getdelta(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, DELTA_ADDR));
}

/* the time (in seconds) since the app was started */
surgescript_var_t* fun_getnow(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    double start_time = surgescript_var_get_number(surgescript_heap_at(heap, START_ADDR));
    double current_time = surgescript_util_gettickcount() * 0.001 - start_time;
    return surgescript_var_set_number(surgescript_var_create(), current_time);
}