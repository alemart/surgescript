/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/vm.c
 * SurgeScript Virtual Machine for the Runtime Engine
 */

#include <locale.h>
#include "vm.h"
#include "stack.h"
#include "sslib/sslib.h"
#include "../util/util.h"

/* VM */
struct surgescript_vm_t
{
    surgescript_stack_t* stack;
    surgescript_programpool_t* program_pool;
    surgescript_tagsystem_t* tag_system;
    surgescript_objectmanager_t* object_manager;
};

/*
 * surgescript_vm_create()
 * Creates a vm
 */
surgescript_vm_t* surgescript_vm_create()
{
    surgescript_vm_t* vm = ssmalloc(sizeof *vm);

    vm->stack = surgescript_stack_create();
    vm->program_pool = surgescript_programpool_create();
    vm->tag_system = surgescript_tagsystem_create();
    vm->object_manager = surgescript_objectmanager_create(vm->program_pool, vm->tag_system, vm->stack);

    return vm;
}

/*
 * surgescript_vm_destroy()
 * Destroys a VM
 */
surgescript_vm_t* surgescript_vm_destroy(surgescript_vm_t* vm)
{
    surgescript_objectmanager_destroy(vm->object_manager);
    surgescript_tagsystem_destroy(vm->tag_system);
    surgescript_programpool_destroy(vm->program_pool);
    surgescript_stack_destroy(vm->stack);
    return ssfree(vm);
}

/*
 * surgescript_vm_launch()
 * Boots up the vm
 */
void surgescript_vm_launch(surgescript_vm_t* vm)
{
    /* SurgeScript uses UTF-8 */
    setlocale(LC_ALL, "en_US.UTF-8");

    /* Load the surgescript library */
    surgescript_sslib_register_object(vm);
    surgescript_sslib_register_string(vm);
    surgescript_sslib_register_number(vm);
    surgescript_sslib_register_boolean(vm);
    surgescript_sslib_register_math(vm);
    surgescript_sslib_register_array(vm);
    surgescript_sslib_register_dictionary(vm);
    surgescript_sslib_register_transform2d(vm);
    surgescript_sslib_register_console(vm);
    surgescript_sslib_register_application(vm);
    surgescript_sslib_register_system(vm);

    /* Create the root object */
    surgescript_objectmanager_spawn_root(vm->object_manager);
}

/*
 * surgescript_vm_is_active()
 * Is the VM active?
 */
bool surgescript_vm_is_active(surgescript_vm_t* vm)
{
    surgescript_objecthandle_t root_handle = surgescript_objectmanager_root(vm->object_manager);
    return surgescript_objectmanager_exists(vm->object_manager, root_handle);
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
        surgescript_objectmanager_collectgarbage(vm->object_manager);
        return surgescript_vm_is_active(vm);
    }
    else
        return false;
}

/*
 * surgescript_vm_kill()
 * terminates the vm
 */
void surgescript_vm_kill(surgescript_vm_t* vm)
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
    surgescript_objecthandle_t handle = surgescript_object_find_child(root, object_name);
    return surgescript_objectmanager_get(vm->object_manager, handle);
}

/*
 * surgescript_vm_bind()
 * Binds a C function to a SurgeScript object
 */
void surgescript_vm_bind(surgescript_vm_t* vm, const char* object_name, const char* fun_name, surgescript_program_cfunction_t cfun, int num_params)
{
    surgescript_program_t* cprogram = surgescript_cprogram_create(num_params, cfun);
    surgescript_programpool_put(vm->program_pool, object_name, fun_name, cprogram);
}