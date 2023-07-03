/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2023 Alexandre Martins <alemartf(at)gmail(dot)com>
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

#include <string.h>
#include <errno.h>
#include <time.h>
#include "vm.h"
#include "stack.h"
#include "variable.h"
#include "program_pool.h"
#include "tag_system.h"
#include "object_manager.h"
#include "vm_time.h"
#include "managed_string.h"
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
    char** data; /* this NULL-terminated array of strings must be the 1st element of the struct */
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
    surgescript_vmtime_t* time;
    bool is_paused;
};

/* misc */
static void init_vm(surgescript_vm_t* vm);
static void release_vm(surgescript_vm_t* vm);
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

    /* initialize the pools */
    sslog("Initializing the pools...");
    surgescript_managedstring_init_pool();
    surgescript_var_init_pool();

    /* set up the VM */
    sslog("Creating the VM...");
    init_vm(vm);

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
    release_vm(vm);

    sslog("Releasing the pools...");
    surgescript_var_release_pool();
    surgescript_managedstring_release_pool();

    sslog("The VM has been shut down.");
    return ssfree(vm);
}

/*
 * surgescript_vm_reset()
 * Resets a VM, clearing up all its programs and objects
 */
bool surgescript_vm_reset(surgescript_vm_t* vm)
{
    sslog("Will reset the VM...");

    if(surgescript_vm_is_active(vm)) {
        /* shut down the VM */
        sslog("Shutting down the VM...");
        release_vm(vm);

        /* release the pools */
        sslog("Releasing the pools...");
        surgescript_var_release_pool();
        surgescript_managedstring_release_pool();

        /* start new pools */
        sslog("Initializing new pools...");
        surgescript_managedstring_init_pool();
        surgescript_var_init_pool();

        /* set up the VM again */
        sslog("Starting the VM again...");
        init_vm(vm);

        /* done */
        return true;
    }
    else {
        sslog("Can't reset an inactive VM!");
        return false;
    }
}

/*
 * surgescript_vm_compile()
 * Compiles a file, given its absolute filepath
 * Returns true on success; false otherwise
 */
bool surgescript_vm_compile(surgescript_vm_t* vm, const char* absolute_path)
{
    const size_t BUFSIZE = 1024;
    size_t read_chars = 0, data_size = 0;
    char* data = NULL;

    /* open the file in binary mode, so that offsets don't get messed up */
    FILE* fp = surgescript_util_fopen_utf8(absolute_path, "rb");
    if(!fp) {
        ssfatal("Can't read file \"%s\": %s", absolute_path, strerror(errno));
        return false;
    }

    /* read file to data[] */
    sslog("Reading file %s...", absolute_path);
    do {
        data_size += BUFSIZE * sizeof(char);
        data = ssrealloc(data, data_size + 1);
        read_chars += fread(data + read_chars, sizeof(char), BUFSIZE, fp);
        data[read_chars] = '\0';
    } while(read_chars == data_size);
    fclose(fp);

    /* parse it */
    bool success = surgescript_parser_parse(vm->parser, data, absolute_path);

    /* done! */
    ssfree(data);
    return success;
}

/*
 * surgescript_vm_compile_code_in_memory()
 * Compiles the given code stored in memory
 * Returns true on success; false otherwise
 */
bool surgescript_vm_compile_code_in_memory(surgescript_vm_t* vm, const char* code)
{
    return surgescript_parser_parse(vm->parser, code, NULL);
}

/*
 * surgescript_vm_compile_virtual_file()
 * Compiles the given code stored in memory and specify a virtual filename or filepath
 * Returns true on success; false otherwise
 */
bool surgescript_vm_compile_virtual_file(surgescript_vm_t* vm, const char* code, const char* filename)
{
    return surgescript_parser_parse(vm->parser, code, filename);
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
    /* Already launched? */
    if(surgescript_vm_is_active(vm))
        return;

    /* Setup the pseudo-number generator */
    surgescript_util_srand(time(NULL));

    /* Setup the command line arguments */
    surgescript_vmargs_configure(vm->args, argc, argv);

    /* Install plugins */
    surgescript_parser_foreach_plugin(vm->parser, vm, install_plugin);

    /* Create the root object */
    surgescript_objectmanager_spawn_root(vm->object_manager);
}

/*
 * surgescript_vm_is_active()
 * Is the VM active? (i.e., turned ON)
 */
bool surgescript_vm_is_active(surgescript_vm_t* vm)
{
    surgescript_objecthandle_t root_handle = surgescript_objectmanager_root(vm->object_manager);
    return surgescript_objectmanager_exists(vm->object_manager, root_handle);
}

/*
 * surgescript_vm_update()
 * Updates the VM. Returns true if the VM is active after this update cycle
 */
bool surgescript_vm_update(surgescript_vm_t* vm)
{
    return surgescript_vm_update_ex(vm, NULL, NULL, NULL);
}

/*
 * surgescript_vm_update_ex()
 * Updates the VM, allowing user-defined callbacks as well.
 * Returns true if the VM is active after this update cycle
 */
bool surgescript_vm_update_ex(surgescript_vm_t* vm, void* user_data, void (*user_update)(surgescript_object_t*,void*), void (*late_update)(surgescript_object_t*,void*))
{
    if(surgescript_vm_is_active(vm) && !vm->is_paused) {
        surgescript_object_t* root = surgescript_vm_root_object(vm);
        surgescript_vm_updater_t updater = { user_data, user_update, late_update };

        /* update time */
        surgescript_vmtime_update(vm->time);

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
    else {
        /* return true if the VM is still on */
        return surgescript_vm_is_active(vm);
    }
}

/*
 * surgescript_vm_terminate()
 * Terminates the vm
 */
void surgescript_vm_terminate(surgescript_vm_t* vm)
{
    surgescript_object_t* root = surgescript_vm_root_object(vm);
    surgescript_object_kill(root);
}

/*
 * surgescript_vm_pause()
 * Pauses the VM, so that surgescript_vm_update_ex() does not update any objects
 */
void surgescript_vm_pause(surgescript_vm_t* vm)
{
    /* nothing to do */
    if(vm->is_paused)
        return;

    /* pause the VM */
    surgescript_vmtime_pause(vm->time);
    vm->is_paused = true;
}

/*
 * surgescript_vm_resume()
 * Resumes a paused VM
 */
void surgescript_vm_resume(surgescript_vm_t* vm)
{
    /* nothing to do */
    if(!vm->is_paused)
        return;

    /* pause the VM */
    surgescript_vmtime_resume(vm->time);
    vm->is_paused = false;
}

/*
 * surgescript_vm_is_paused()
 * Is the VM paused?
 */
bool surgescript_vm_is_paused(const surgescript_vm_t* vm)
{
    return vm->is_paused;
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
const surgescript_vmargs_t* surgescript_vm_args(const surgescript_vm_t* vm)
{
    return vm->args;
}

/*
 * surgescript_vm_time()
 * Gets the VM time
 */
const surgescript_vmtime_t* surgescript_vm_time(const surgescript_vm_t* vm)
{
    return vm->time;
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

/* initializes the VM */
void init_vm(surgescript_vm_t* vm)
{
    vm->is_paused = false;

    /* create the VM components */
    vm->stack = surgescript_stack_create();
    vm->program_pool = surgescript_programpool_create();
    vm->tag_system = surgescript_tagsystem_create();
    vm->args = surgescript_vmargs_create();
    vm->time = surgescript_vmtime_create();
    vm->object_manager = surgescript_objectmanager_create(vm->program_pool, vm->tag_system, vm->stack, vm->args, vm->time);
    vm->parser = surgescript_parser_create(vm->program_pool, vm->tag_system);

    /* load the SurgeScript standard library */
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
}

/* releases the VM */
void release_vm(surgescript_vm_t* vm)
{
    /* destroy the VM components */
    surgescript_parser_destroy(vm->parser);
    surgescript_objectmanager_destroy(vm->object_manager);
    surgescript_vmtime_destroy(vm->time);
    surgescript_vmargs_destroy(vm->args);
    surgescript_tagsystem_destroy(vm->tag_system);
    surgescript_programpool_destroy(vm->program_pool);
    surgescript_stack_destroy(vm->stack);
}

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
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    bool update_children = true;

    update_children = surgescript_object_update(object);
    if(surgescript_objectmanager_exists(manager, handle) && /* is the object still valid? */
    surgescript_objectmanager_get(manager, handle) == object)
        vm_updater->late_update(object, vm_updater->user_data);

    return update_children;
}

bool call_updater3(surgescript_object_t* object, void* updater)
{
    surgescript_vm_updater_t* vm_updater = (surgescript_vm_updater_t*)updater;
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    bool update_children = true;

    vm_updater->user_update(object, vm_updater->user_data);
    update_children = surgescript_object_update(object);
    if(surgescript_objectmanager_exists(manager, handle) && /* is the object still valid? */
    surgescript_objectmanager_get(manager, handle) == object)
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
