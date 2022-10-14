/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2022  Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/vm.h
 * SurgeScript Virtual Machine - Runtime Engine
 */

#ifndef _SURGESCRIPT_RUNTIME_VM_H
#define _SURGESCRIPT_RUNTIME_VM_H

#include <stdint.h>
#include <stdbool.h>
#include "program.h"
#include "object.h"

/* types */
typedef struct surgescript_vm_t surgescript_vm_t;
struct surgescript_parser_t;
struct surgescript_programpool_t;
struct surgescript_objectmanager_t;
struct surgescript_tagsystem_t;
struct surgescript_vmargs_t;
struct surgescript_vmtime_t;

/* api */
surgescript_vm_t* surgescript_vm_create();
surgescript_vm_t* surgescript_vm_destroy(surgescript_vm_t* vm);

/* SurgeScript Compiler */
bool surgescript_vm_compile(surgescript_vm_t* vm, const char* absolute_path); /* compiles a file */
bool surgescript_vm_compile_code_in_memory(surgescript_vm_t* vm, const char* code); /* compiles the given code */
bool surgescript_vm_compile_virtual_file(surgescript_vm_t* vm, const char* code, const char* filename); /* compiles the given code specifying a virtual filename */

/* VM lifecycle */
bool surgescript_vm_is_active(surgescript_vm_t* vm); /* is the vm active? (i.e., turned on) */
void surgescript_vm_launch(surgescript_vm_t* vm); /* boots up the vm */
void surgescript_vm_launch_ex(surgescript_vm_t* vm, int argc, char** argv); /* boots up the vm with command line arguments */
void surgescript_vm_terminate(surgescript_vm_t* vm); /* terminates the vm */
bool surgescript_vm_reset(surgescript_vm_t* vm); /* resets the VM, clearing up all its programs and objects */
bool surgescript_vm_update(surgescript_vm_t* vm); /* updates the vm */
bool surgescript_vm_update_ex(surgescript_vm_t* vm, void* user_data, void (*user_update)(surgescript_object_t*,void*), void (*late_update)(surgescript_object_t*,void*)); /* updates the vm and allows more callbacks */
void surgescript_vm_pause(surgescript_vm_t* vm); /* pause the VM */
void surgescript_vm_resume(surgescript_vm_t* vm); /* resume a paused VM */
bool surgescript_vm_is_paused(const surgescript_vm_t* vm); /* is the VM paused? */

/* VM components */
struct surgescript_programpool_t* surgescript_vm_programpool(const surgescript_vm_t* vm); /* gets the program pool */
struct surgescript_tagsystem_t* surgescript_vm_tagsystem(const surgescript_vm_t* vm); /* gets the tag system */
struct surgescript_objectmanager_t* surgescript_vm_objectmanager(const surgescript_vm_t* vm); /* gets the object manager */
struct surgescript_parser_t* surgescript_vm_parser(const surgescript_vm_t* vm); /* gets the parser */
const struct surgescript_vmargs_t* surgescript_vm_args(const surgescript_vm_t* vm); /* gets the command-line arguments */
const struct surgescript_vmtime_t* surgescript_vm_time(const surgescript_vm_t* vm); /* gets the VM time */

/* utilities */
surgescript_object_t* surgescript_vm_root_object(surgescript_vm_t* vm); /* root object */
surgescript_object_t* surgescript_vm_spawn_object(surgescript_vm_t* vm, surgescript_object_t* parent, const char* object_name, void* user_data); /* user_data may be NULL */
surgescript_object_t* surgescript_vm_find_object(surgescript_vm_t* vm, const char* object_name); /* finds an object */
void surgescript_vm_bind(surgescript_vm_t* vm, const char* object_name, const char* fun_name, surgescript_program_cfunction_t cfun, int num_params); /* binds a C function to an object */
void surgescript_vm_install_plugin(surgescript_vm_t* vm, const char* object_name); /* sets a certain object as a plugin */

#endif
