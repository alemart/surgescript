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
 * runtime/sslib/plugin.c
 * SurgeScript standard library: plugins
 */

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "../vm.h"
#include "../heap.h"
#include "../program.h"
#include "../program_pool.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* helpers */
#define isidchar(c) (isalnum(c) || (c) == '_' || (c) == '$')
static surgescript_program_t* make_accessor(surgescript_objecthandle_t plugin_handle);
static bool is_valid_name(const char* plugin_name);
static bool is_builtin_object(const char* plugin_name, surgescript_objectmanager_t* manager);

/*
 * surgescript_sslib_register_plugin()
 * Register methods
 */
void surgescript_sslib_register_plugin(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Plugin", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Plugin", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Plugin", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Plugin", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Plugin", "get", fun_get, 1);
    surgescript_vm_bind(vm, "Plugin", "get_count", fun_getcount, 0);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* done! */
    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* spawn */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* Plugin.spawn() shouldn't be used in SurgeScript code */
    const char* plugin_name = surgescript_var_fast_get_string(param[0]);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t me = surgescript_object_handle(object);
    surgescript_objecthandle_t plugin_handle = surgescript_object_child(object, plugin_name);

    /* exceptional check */
    if(is_builtin_object(plugin_name, manager)) {
        ssfatal("Runtime Error: can't spawn \"%s\" as a plugin.", plugin_name);
        return NULL;
    }

    /* plugin not installed */
    if(plugin_handle == surgescript_objectmanager_null(manager)) {
        /* spawn the plugin and save a reference to it in the memory */
        surgescript_heap_t* heap = surgescript_object_heap(object);
        surgescript_var_t* mem = surgescript_heap_at(heap, surgescript_heap_malloc(heap));
        plugin_handle = surgescript_objectmanager_spawn(manager, me, plugin_name, NULL);
        surgescript_var_set_objecthandle(mem, plugin_handle);

        /* create a getter */
        if(is_valid_name(plugin_name)) {
            const char* object_name = surgescript_object_name(object);
            surgescript_programpool_t* pool = surgescript_objectmanager_programpool(manager);
            char* accessor_name = surgescript_util_accessorfun("get", plugin_name);
            if(surgescript_programpool_get(pool, object_name, accessor_name) == NULL) {
                surgescript_program_t* accessor = make_accessor(plugin_handle);
                surgescript_programpool_put(pool, object_name, accessor_name, accessor);
            }
            else
                ssfatal("Runtime Error: duplicate plugin name \"%s\".", plugin_name); /* this shouldn't happen */
            ssfree(accessor_name);
        }
        else
            sslog("Warning: illegal plugin name \"%s\".", plugin_name);
    }
    else
        sslog("Warning: duplicate plugin \"%s\". Ignoring...", plugin_name);

    /* done! */
    return surgescript_var_set_objecthandle(surgescript_var_create(), plugin_handle);
}

/* destroy */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* can't destroy this! */
    return NULL;
}

/* gets a plugin (returns null if there is no plugin with the given name) */
surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const char* plugin_name = surgescript_var_fast_get_string(param[0]); /* this must be fast */
    surgescript_objecthandle_t plugin_handle = surgescript_object_child(object, plugin_name);
    return surgescript_var_set_objecthandle(surgescript_var_create(), plugin_handle);
}

/* plugin count */
surgescript_var_t* fun_getcount(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    int count = surgescript_object_child_count(object);
    return surgescript_var_set_number(surgescript_var_create(), count);
}

/* creates a getter that returns plugin_handle */
surgescript_program_t* make_accessor(surgescript_objecthandle_t plugin_handle)
{
    surgescript_program_t* program = surgescript_program_create(0);
    surgescript_program_add_line(program, SSOP_MOVO, SSOPu(0), SSOPu(plugin_handle));
    surgescript_program_add_line(program, SSOP_RET, SSOPu(0), SSOPu(0));
    return program;
}

/* a plugin name is valid if it matches that of an IDENTIFIER (see compiler/lexer.c) */
bool is_valid_name(const char* plugin_name)
{
    const char* p = plugin_name;

    if(!isidchar(*p) || isdigit(*p))
        return false;

    while(*(++p)) {
        if(!isidchar(*p))
            return false;
    }

    return true;
}

/* is this a builtin object? can't be added as a plugin */
bool is_builtin_object(const char* plugin_name, surgescript_objectmanager_t* manager)
{
    const char** builtins = surgescript_objectmanager_builtin_objects(manager);

    while(*builtins) {
        if(0 == strcmp(*builtins++, plugin_name))
            return true;
    }

    return false;
}