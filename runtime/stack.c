/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
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
static const size_t SSSTACK_INITIAL_SIZE = 20480 * 2;

/* the stack structure */
struct surgescript_stack_t
{
    size_t size;                /* size of the stack */
    int sp, bp;                 /* pointers */
    surgescript_var_t** data;   /* stack data */
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

    stack->data[0] = surgescript_var_set_number(surgescript_var_create(), stack->bp);
    return stack;
}

/*
 * surgescript_stack_destroy()
 * Destroys an existing stack
 */
surgescript_stack_t* surgescript_stack_destroy(surgescript_stack_t* stack)
{
    int i;

    for(i = stack->size - 1; i >= 0; i--) {
        if(stack->data[i] != NULL)
            surgescript_var_destroy(stack->data[i]);
    }

    ssfree(stack->data);
    ssfree(stack);
    return NULL;
}

/*
 * surgescript_stack_push()
 * Pushes a variable onto the stack (the var is automatically allocated)
 */
surgescript_var_t* surgescript_stack_push(surgescript_stack_t* stack)
{
    if(++stack->sp >= stack->size) {
        ssfatal("surgescript_stack_push(): stack overflow.");
        return NULL;
    }

    return stack->data[stack->sp] = surgescript_var_create();
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
        ssfatal("surgescript_stack_pop(): empty stack.");
}

/*
 * surgescript_stack_pushenv()
 * Pushes an environment to the stack, allocating num_vars variables
 */
void surgescript_stack_pushenv(surgescript_stack_t* stack, int num_vars)
{
    /* push prev BP & set new BP */
    surgescript_var_t* prev_bp = surgescript_stack_push(stack);
    surgescript_var_set_number(prev_bp, stack->bp);
    stack->bp = stack->sp;

    /* allocates the variables */
    while(num_vars--)
        surgescript_stack_push(stack);
}

/*
 * surgescript_stack_popenv()
 * Pops an environment
 */
void surgescript_stack_popenv(surgescript_stack_t* stack)
{
    if(stack->sp > 0) {
        /* get previous bp & deallocate everything in between */
        int i, prev_bp = surgescript_var_get_number(stack->data[stack->bp]);
        for(i = stack->sp; i >= stack->bp; i--) {
            if(stack->data[i] != NULL)
                stack->data[i] = surgescript_var_destroy(stack->data[i]);
        }

        stack->sp = stack->bp - 1;
        stack->bp = prev_bp;
    }
    else
        ssfatal("surgescript_stack_popenv(): empty stack.");
}


/*
 * surgescript_stack_top()
 * Gets the top element from the stack (minus some positive offset)
 */
surgescript_var_t* surgescript_stack_top(surgescript_stack_t* stack)
{
    return stack->data[stack->sp];
}


/*
 * surgescript_stack_at()
 * Gets the (base+offset)-th element from the stack, offset > 0
 */
surgescript_var_t* surgescript_stack_at(surgescript_stack_t* stack, int offset)
{
    if(offset > 0 && stack->bp + offset <= stack->sp)
        return stack->data[stack->bp + offset];

    ssfatal("surgescript_stack_at(): can't get an element (%d) that is out of bounds [%d, %d].", stack->bp + offset, stack->bp + 1, stack->sp);
    return NULL;
}

/*
 * surgescript_stack_empty()
 * Is the stack empty?
 */
int surgescript_stack_empty(surgescript_stack_t* stack)
{
    return stack->sp <= stack->bp;
}
