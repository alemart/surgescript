/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
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
    struct surgescript_object_t* owner; /* pointer to the object this program refers to (i.e., the "owner") */
    struct surgescript_stack_t* stack; /* pointer to the stack */
    struct surgescript_heap_t* heap; /* pointer to the heap */
    struct surgescript_programpool_t* program_pool; /* pointer to the program pool */
    struct surgescript_objectmanager_t* object_manager; /* pointer to the object manager */
    struct surgescript_var_t** tmp; /* temporary variables */
} surgescript_renv_t ;

/* creates a new renv */
const surgescript_renv_t* surgescript_renv_create(struct surgescript_object_t* owner, struct surgescript_stack_t* stack, struct surgescript_heap_t* heap, struct surgescript_programpool_t* program_pool, struct surgescript_objectmanager_t* object_manager);

/* clones an existing renv (the temporary variables are allocated anew) */
const surgescript_renv_t* surgescript_renv_clone(const surgescript_renv_t* runtime_environment);

/* destroys a renv */
const surgescript_renv_t* surgescript_renv_destroy(const surgescript_renv_t* runtime_environment);

/* getters */
inline struct surgescript_object_t* surgescript_renv_owner(const surgescript_renv_t* renv) { return renv->owner; }
inline struct surgescript_stack_t* surgescript_renv_stack(const surgescript_renv_t* renv) { return renv->stack; }
inline struct surgescript_heap_t* surgescript_renv_heap(const surgescript_renv_t* renv) { return renv->heap; }
inline struct surgescript_programpool_t* surgescript_renv_programpool(const surgescript_renv_t* renv) { return renv->program_pool; }
inline struct surgescript_objectmanager_t* surgescript_renv_objectmanager(const surgescript_renv_t* renv) { return renv->object_manager; }
inline struct surgescript_var_t** surgescript_renv_tmp(const surgescript_renv_t* renv) { return renv->tmp; }

#endif
