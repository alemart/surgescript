/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/time.c
 * SurgeScript standard library: Time routines
 */

#include <time.h>
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

/* utilities */
static float get_tick_count();
static const surgescript_heapptr_t TIME_ADDR = 0;
static const surgescript_heapptr_t DELTA_ADDR = 1;


/*
 * surgescript_sslib_register_time()
 * Register methods
 */
void surgescript_sslib_register_time(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Time", "__constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Time", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Time", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Time", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Time", "getTime", fun_gettime, 0);
    surgescript_vm_bind(vm, "Time", "getDelta", fun_getdelta, 0);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);

    ssassert(TIME_ADDR == surgescript_heap_malloc(heap));
    ssassert(DELTA_ADDR == surgescript_heap_malloc(heap));

    surgescript_var_set_number(surgescript_heap_at(heap, TIME_ADDR), get_tick_count());
    surgescript_var_set_number(surgescript_heap_at(heap, DELTA_ADDR), 0.016f);

    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    float new_time = get_tick_count();
    float old_time = surgescript_var_get_number(surgescript_heap_at(heap, TIME_ADDR));

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

/* utilities */

/* returns the number of seconds since the app was started */
float get_tick_count()
{
    /* this might crash after 24.8 days of uninterrupt execution */
    return (float)clock() / CLOCKS_PER_SEC;
}