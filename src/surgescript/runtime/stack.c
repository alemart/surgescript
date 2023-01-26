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
 * runtime/stack.c
 * SurgeScript stack
 */

#include "stack.h"
#include "variable.h"
#include "../util/util.h"

/*
 * this is the stack (with 2 envs):
 * +---------------------+
 * |      top data       | <-- SP
 * |     some data       |
 * |     some data       |
 * |     some data       |
 * |     some data       |
 * |     some data       |
 * |    previous BP      | <-- BP
 * +---------------------+
 * |     some data       |
 * |     some data       |
 * |   previous BP (0)   |
 * +---------------------+
 */

/* constants */
static const size_t SSSTACK_INITIAL_SIZE = 65536; /* 64K */

/* the stack structure */
struct surgescript_stack_t
{
    size_t size;                     /* size of the stack */
    surgescript_stackptr_t sp, bp;   /* pointers */
    surgescript_var_t** data;        /* stack data */
};


/* -------------------------------
 * public methods
 * ------------------------------- */

/*
 * surgescript_stack_create()
 * Creates a new stack
 */
surgescript_stack_t* surgescript_stack_create()
{
    surgescript_stack_t* stack = ssmalloc(sizeof *stack);
    size_t size = SSSTACK_INITIAL_SIZE;

    stack->data = ssmalloc(size * sizeof(*(stack->data)));
    stack->size = size;
    stack->sp = stack->bp = 0;
    while(size)
        stack->data[--size] = NULL;

    stack->data[0] = surgescript_var_set_rawbits(surgescript_var_create(), stack->bp);
    return stack;
}

/*
 * surgescript_stack_destroy()
 * Destroys an existing stack
 */
surgescript_stack_t* surgescript_stack_destroy(surgescript_stack_t* stack)
{
    for(surgescript_stackptr_t i = stack->size - 1; i >= 0; i--) {
        if(stack->data[i] != NULL)
            surgescript_var_destroy(stack->data[i]);
    }

    ssfree(stack->data);
    ssfree(stack);
    return NULL;
}

/*
 * surgescript_stack_push()
 * Pushes a variable onto the stack
 */
void surgescript_stack_push(surgescript_stack_t* stack, surgescript_var_t* data)
{
    if(++stack->sp < stack->size)
        stack->data[stack->sp] = data;
    else
        ssfatal("Runtime Error: surgescript_stack_push() - stack overflow");
}

/*
 * surgescript_stack_pop()
 * Pops a variable from the stack, deallocating it
 */
void surgescript_stack_pop(surgescript_stack_t* stack)
{
    if(stack->sp > stack->bp) {
        stack->data[stack->sp] = surgescript_var_destroy(stack->data[stack->sp]);
        stack->sp--;
    }
    else
        ssfatal("Runtime Error: can't surgescript_stack_pop() - empty stack");
}

/*
 * surgescript_stack_pushenv()
 * Pushes an environment to the stack
 */
void surgescript_stack_pushenv(surgescript_stack_t* stack)
{
    /* push prev BP & set new BP */
    surgescript_var_t* prev_bp = surgescript_var_set_rawbits(surgescript_var_create(), stack->bp);
    surgescript_stack_push(stack, prev_bp);
    stack->bp = stack->sp; /* the base of the stack points to the previous bp */
}

/*
 * surgescript_stack_popenv()
 * Pops an environment
 */
void surgescript_stack_popenv(surgescript_stack_t* stack)
{
    if(stack->sp > 0) {
        /* get previous bp & deallocate everything in between */
        surgescript_stackptr_t i, prev_bp = surgescript_var_get_rawbits(stack->data[stack->bp]);
        for(i = stack->sp; i >= stack->bp; i--) {
            if(stack->data[i] != NULL)
                stack->data[i] = surgescript_var_destroy(stack->data[i]);
        }

        stack->sp = stack->bp - 1;
        stack->bp = prev_bp;
    }
    else
        ssfatal("Runtime Error: surgescript_stack_popenv() has found an empty stack");
}

/*
 * surgescript_stack_pushn()
 * pushes n empty variables to the stack
 */
void surgescript_stack_pushn(surgescript_stack_t* stack, size_t n)
{
    while(n--)
        surgescript_stack_push(stack, surgescript_var_create());
}

/*
 * surgescript_stack_popn()
 * pops n variables from the stack
 */
void surgescript_stack_popn(surgescript_stack_t* stack, size_t n)
{
    while(n--)
        surgescript_stack_pop(stack);
}

/*
 * surgescript_stack_top()
 * Gets the top element from the stack
 */
const surgescript_var_t* surgescript_stack_top(const surgescript_stack_t* stack)
{
    return stack->data[stack->sp];
}


/*
 * surgescript_stack_peek()
 * Reads the (base+offset)-th element from the stack
 */
const surgescript_var_t* surgescript_stack_peek(const surgescript_stack_t* stack, surgescript_stackptr_t offset)
{
    const surgescript_stackptr_t idx = stack->bp + offset;

    if(idx >= 0 && idx <= stack->sp)
        return stack->data[idx];

    ssfatal("Runtime Error: surgescript_stack_peek() can't read an element (%d) that is out of bounds [%d, %d]", idx, 0, stack->sp);
    return NULL;
}

/*
 * surgescript_stack_poke()
 * Writes data on stack[base+offset]
 */
void surgescript_stack_poke(surgescript_stack_t* stack, surgescript_stackptr_t offset, const surgescript_var_t* data)
{
    const surgescript_stackptr_t idx = stack->bp + offset;

    if(idx >= 0 && idx <= stack->sp)
        surgescript_var_copy(stack->data[idx], data);
    else
        ssfatal("Runtime Error: surgescript_stack_poke() can't write to an element (%d) that is out of bounds [%d, %d]", idx, 0, stack->sp);
}

/*
 * surgescript_stack_empty()
 * Is the stack empty?
 */
int surgescript_stack_empty(const surgescript_stack_t* stack)
{
    return stack->sp <= stack->bp;
}

/*
 * surgescript_stack_scan_objects()
 * For each object in the stack, call callback with its handle
 */
void surgescript_stack_scan_objects(surgescript_stack_t* stack, void* userdata, bool (*callback)(unsigned,void*))
{
    for(surgescript_stackptr_t i = stack->sp - 1; i >= 0; i--) { /* check all environments */
        if(stack->data[i] != NULL) {
            unsigned handle = surgescript_var_get_objecthandle(stack->data[i]);
            if(handle != 0) { /* if it is an object and not null */
                if(!callback(handle, userdata)) /* if the handle is broken */
                    surgescript_var_set_null(stack->data[i]); /* fix it */
            }
        }
    }
}

/*
 * surgescript_stack_size()
 * Returns the stack size
 */
size_t surgescript_stack_size(const surgescript_stack_t* stack)
{
    return stack->sp;
}