/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/vm.h
 * SurgeScript Virtual Machine for the Runtime Engine
 */

#ifndef _SURGESCRIPT_RUNTIME_VM_H
#define _SURGESCRIPT_RUNTIME_VM_H

#include <stdbool.h>
#include "program.h"
#include "object.h"
#include "program_pool.h"
#include "tag_system.h"
#include "object_manager.h"
#include "variable.h"

/* types */
typedef struct surgescript_vm_t surgescript_vm_t;
struct surgescript_parser_t;

/* api */
surgescript_vm_t* surgescript_vm_create();
surgescript_vm_t* surgescript_vm_destroy(surgescript_vm_t* vm);

/* SurgeScript Compiler */
bool surgescript_vm_compile(surgescript_vm_t* vm, const char* absolute_path); /* compiles a file */
bool surgescript_vm_compile_code_in_memory(surgescript_vm_t* vm, const char* code); /* compiles the given code */

/* VM lifecycle */
bool surgescript_vm_is_active(surgescript_vm_t* vm); /* is the vm active? */
void surgescript_vm_launch(surgescript_vm_t* vm); /* boots up the vm */
void surgescript_vm_terminate(surgescript_vm_t* vm); /* terminates the vm */
bool surgescript_vm_update(surgescript_vm_t* vm); /* updates the vm */
bool surgescript_vm_update_ex(surgescript_vm_t* vm, void* user_data, void (*user_update)(surgescript_object_t*,void*), void (*late_update)(surgescript_object_t*,void*)); /* updates the vm and allows more callbacks */

/* VM components */
surgescript_programpool_t* surgescript_vm_programpool(const surgescript_vm_t* vm); /* gets the program pool */
surgescript_tagsystem_t* surgescript_vm_tagsystem(const surgescript_vm_t* vm); /* gets the tag system */
surgescript_objectmanager_t* surgescript_vm_objectmanager(const surgescript_vm_t* vm); /* gets the object manager */
struct surgescript_parser_t* surgescript_vm_parser(const surgescript_vm_t* vm); /* gets the parser */

/* utilities */
surgescript_object_t* surgescript_vm_root_object(surgescript_vm_t* vm); /* root object */
surgescript_object_t* surgescript_vm_spawn_object(surgescript_vm_t* vm, surgescript_object_t* parent, const char* object_name, void* user_data); /* user_data may be NULL */
surgescript_object_t* surgescript_vm_find_object(surgescript_vm_t* vm, const char* object_name); /* finds an object */
void surgescript_vm_bind(surgescript_vm_t* vm, const char* object_name, const char* fun_name, surgescript_program_cfunction_t cfun, int num_params); /* binds a C function to an object */

#endif