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

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "../vm.h"
#include "../object.h"
#include "../object_manager.h"
#include "../heap.h"
#include "../../util/util.h"

/* helpers & constants */
static const int DEFAULT_GC_INTERVAL = 1000;  /* will run GC.collect() every DEFAULT_GC_INTERVAL milliseconds by default
                                                (it traverses the entire object tree from the root) */
static const int MINIMUM_GC_INTERVAL = 0;     /* run the GC as fast as possible */
static const int MAXIMUM_GC_INTERVAL = 20000;
static const char GC_INTERVAL_COMMAND_LINE_OPTION_NAME[] = "--surgescript-gc-interval";
static int find_gc_interval(const struct surgescript_vmargs_t* args);
static inline bool is_integer(const char* str);

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
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    const struct surgescript_vmargs_t* args = surgescript_objectmanager_vmargs(manager);
    double gc_interval = 0.001 * find_gc_interval(args);
    double now = 0.001 * surgescript_util_gettickcount();

    ssassert(INTERVAL_ADDR == surgescript_heap_malloc(heap));
    ssassert(LASTCOLLECT_ADDR == surgescript_heap_malloc(heap));

    surgescript_var_set_number(surgescript_heap_at(heap, INTERVAL_ADDR), gc_interval);
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

    /* look for garbage */
    surgescript_objectmanager_garbagecheck(manager);

    /* is it time to collect? */
    double now = surgescript_util_gettickcount() * 0.001;
    if(now - last_collect >= interval) {
        /* collect garbage */
        surgescript_object_call_function(object, "collect", NULL, 0, NULL);

        /* update collect time */
        now = surgescript_util_gettickcount() * 0.001;
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
    /* this setter is obsolete and does nothing since SurgeScript 0.6.0.
       It has been kept for backwards compatibility. */
    return NULL;
}

/* returns the (last) number of garbage-collected objects */
surgescript_var_t* fun_getobjectcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    int count = surgescript_objectmanager_garbagecount(manager);
    return surgescript_var_set_number(surgescript_var_create(), count);
}

/* ----- */

/* finds the desired the interval of the Garbage Collector */
int find_gc_interval(const struct surgescript_vmargs_t* args)
{
    /* TODO add a vmargs parser */
    const char** argv = *((const char***)args);

    for(const char** it = argv; *it != NULL; it++) {
        if(0 == strcmp(*it, GC_INTERVAL_COMMAND_LINE_OPTION_NAME)) {
            if(*(++it) != NULL && is_integer(*it)) {
                int x = atoi(*it);
                int milliseconds = ssclamp(x, MINIMUM_GC_INTERVAL, MAXIMUM_GC_INTERVAL);
                sslog("The garbage collector interval has been set to %d ms via %s", milliseconds, GC_INTERVAL_COMMAND_LINE_OPTION_NAME);
                return milliseconds;
            }

            sslog("Invalid argument given to %s: \"%s\"", GC_INTERVAL_COMMAND_LINE_OPTION_NAME, *it);
            --it;
        }
    }

    sslog("The garbage collector interval has been set to the default of %d ms", DEFAULT_GC_INTERVAL);
    return DEFAULT_GC_INTERVAL;
}

/* checks if a string encodes a non-negative integer number written in base 10 */
bool is_integer(const char* str)
{
    return strspn(str, "0123456789") == strlen(str);
}
