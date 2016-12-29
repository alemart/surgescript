/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/renv.c
 * SurgeScript runtime environment (used to execute surgescript programs)
 */

#include "renv.h"
#include "variable.h"
#include "heap.h"
#include "stack.h"
#include "object.h"
#include "program_pool.h"
#include "object_manager.h"
#include "../util/util.h"

/* how many temporary vars does a runtime environment have? */
static const int MAX_TMPVARS = 4; /* used for calculations. the last one is used as a return value (for functions) */


/*
 * surgescript_renv_create()
 * Creates a runtime environment
 */
const surgescript_renv_t* surgescript_renv_create(surgescript_object_t* owner, surgescript_stack_t* stack, surgescript_heap_t* heap, surgescript_programpool_t* program_pool, surgescript_objectmanager_t* object_manager)
{
    int i;
    surgescript_renv_t* runtime_environment = ssmalloc(sizeof *runtime_environment);

    runtime_environment->owner = owner; 
    runtime_environment->stack = stack;
    runtime_environment->heap = heap;
    runtime_environment->program_pool = program_pool;
    runtime_environment->object_manager = object_manager;
    runtime_environment->tmp = ssmalloc(MAX_TMPVARS * sizeof *(runtime_environment->tmp));
    for(i = 0; i < MAX_TMPVARS; i++)
        runtime_environment->tmp[i] = surgescript_var_create();

    return runtime_environment;
}

/*
 * surgescript_renv_clone()
 * Clones a runtime environment
 */
const surgescript_renv_t* surgescript_renv_clone(const surgescript_renv_t* runtime_environment)
{
    int i;
    surgescript_renv_t* clone = ssmalloc(sizeof *clone);

    clone->owner = runtime_environment->owner;
    clone->stack = runtime_environment->stack;
    clone->heap = runtime_environment->heap;
    clone->program_pool = runtime_environment->program_pool;
    clone->object_manager = runtime_environment->object_manager;
    clone->tmp = ssmalloc(MAX_TMPVARS * sizeof *(clone->tmp));
    for(i = 0; i < MAX_TMPVARS; i++)
        clone->tmp[i] = surgescript_var_create();

    return clone;
}

/*
 * surgescript_renv_destroy()
 * Destroys a runtime environment
 */
const surgescript_renv_t* surgescript_renv_destroy(const surgescript_renv_t* runtime_environment)
{
    int i;
    for(i = 0; i < MAX_TMPVARS; i++)
        surgescript_var_destroy(runtime_environment->tmp[i]);
    ssfree(runtime_environment->tmp);
    ssfree((surgescript_renv_t*)runtime_environment);
    return NULL;
}

/* inline getters */
extern struct surgescript_object_t* surgescript_renv_owner(const surgescript_renv_t* renv);
extern struct surgescript_stack_t* surgescript_renv_stack(const surgescript_renv_t* renv);
extern struct surgescript_heap_t* surgescript_renv_heap(const surgescript_renv_t* renv);
extern struct surgescript_programpool_t* surgescript_renv_programpool(const surgescript_renv_t* renv);
extern struct surgescript_objectmanager_t* surgescript_renv_objectmanager(const surgescript_renv_t* renv);
extern struct surgescript_var_t** surgescript_renv_tmp(const surgescript_renv_t* renv);