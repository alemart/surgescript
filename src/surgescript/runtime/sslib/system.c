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
 * runtime/sslib/system.c
 * SurgeScript standard library: System object
 */

#include <stdio.h>
#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_exit(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_gettemp(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getgc(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_gettags(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getobjectcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* helpers */
static void install_plugins(surgescript_object_t* plugin_object, const char** plugins);
static surgescript_heapptr_t ISACTIVE_ADDR = 0;

/*
 * surgescript_sslib_register_system()
 * Register methods
 */
void surgescript_sslib_register_system(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "System", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "System", "exit", fun_exit, 0);
    surgescript_vm_bind(vm, "System", "destroy", fun_destroy, 0); /* overloads Object's destroy() */
    surgescript_vm_bind(vm, "System", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "System", "get_temp", fun_gettemp, 0);
    surgescript_vm_bind(vm, "System", "get_gc", fun_getgc, 0);
    surgescript_vm_bind(vm, "System", "get_tags", fun_gettags, 0);
    surgescript_vm_bind(vm, "System", "get_objectCount", fun_getobjectcount, 0);
    surgescript_vm_bind(vm, "System", "state:main", fun_main, 0);
}



/* my functions */

/* register some built-in objects */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char** system_objects = ((const char***)surgescript_object_userdata(object))[0];
    const char** plugins = ((const char***)surgescript_object_userdata(object))[1];
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t me = surgescript_object_handle(object);
    surgescript_heap_t* heap = surgescript_object_heap(object);

    /* is_active flag */
    surgescript_heapptr_t isactive_addr = surgescript_heap_malloc(heap);
    ssassert(isactive_addr == ISACTIVE_ADDR);
    surgescript_var_set_bool(surgescript_heap_at(heap, ISACTIVE_ADDR), true);

    /* spawn children; system_objects is a NULL-terminated array */
    for(const char** p = system_objects; *p != NULL; p++) {
        surgescript_var_t* mem = surgescript_heap_at(heap, surgescript_heap_malloc(heap));
        surgescript_var_set_objecthandle(mem, surgescript_objectmanager_spawn(manager, me, *p, NULL));
    }

    /* spawn plugins */
    install_plugins(
        surgescript_objectmanager_get(manager, surgescript_object_child(object, "Plugin")),
        plugins
    );

    /* spawn Application */
    surgescript_var_set_objecthandle(
        surgescript_heap_at(heap, surgescript_heap_malloc(heap)),
        surgescript_objectmanager_spawn(manager, me, "Application", NULL)
    );

    /* done! */
    return NULL;
}

/* exit() will shut down the VM */
surgescript_var_t* fun_exit(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_var_set_bool(surgescript_heap_at(heap, ISACTIVE_ADDR), false);

    /* can't do this directly, because in-use system objects
       required by destructors may be deleted */
    /*surgescript_object_kill(object);*/

    return NULL;
}

/* destroy function */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* this is the same as exit() */
    return fun_exit(object, param, num_params);
}

/* spawn function */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing; you can't spawn children on this object */
    return NULL;
}
 
/* get the temp area */
surgescript_var_t* fun_gettemp(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_child(object, "__Temp"));
}

/* get a reference to the Garbage Collector */
surgescript_var_t* fun_getgc(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_child(object, "__GC"));
}

/* get a reference to the Tag System */
surgescript_var_t* fun_gettags(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_child(object, "__TagSystem"));
}

/* number of objects allocated in the Runtime Environment */
surgescript_var_t* fun_getobjectcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    int count = surgescript_objectmanager_count(manager);
    return surgescript_var_set_number(surgescript_var_create(), count);
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    bool is_active = surgescript_var_get_bool(surgescript_heap_at(heap, ISACTIVE_ADDR));

    if(!is_active)
        surgescript_object_kill(object);

    return NULL;
}

/* install a set of plugins in the plugin object */
void install_plugins(surgescript_object_t* plugin_object, const char** plugins)
{
    /* Plugins are spawned in no particular order. At some
       point, we might want to add a dependency resolver. */
    surgescript_var_t* tmp = surgescript_var_create();
    while(*plugins) {
        const surgescript_var_t* param[] = { surgescript_var_set_string(tmp, *plugins++) };
        surgescript_object_call_function(plugin_object, "spawn", param, 1, NULL);
    }
    surgescript_var_destroy(tmp);
}