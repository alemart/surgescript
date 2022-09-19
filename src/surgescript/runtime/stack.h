/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2022 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/stack.h
 * SurgeScript stack
 */

#ifndef _SURGESCRIPT_RUNTIME_STACK_H
#define _SURGESCRIPT_RUNTIME_STACK_H

#include <stdlib.h>
#include <stdbool.h>

/* types */
typedef struct surgescript_stack_t surgescript_stack_t;
struct surgescript_stack_t;
typedef int surgescript_stackptr_t;

/* forward declarations */
struct surgescript_var_t;

/* public methods */
surgescript_stack_t* surgescript_stack_create();
surgescript_stack_t* surgescript_stack_destroy(surgescript_stack_t* stack);
void surgescript_stack_push(surgescript_stack_t* stack, struct surgescript_var_t* data); /* pushes data to the stack */
void surgescript_stack_pop(surgescript_stack_t* stack); /* pops and deallocates a var from the stack */
void surgescript_stack_pushenv(surgescript_stack_t* stack); /* pushes an environment */
void surgescript_stack_popenv(surgescript_stack_t* stack); /* pops an environment */
void surgescript_stack_pushn(surgescript_stack_t* stack, size_t n); /* pushes n empty variables to the stack */
void surgescript_stack_popn(surgescript_stack_t* stack, size_t n); /* pops n variables from the stack */
const struct surgescript_var_t* surgescript_stack_top(const surgescript_stack_t* stack); /* gets the topmost element */
const struct surgescript_var_t* surgescript_stack_peek(const surgescript_stack_t* stack, surgescript_stackptr_t offset); /* reads stack[base + offset] */
void surgescript_stack_poke(surgescript_stack_t* stack, surgescript_stackptr_t offset, const struct surgescript_var_t* data); /* writes data on stack[base + offset] */
int surgescript_stack_empty(const surgescript_stack_t* stack); /* is the stack empty? */
void surgescript_stack_scan_objects(surgescript_stack_t* stack, void* userdata, bool (*callback)(unsigned,void*));
size_t surgescript_stack_size(const surgescript_stack_t* stack); /* stack size */

#endif
