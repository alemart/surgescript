/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/runtime_environment.c
 * SurgeScript runtime environment (used to execute surgescript programs)
 */

#include "runtime_environment.h"
#include "variable.h"
#include "heap.h"
#include "stack.h"
#include "object.h"
#include "program_pool.h"
#include "object_pool.h"
#include "../util/util.h"

/* how many temporary vars does a runtime environment have? */
static const int MAX_TMPVARS = 3;


/*
 * surgescript_program_runtimeenv_create()
 * Creates a runtime environment
 */
surgescript_program_runtimeenv_t* surgescript_program_runtimeenv_create(surgescript_object_t* self, surgescript_stack_t* stack, surgescript_heap_t* heap, surgescript_programpool_t* program_pool, surgescript_objectpool_t* object_pool)
{
    int i;
    surgescript_program_runtimeenv_t* runtime_environment = ssmalloc(sizeof *runtime_environment);

    runtime_environment->self = self;
    runtime_environment->stack = stack;
    runtime_environment->heap = heap;
    runtime_environment->program_pool = program_pool;
    runtime_environment->object_pool = object_pool;
    runtime_environment->tmp = ssmalloc(MAX_TMPVARS * sizeof *(runtime_environment->tmp));
    for(i = 0; i < MAX_TMPVARS; i++)
        runtime_environment->tmp[i] = surgescript_var_create();

    return runtime_environment;
}

/*
 * surgescript_program_runtimeenv_clone()
 * Clones a runtime environment
 */
surgescript_program_runtimeenv_t* surgescript_program_runtimeenv_clone(const surgescript_program_runtimeenv_t* runtime_environment)
{
    int i;
    surgescript_program_runtimeenv_t* clone = ssmalloc(sizeof *clone);

    clone->self = runtime_environment->self;
    clone->stack = runtime_environment->stack;
    clone->heap = runtime_environment->heap;
    clone->program_pool = runtime_environment->program_pool;
    clone->object_pool = runtime_environment->object_pool;
    clone->tmp = ssmalloc(MAX_TMPVARS * sizeof *(clone->tmp));
    for(i = 0; i < MAX_TMPVARS; i++)
        clone->tmp[i] = surgescript_var_create();

    return clone;
}

/*
 * surgescript_program_runtimeenv_destroy()
 * Destroys a runtime environment
 */
surgescript_program_runtimeenv_t* surgescript_program_runtimeenv_destroy(surgescript_program_runtimeenv_t* runtime_environment)
{
    int i;
    for(i = 0; i < MAX_TMPVARS; i++)
        surgescript_var_destroy(runtime_environment->tmp[i]);
    ssfree(runtime_environment->tmp);
    ssfree(runtime_environment);
    return NULL;
}
