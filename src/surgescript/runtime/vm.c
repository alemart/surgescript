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
 * runtime/vm.c
 * SurgeScript Virtual Machine - Runtime Engine
 */

#include <locale.h>
#include "vm.h"
#include "stack.h"
#include "variable.h"
#include "program_pool.h"
#include "tag_system.h"
#include "object_manager.h"
#include "sslib/sslib.h"
#include "../compiler/parser.h"
#include "../util/util.h"


/* auxiliary data structure */
typedef struct surgescript_vm_updater_t surgescript_vm_updater_t;
struct surgescript_vm_updater_t { 
    /* this allows the introduction of user-defined callbacks
       for each surgescript object in the object tree */
    void* user_data; /* user-defined data */
    void (*user_update)(surgescript_object_t*,void*); /* runs just before surgescript_object_update() */
    void (*late_update)(surgescript_object_t*,void*); /* runs immediately after surgescript_object_update() */
};

/* VM command-line arguments */
typedef struct surgescript_vmargs_t surgescript_vmargs_t;
struct surgescript_vmargs_t {
    char** data; /* NULL-terminated string; must be the 1st element */
};

static surgescript_vmargs_t* surgescript_vmargs_create();
static surgescript_vmargs_t* surgescript_vmargs_destroy(surgescript_vmargs_t* args);
static surgescript_vmargs_t* surgescript_vmargs_configure(surgescript_vmargs_t* args, int argc, char** argv);


/* VM */
struct surgescript_vm_t
{
    surgescript_stack_t* stack;
    surgescript_programpool_t* program_pool;
    surgescript_tagsystem_t* tag_system;
    surgescript_objectmanager_t* object_manager;
    surgescript_parser_t* parser;
    surgescript_vmargs_t* args;
    double start_time;
};

/* misc */
static bool call_updater1(surgescript_object_t* object, void* updater);
static bool call_updater2(surgescript_object_t* object, void* updater);
static bool call_updater3(surgescript_object_t* object, void* updater);
static void install_plugin(const char* object_name, void* data);



/*
 * surgescript_vm_create()
 * Creates a vm
 */
surgescript_vm_t* surgescript_vm_create()
{
    surgescript_vm_t* vm = ssmalloc(sizeof *vm);

    /* SurgeScript info */
    sslog("Using SurgeScript %s", surgescript_util_version());

    /* boot */
    sslog("Booting up the VM...");
    surgescript_var_init_pool();

    /* set up the VM */
    vm->start_time = 0.0;
    vm->stack = surgescript_stack_create();
    vm->program_pool = surgescript_programpool_create();
    vm->tag_system = surgescript_tagsystem_create();
    vm->args = surgescript_vmargs_create();
    vm->object_manager = surgescript_objectmanager_create(vm->program_pool, vm->tag_system, vm->stack, vm->args);
    vm->parser = surgescript_parser_create(vm->program_pool, vm->tag_system);

    /* load the SurgeScript library */
    surgescript_sslib_register_object(vm);
    surgescript_sslib_register_string(vm);
    surgescript_sslib_register_number(vm);
    surgescript_sslib_register_boolean(vm);
    surgescript_sslib_register_temp(vm);
    surgescript_sslib_register_gc(vm);
    surgescript_sslib_register_array(vm);
    surgescript_sslib_register_dictionary(vm);
    surgescript_sslib_register_time(vm);
    surgescript_sslib_register_date(vm);
    surgescript_sslib_register_math(vm);
    surgescript_sslib_register_console(vm);
    surgescript_sslib_register_tagsystem(vm);
    surgescript_sslib_register_plugin(vm);
    surgescript_sslib_register_surgescript(vm);
    surgescript_sslib_register_arguments(vm);
    surgescript_sslib_register_application(vm);
    surgescript_sslib_register_system(vm);

    /* done! */
    return vm;
}

/*
 * surgescript_vm_destroy()
 * Destroys a VM
 */
surgescript_vm_t* surgescript_vm_destroy(surgescript_vm_t* vm)
{
    sslog("Shutting down the VM...");
    surgescript_parser_destroy(vm->parser);
    surgescript_objectmanager_destroy(vm->object_manager);
    surgescript_vmargs_destroy(vm->args);
    surgescript_tagsystem_destroy(vm->tag_system);
    surgescript_programpool_destroy(vm->program_pool);
    surgescript_stack_destroy(vm->stack);
    surgescript_var_release_pool();
    return ssfree(vm);
}

/*
 * surgescript_vm_compile()
 * Compiles a file, given its absolute filepath
 * Returns true on success; false otherwise
 */
bool surgescript_vm_compile(surgescript_vm_t* vm, const char* absolute_path)
{
    return surgescript_parser_parsefile(vm->parser, absolute_path);
}

/*
 * surgescript_vm_compile_code_in_memory()
 * Compiles the given code, stored in memory
 * Returns true on success; false otherwise
 */
bool surgescript_vm_compile_code_in_memory(surgescript_vm_t* vm, const char* code)
{
    return surgescript_parser_parsemem(vm->parser, code);
}

/*
 * surgescript_vm_launch()
 * Boots up the vm
 */
void surgescript_vm_launch(surgescript_vm_t* vm)
{
    surgescript_vm_launch_ex(vm, 0, NULL);
}

/*
 * surgescript_vm_launch_ex()
 * Boots up the vm with command line arguments
 */
void surgescript_vm_launch_ex(surgescript_vm_t* vm, int argc, char** argv)
{
    /* SurgeScript uses UTF-8 */
    setlocale(LC_ALL, "en_US.UTF-8");

    /* Setup the command line arguments */
    surgescript_vmargs_configure(vm->args, argc, argv);

    /* Install plugins */
    surgescript_parser_foreach_plugin(vm->parser, vm, install_plugin);

    /* Create the root object */
    surgescript_objectmanager_spawn_root(vm->object_manager);
}

/*
 * surgescript_vm_is_active()
 * Is the VM active?
 */
bool surgescript_vm_is_active(surgescript_vm_t* vm)
{
    surgescript_objectmanager_t* manager = vm->object_manager;
    surgescript_objecthandle_t root_handle = surgescript_objectmanager_root(manager);
    return surgescript_objectmanager_exists(manager, root_handle) &&
           !surgescript_object_is_killed(surgescript_objectmanager_get(manager, root_handle));
}

/*
 * surgescript_vm_update()
 * Updates the VM
 */
bool surgescript_vm_update(surgescript_vm_t* vm)
{
    if(surgescript_vm_is_active(vm)) {
        surgescript_object_t* root = surgescript_vm_root_object(vm);
        surgescript_object_traverse_tree(root, surgescript_object_update);
        return surgescript_vm_is_active(vm);
    }
    else
        return false;
}

/*
 * surgescript_vm_update_ex()
 * Updates the VM, allowing user-defined callbacks as well
 */
bool surgescript_vm_update_ex(surgescript_vm_t* vm, void* user_data, void (*user_update)(surgescript_object_t*,void*), void (*late_update)(surgescript_object_t*,void*))
{
    if(surgescript_vm_is_active(vm)) {
        surgescript_object_t* root = surgescript_vm_root_object(vm);
        surgescript_vm_updater_t updater = { user_data, user_update, late_update };

        /* update */
        if(user_update != NULL && late_update != NULL)
            surgescript_object_traverse_tree_ex(root, &updater, call_updater3);
        else if(late_update != NULL)
            surgescript_object_traverse_tree_ex(root, &updater, call_updater2);
        else if(user_update != NULL)
            surgescript_object_traverse_tree_ex(root, &updater, call_updater1);
        else
            surgescript_object_traverse_tree(root, surgescript_object_update);

        /* done! */
        return surgescript_vm_is_active(vm);
    }
    else
        return false;
}

/*
 * surgescript_vm_terminate()
 * terminates the vm
 */
void surgescript_vm_terminate(surgescript_vm_t* vm)
{
    surgescript_object_t* root = surgescript_vm_root_object(vm);
    surgescript_object_kill(root);
}

/*
 * surgescript_vm_programpool()
 * Gets the program pool
 */
surgescript_programpool_t* surgescript_vm_programpool(const surgescript_vm_t* vm)
{
    return vm->program_pool;
}

/*
 * surgescript_vm_tagsystem()
 * Gets the tag system
 */
surgescript_tagsystem_t* surgescript_vm_tagsystem(const surgescript_vm_t* vm)
{
    return vm->tag_system;
}

/*
 * surgescript_vm_objectmanager()
 * Gets the object manager
 */
surgescript_objectmanager_t* surgescript_vm_objectmanager(const surgescript_vm_t* vm)
{
    return vm->object_manager;
}

/*
 * surgescript_vm_parser()
 * Gets the parser
 */
surgescript_parser_t* surgescript_vm_parser(const surgescript_vm_t* vm)
{
    return vm->parser;
}

/*
 * surgescript_vm_args()
 * Gets the command-line arguments
 */
surgescript_vmargs_t* surgescript_vm_args(const surgescript_vm_t* vm)
{
    return vm->args;
}

/*
 * surgescript_vm_root_object()
 * Gets the root object
 */
surgescript_object_t* surgescript_vm_root_object(surgescript_vm_t* vm)
{
    surgescript_objecthandle_t root_handle = surgescript_objectmanager_root(vm->object_manager);
    return surgescript_objectmanager_get(vm->object_manager, root_handle);
}

/*
 * surgescript_vm_spawn_object()
 * Spawns an object. user_data may be NULL
 */
surgescript_object_t* surgescript_vm_spawn_object(surgescript_vm_t* vm, surgescript_object_t* parent, const char* object_name, void* user_data)
{
    surgescript_objecthandle_t parent_handle = surgescript_object_handle(parent);
    surgescript_objecthandle_t child_handle = surgescript_objectmanager_spawn(vm->object_manager, parent_handle, object_name, user_data);
    return surgescript_objectmanager_get(vm->object_manager, child_handle);
}

/*
 * surgescript_vm_find_object()
 * Finds an object named object_name (this may be slow; use with care)
 */
surgescript_object_t* surgescript_vm_find_object(surgescript_vm_t* vm, const char* object_name)
{
    const surgescript_object_t* root = surgescript_vm_root_object(vm);
    surgescript_objecthandle_t handle = surgescript_object_find_descendant(root, object_name);
    return surgescript_objectmanager_get(vm->object_manager, handle);
}

/*
 * surgescript_vm_bind()
 * Binds a C function to a SurgeScript object
 */
void surgescript_vm_bind(surgescript_vm_t* vm, const char* object_name, const char* fun_name, surgescript_program_cfunction_t cfun, int num_params)
{
    surgescript_program_t* cprogram = surgescript_program_create_native(num_params, cfun);
    surgescript_programpool_replace(vm->program_pool, object_name, fun_name, cprogram);
}

/*
 * surgescript_vm_install_plugin()
 * Sets a certain object as a plugin. Call before launching the VM.
 */
void surgescript_vm_install_plugin(surgescript_vm_t* vm, const char* object_name)
{
    surgescript_objectmanager_t* manager = vm->object_manager;
    surgescript_objectmanager_install_plugin(manager, object_name);
}

/* ----- private ----- */

/* these auxiliary functions help traversing the object tree */
bool call_updater1(surgescript_object_t* object, void* updater)
{
    surgescript_vm_updater_t* vm_updater = (surgescript_vm_updater_t*)updater;
    vm_updater->user_update(object, vm_updater->user_data);
    return surgescript_object_update(object);
}

bool call_updater2(surgescript_object_t* object, void* updater)
{
    surgescript_vm_updater_t* vm_updater = (surgescript_vm_updater_t*)updater;
    bool update_children = true;
    update_children = surgescript_object_update(object);
    vm_updater->late_update(object, vm_updater->user_data);
    return update_children;
}

bool call_updater3(surgescript_object_t* object, void* updater)
{
    surgescript_vm_updater_t* vm_updater = (surgescript_vm_updater_t*)updater;
    bool update_children = true;
    vm_updater->user_update(object, vm_updater->user_data);
    update_children = surgescript_object_update(object);
    vm_updater->late_update(object, vm_updater->user_data);
    return update_children;
}

/* plugin installer */
void install_plugin(const char* object_name, void* data)
{
    surgescript_vm_t* vm = (surgescript_vm_t*)data;
    surgescript_objectmanager_install_plugin(vm->object_manager, object_name);
}

/* VM command-line arguments */
surgescript_vmargs_t* surgescript_vmargs_create()
{
    surgescript_vmargs_t* args = ssmalloc(sizeof *args);
    args->data = NULL;
    return args;
}

surgescript_vmargs_t* surgescript_vmargs_configure(surgescript_vmargs_t* args, int argc, char** argv)
{
    /* delete existing data */
    if(args->data != NULL) {
        for(char** it = args->data; *it != NULL; it++)
            ssfree(*it);
        ssfree(args->data);
        args->data = NULL;
    }

    /* set up the new data */
    if(argc >= 0) {
        args->data = ssmalloc((1 + argc) * sizeof(*(args->data)));
        args->data[argc] = NULL;
        while(argc--)
            args->data[argc] = ssstrdup(argv[argc]);
    }

    /* done! */
    return args;
}

surgescript_vmargs_t* surgescript_vmargs_destroy(surgescript_vmargs_t* args)
{
    surgescript_vmargs_configure(args, -1, NULL);
    return ssfree(args);
}
