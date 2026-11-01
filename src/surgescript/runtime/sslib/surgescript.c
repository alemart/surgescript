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
 * runtime/sslib/surgescript.c
 * SurgeScript standard library: SurgeScript object
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
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getversion(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_gettimer(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static const char surgescript_code[];
static const surgescript_heapptr_t TIMERFACTORY_ADDR = 0;

/*
 * surgescript_sslib_register_surgescript()
 * Register methods
 */
void surgescript_sslib_register_surgescript(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "SurgeScript", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "SurgeScript", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "SurgeScript", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "SurgeScript", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "SurgeScript", "get_version", fun_getversion, 0);
    surgescript_vm_bind(vm, "SurgeScript", "get_Timer", fun_gettimer, 0);

    surgescript_vm_compile_code_in_memory(vm, surgescript_code);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    ssassert(TIMERFACTORY_ADDR == surgescript_heap_malloc(heap));

    /* SurgeScript is a system object. Thus, the spawning of children must be
       delayed until all other system objects, which have fixed and known object
       handles, are spawned. Alternatively, the SurgeScript object could become
       a Plugin global, so that it's spawned after all system objects. */
    ;

    /* done */
    return NULL;
}

/* destroy function */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* spawn function */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing; you can't spawn children on this object */
    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_set_active(object, false); /* we don't need to spend time updating this object */
    return NULL;
}
 
/* SurgeScript version */
surgescript_var_t* fun_getversion(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_string(surgescript_var_create(), surgescript_util_version());
}

/* Timer factory */
surgescript_var_t* fun_gettimer(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_var_t* timer_var = surgescript_heap_at(heap, TIMERFACTORY_ADDR);

    /* lazy spawn */
    if(surgescript_var_is_null(timer_var)) {
        surgescript_objectmanager_t* manager = surgescript_object_manager(object);
        surgescript_objecthandle_t self_handle = surgescript_object_handle(object);
        surgescript_objecthandle_t timer_factory_handle = surgescript_objectmanager_spawn(manager, self_handle, "TimerFactory", NULL);
        surgescript_var_set_objecthandle(timer_var, timer_factory_handle);
    }

    /* done! */
    return surgescript_var_clone(timer_var);
}




/* SurgeScript code */
static const char surgescript_code[] = "\
object 'TimerFactory' \n\
{ \n\
    fun call() { return caller.spawn('Timer'); } \n\
    fun destroy() { } \n\
} \n\
";

/* EOF */