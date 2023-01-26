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
static const int MAX_TMPVARS = 4; /* used for calculations */
static surgescript_renv_t* full_destructor(surgescript_renv_t* runtime_environment);
static surgescript_renv_t* partial_destructor(surgescript_renv_t* runtime_environment);


/*
 * surgescript_renv_create()
 * Creates a runtime environment
 */
surgescript_renv_t* surgescript_renv_create(surgescript_object_t* owner, surgescript_stack_t* stack, surgescript_heap_t* heap, surgescript_programpool_t* program_pool, surgescript_objectmanager_t* object_manager, surgescript_var_t** tmp)
{
    surgescript_renv_t* runtime_environment = ssmalloc(sizeof *runtime_environment);

    runtime_environment->owner = owner; 
    runtime_environment->stack = stack;
    runtime_environment->heap = heap;
    runtime_environment->program_pool = program_pool;
    runtime_environment->object_manager = object_manager;
    runtime_environment->caller = surgescript_objectmanager_null(object_manager);

    if(!tmp) {
        int i;
        runtime_environment->tmp = ssmalloc(MAX_TMPVARS * sizeof *(runtime_environment->tmp));
        for(i = 0; i < MAX_TMPVARS; i++)
            runtime_environment->tmp[i] = surgescript_var_create();
        runtime_environment->_destructor = full_destructor;
    }
    else {
        runtime_environment->tmp = tmp;
        surgescript_var_set_null(runtime_environment->tmp[3]);
        runtime_environment->_destructor = partial_destructor;
    }

    return runtime_environment;
}

/*
 * surgescript_renv_destroy()
 * Destroys a runtime environment
 */
surgescript_renv_t* surgescript_renv_destroy(surgescript_renv_t* runtime_environment)
{
    return runtime_environment->_destructor(runtime_environment);
}


/* privates */

surgescript_renv_t* full_destructor(surgescript_renv_t* runtime_environment)
{
    for(int i = 0; i < MAX_TMPVARS; i++)
        surgescript_var_destroy(runtime_environment->tmp[i]);
    ssfree(runtime_environment->tmp);
    return ssfree(runtime_environment);
}

surgescript_renv_t* partial_destructor(surgescript_renv_t* runtime_environment)
{
    return ssfree(runtime_environment);
}