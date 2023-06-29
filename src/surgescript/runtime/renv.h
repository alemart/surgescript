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
 * runtime/renv.h
 * SurgeScript runtime environment (used to execute surgescript programs)
 */

#ifndef _SURGESCRIPT_RUNTIME_RENV_H
#define _SURGESCRIPT_RUNTIME_RENV_H

/* types */
struct surgescript_object_t;
struct surgescript_stack_t;
struct surgescript_heap_t;
struct surgescript_programpool_t;
struct surgescript_objectmanager_t;

/* a program, to be run, needs a runtime environment (renv) */
/* this is composed by an owner object, plus heap-stack-etc, plus some unique temporary variables */
/* --- instead of messing with this directly, use the functions below --- */
typedef struct surgescript_renv_t
{
    struct surgescript_object_t* owner; /* pointer to the object the program refers to (i.e., the "owner") */
    struct surgescript_stack_t* stack; /* pointer to the stack */
    struct surgescript_heap_t* heap; /* pointer to the heap */
    struct surgescript_programpool_t* program_pool; /* pointer to the program pool */
    struct surgescript_objectmanager_t* object_manager; /* pointer to the object manager */
    struct surgescript_var_t** tmp; /* temporary variables */
    struct surgescript_renv_t* (*_destructor)(struct surgescript_renv_t*); /* internal destructor */
    unsigned caller; /* handle to the object that called this program */
} surgescript_renv_t ;

/* creates a new renv (the tmp parameter may be NULL) */
surgescript_renv_t* surgescript_renv_create(struct surgescript_object_t* owner, struct surgescript_stack_t* stack, struct surgescript_heap_t* heap, struct surgescript_programpool_t* program_pool, struct surgescript_objectmanager_t* object_manager, struct surgescript_var_t** tmp);

/* destroys a renv */
surgescript_renv_t* surgescript_renv_destroy(surgescript_renv_t* runtime_environment);

/* getters */
#define surgescript_renv_owner(renv)            ((renv)->owner)
#define surgescript_renv_stack(renv)            ((renv)->stack)
#define surgescript_renv_heap(renv)             ((renv)->heap)
#define surgescript_renv_programpool(renv)      ((renv)->program_pool)
#define surgescript_renv_objectmanager(renv)    ((renv)->object_manager)
#define surgescript_renv_tmp(renv)              ((renv)->tmp)
#define surgescript_renv_caller(renv)           ((renv)->caller)

#endif