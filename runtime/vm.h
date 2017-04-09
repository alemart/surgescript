/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/vm.h
 * SurgeScript Virtual Machine for the Runtime Engine
 */

#ifndef _SURGESCRIPT_RUNTIME_VM_H
#define _SURGESCRIPT_RUNTIME_VM_H

#include <stdbool.h>
#include "program.h"
#include "object.h"
#include "program_pool.h"
#include "object_manager.h"
#include "variable.h"

/* types */
typedef struct surgescript_vm_t surgescript_vm_t;

/* api */
surgescript_vm_t* surgescript_vm_create();
surgescript_vm_t* surgescript_vm_destroy(surgescript_vm_t* vm);

/* bool surgescript_vm_compile(surgescript_vm_t* vm, ...); */

void surgescript_vm_launch(surgescript_vm_t* vm); /* boots up the vm */
bool surgescript_vm_update(surgescript_vm_t* vm); /* updates the vm */
void surgescript_vm_kill(surgescript_vm_t* vm); /* terminates the vm */
bool surgescript_vm_is_active(surgescript_vm_t* vm); /* is the vm active? */

surgescript_programpool_t* surgescript_vm_programpool(const surgescript_vm_t* vm); /* gets the program pool */
surgescript_objectmanager_t* surgescript_vm_objectmanager(const surgescript_vm_t* vm); /* gets the object manager */

surgescript_object_t* surgescript_vm_root_object(surgescript_vm_t* vm); /* root object */
surgescript_object_t* surgescript_vm_spawn_object(surgescript_vm_t* vm, surgescript_object_t* parent, const char* object_name, void* user_data); /* user_data may be NULL */
surgescript_object_t* surgescript_vm_find_object(surgescript_vm_t* vm, const char* object_name); /* finds an object */
void surgescript_vm_bind(surgescript_vm_t* vm, const char* object_name, const char* fun_name, surgescript_program_cfunction_t cfun, int num_params); /* binds a C function to an object */

#endif