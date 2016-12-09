/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/runtime_environment.h
 * SurgeScript runtime environment (used to execute surgescript programs)
 */

#ifndef _SURGESCRIPT_RUNTIME_RUNTIMEENVIRONMENT_H
#define _SURGESCRIPT_RUNTIME_RUNTIMEENVIRONMENT_H

/* types */
struct surgescript_object_t;
struct surgescript_stack_t;
struct surgescript_heap_t;
struct surgescript_programpool_t;
struct surgescript_objectpool_t;

/* a program, to be run, needs a runtime environment */
typedef struct surgescript_program_runtimeenv_t surgescript_program_runtimeenv_t;
struct surgescript_program_runtimeenv_t
{
    struct surgescript_object_t* self; /* pointer to the object this program refers to (i.e., the "owner") */
    struct surgescript_stack_t* stack; /* pointer to the stack */
    struct surgescript_heap_t* heap; /* pointer to the heap */
    struct surgescript_programpool_t* program_pool; /* pointer to the program pool */
    struct surgescript_objectpool_t* object_pool; /* pointer to the object pool */
    struct surgescript_var_t** tmp; /* temporary variables */
};

/* api */
surgescript_program_runtimeenv_t* surgescript_program_runtimeenv_create(struct surgescript_object_t* self, struct surgescript_stack_t* stack, struct surgescript_heap_t* heap, struct surgescript_programpool_t* program_pool, struct surgescript_objectpool_t* object_pool);
surgescript_program_runtimeenv_t* surgescript_program_runtimeenv_clone(const surgescript_program_runtimeenv_t* runtime_environment);
surgescript_program_runtimeenv_t* surgescript_program_runtimeenv_destroy(surgescript_program_runtimeenv_t* runtime_environment);

#endif
