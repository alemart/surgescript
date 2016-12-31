/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/program.c
 * SurgeScript program
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "program.h"
#include "variable.h"
#include "heap.h"
#include "stack.h"
#include "object.h"
#include "renv.h"
#include "object_manager.h"
#include "program_pool.h"
#include "../util/util.h"
#include "../util/ssarray.h"

/* forward declarations */
typedef struct surgescript_program_operation_t surgescript_program_operation_t;
/*typedef struct surgescript_program_delayedcalls_t surgescript_program_delayedcalls_t;*/


/* the program structure */
struct surgescript_program_t
{
    int arity, num_local_vars; /* config */
    int ip; /* instruction pointer */
    SSARRAY(surgescript_program_operation_t, line); /* a set of operations (or lines of code) */
    SSARRAY(surgescript_program_label_t, label); /* labels (label[j] is the index of a line of code, j is a label) */
    SSARRAY(char*, text); /* read-only text data */
    /*surgescript_program_delayedcalls_t* delayedcalls;*/ /* used for rendering */
};

/* an operation / command */
struct surgescript_program_operation_t
{
    surgescript_program_operator_t instruction;
    surgescript_program_operand_t a, b, c;
};

static inline void run_instruction(surgescript_program_t* program, surgescript_renv_t* runtime_environment, surgescript_program_operator_t instruction, surgescript_program_operand_t a, surgescript_program_operand_t b, surgescript_program_operand_t c);

/* delayed BUILT-IN calls (used for rendering) */
/* calls fun(self, params, num_params) when called */
/*struct surgescript_program_delayedcalls_t
{
    void (*fun)(surgescript_object_t*, surgescript_var_t**, int);
    surgescript_var_t** param;
    int num_params;
    surgescript_program_delayedcalls_t* next;
};

static surgescript_program_delayedcalls_t* surgescript_program_delayedcalls_create();
static surgescript_program_delayedcalls_t* surgescript_program_delayedcalls_destroy(surgescript_program_delayedcalls_t* delayedcalls);
static surgescript_program_delayedcalls_t* surgescript_program_delayedcalls_add(surgescript_program_delayedcalls_t* delayedcalls, void (*fun)(surgescript_object_t*, surgescript_var_t**, int), surgescript_var_t** param, int num_params);
static void surgescript_program_delayedcalls_run(surgescript_program_delayedcalls_t* delayedcalls, surgescript_object_t* self);*/

/* -------------------------------
 * public methods
 * ------------------------------- */

/*
 * surgescript_program_create()
 * Creates a new program (aka function call)
 */
surgescript_program_t* surgescript_program_create(int arity, int num_local_vars)
{
    surgescript_program_t* program = ssmalloc(sizeof *program);

    program->arity = arity;
    program->num_local_vars = num_local_vars;
    program->ip = 0;

    ssarray_init(program->line);
    ssarray_init(program->label);
    ssarray_init(program->text);
    /*program->delayedcalls = NULL;*/

    return program;
}

/*
 * surgescript_program_destroy()
 * Destroys an existing program
 */
surgescript_program_t* surgescript_program_destroy(surgescript_program_t* program)
{
    int j;

    for(j = 0; j < ssarray_length(program->text); j++)
        ssfree(program->text[j]);

    ssarray_release(program->text);
    ssarray_release(program->label);
    ssarray_release(program->line);
    ssfree(program);

    return NULL;
}

/*
 * surgescript_program_operand_u()
 * Operand constructor (unsigned)
 */
surgescript_program_operand_t surgescript_program_operand_u(unsigned u)
{
    surgescript_program_operand_t op = { .u = u };
    return op;
}

/*
 * surgescript_program_operand_f()
 * Operand constructor (float)
 */
surgescript_program_operand_t surgescript_program_operand_f(float f)
{
    surgescript_program_operand_t op = { .f = f };
    return op;
}



/*
 * surgescript_program_add_line()
 * Adds a line of code to a program
 */
void surgescript_program_add_line(surgescript_program_t* program, surgescript_program_operator_t op, surgescript_program_operand_t a, surgescript_program_operand_t b, surgescript_program_operand_t c)
{
    surgescript_program_operation_t line = { op, a, b, c };
    ssarray_push(program->line, line);
}

/*
 * surgescript_program_add_label()
 * Adds a newly created label to the program
 */
void surgescript_program_add_label(surgescript_program_t* program, surgescript_program_label_t label)
{
    program->label[label] = ssarray_length(program->line);
}

/*
 * surgescript_program_add_text()
 * Adds a text to a program (each program has a set of read-only texts)
 */
int surgescript_program_add_text(surgescript_program_t* program, const char* text)
{
    ssarray_push(program->text, surgescript_util_strdup(text));
    return ssarray_length(program->text) - 1;
}

/*
 * surgescript_program_new_label()
 * Creates and returns a new label
 */
surgescript_program_label_t surgescript_program_new_label(surgescript_program_t* program)
{
    ssarray_push(program->label, 0);
    return ssarray_length(program->label) - 1;
}

/*
 * surgescript_program_find_text()
 * Finds the first index such that text[index] == text, or returns -1 if not found
 */
int surgescript_program_find_text(const surgescript_program_t* program, const char* text)
{
    int i, len = ssarray_length(program->text);

    for(i = 0; i < len; i++) {
        if(strcmp(program->text[i], text) == 0)
            return i;
    }

    return -1;
}

/*
 * surgescript_program_get_text()
 * Gets a string literal from the program
 */
const char* surgescript_program_get_text(const surgescript_program_t* program, int index)
{
    if(index >= 0 && index < ssarray_length(program->text))
        return program->text[index];
    else
        return "";
}

/*
 * surgescript_program_text_count()
 * How many string literals exist in the program?
 */
int surgescript_program_text_count(const surgescript_program_t* program)
{
    return ssarray_length(program->text);
}

/*
 * surgescript_program_arity()
 * what's the arity of this program?
 */
int surgescript_program_arity(const surgescript_program_t* program)
{
    return program->arity;
}

/*
 * surgescript_program_run()
 * Runs a written program
 */
void surgescript_program_run(surgescript_program_t* program, surgescript_renv_t* runtime_environment)
{
    program->ip = 0;
    while(program->ip < ssarray_length(program->line))
        run_instruction(program, runtime_environment, program->line[program->ip].instruction, program->line[program->ip].a, program->line[program->ip].b, program->line[program->ip].c);
}

/*
 * surgescript_program_run_update()
 * Runs a written program
 */
/*void surgescript_program_run_update(surgescript_program_t* program, const surgescript_renv_t* runtime_environment)
{
    program->delayedcalls = surgescript_program_delayedcalls_create();
    program->ip = 0;
    while(program->ip < ssarray_length(program->line))
        run_instruction(program, runtime_environment, program->line[program->ip].instruction, program->line[program->ip].a, program->line[program->ip].b, program->line[program->ip].c);
}*/

/*
 * surgescript_program_run_render()
 * Rendering routines of a written program (called after update)
 */
/*void surgescript_program_run_render(surgescript_program_t* program, const surgescript_renv_t* runtime_environment)
{
    surgescript_program_delayedcalls_run(program->delayedcalls, surgescript_renv_owner(runtime_environment));
    program->delayedcalls = surgescript_program_delayedcalls_destroy(program->delayedcalls);
}*/

/* -------------------------------
 * private stuff
 * ------------------------------- */

/* delayed calls */
/*surgescript_program_delayedcalls_t* surgescript_program_delayedcalls_create()
{
    return NULL;
}

surgescript_program_delayedcalls_t* surgescript_program_delayedcalls_destroy(surgescript_program_delayedcalls_t* delayedcalls)
{
    int i;

    if(delayedcalls) {
        surgescript_program_delayedcalls_destroy(delayedcalls->next);
        for(i = 0; i < delayedcalls->num_params; i++)
            surgescript_var_destroy(delayedcalls->param[i]);
        ssfree(delayedcalls->param);
        ssfree(delayedcalls);
    }

    return NULL;
}

surgescript_program_delayedcalls_t* surgescript_program_delayedcalls_add(surgescript_program_delayedcalls_t* delayedcalls, void (*fun)(surgescript_object_t*, surgescript_var_t**, int), surgescript_var_t** param, int num_params)
{
    int i;
    surgescript_program_delayedcalls_t* node = ssmalloc(sizeof *node);

    node->fun = fun;
    node->num_params = num_params;
    node->param = ssmalloc(num_params * sizeof(*(node->param)));
    for(i = 0; i < num_params; i++)
        node->param[i] = surgescript_var_clone(param[i]);

    node->next = delayedcalls;
    return node;
}

void surgescript_program_delayedcalls_run(surgescript_program_delayedcalls_t* delayedcalls, surgescript_object_t* self)
{
    if(delayedcalls) {
        surgescript_program_delayedcalls_run(delayedcalls->next, self);
        delayedcalls->fun(self, delayedcalls->param, delayedcalls->num_params);
    }
}*/

/* runs an instruction */
void run_instruction(surgescript_program_t* program, surgescript_renv_t* runtime_environment, surgescript_program_operator_t instruction, surgescript_program_operand_t a, surgescript_program_operand_t b, surgescript_program_operand_t c)
{
    /* temporary variables */
    surgescript_var_t** t = surgescript_renv_tmp(runtime_environment);

    /* run the instruction */
    switch(instruction) {
        /* NOP */
        case SSOP_NOP:
            break;

        case SSOP_OUT: {
            char* str = surgescript_var_get_string(t[a.u]);
            printf("%s\n", str);
            ssfree(str);
            break;
        }

        case SSOP_IN: {
            char buf[81];
            scanf("%80s", buf);
            surgescript_var_set_string(t[a.u], buf);
            break;
        }

        /* assignment operations */
        case SSOP_ASSIGN_NULL:
            surgescript_var_set_null(t[a.u]);
            break;

        case SSOP_ASSIGN_BOOL:
            surgescript_var_set_bool(t[a.u], b.u);
            break;

        case SSOP_ASSIGN_NUMBER:
            surgescript_var_set_number(t[a.u], b.f);
            break;

        case SSOP_ASSIGN_STRING:
            surgescript_var_set_string(t[a.u], program->text[b.u]);
            break;

        case SSOP_ASSIGN_OBJECTHANDLE:
            surgescript_var_set_objecthandle(t[a.u], b.u);
            break;

        /*case SSOP_ASSIGN_LAMBDA: {
            surgescript_object_t* o = surgescript_objectmanager_get(runtime_environment->object_manager, surgescript_var_get_number(t[b.u]));
            if(o != NULL) {
                surgescript_var_set_lambda(t[a.u], surgescript_program_lambda_create(
                    surgescript_programpool_get(
                        runtime_environment->program_pool,
                        surgescript_object_name(o), // TODO: inheritance
                        program->text[b.u]
                    ),
                    surgescript_renv_create(
                        o,
                        surgescript_renv_stack(runtime_environment),
                        surgescript_object_heap(o),
                        runtime_environment->program_pool,
                        runtime_environment->object_manager
                    )
                ));
            }
            else
                ssfatal("Runtime Error: null pointer exception - accessing invalid object in %s", surgescript_object_name(surgescript_renv_owner(runtime_environment)));
            break;
        }*/

        /* heap operations */
        case SSOP_STORE:
            surgescript_var_copy(surgescript_heap_at(surgescript_renv_heap(runtime_environment), (surgescript_heapptr_t)surgescript_var_get_number(t[a.u])), t[b.u]);
            break;

        case SSOP_LOAD:
            surgescript_var_copy(t[a.u], surgescript_heap_at(surgescript_renv_heap(runtime_environment), (surgescript_heapptr_t)surgescript_var_get_number(t[b.u])));
            break;

        case SSOP_MALLOC:
            surgescript_var_set_number(t[a.u], surgescript_heap_malloc(surgescript_renv_heap(runtime_environment)));
            break;

        case SSOP_FREE:
            surgescript_heap_free(surgescript_renv_heap(runtime_environment), (surgescript_heapptr_t)surgescript_var_get_number(t[a.u]));
            break;

        /* stack operations */
        case SSOP_PUSH:
            surgescript_var_copy(surgescript_stack_push(surgescript_renv_stack(runtime_environment), surgescript_var_create()), t[a.u]);
            break;

        case SSOP_POP:
            surgescript_var_copy(t[a.u], surgescript_stack_top(surgescript_renv_stack(runtime_environment)));
            surgescript_stack_pop(surgescript_renv_stack(runtime_environment));
            break;

        case SSOP_PEEK:
            surgescript_var_copy(t[a.u], surgescript_stack_at(surgescript_renv_stack(runtime_environment), (int)b.u));
            break;

        case SSOP_POKE:
            surgescript_var_copy(surgescript_stack_at(surgescript_renv_stack(runtime_environment), (int)a.u),  t[b.u]);
            break;

        /* basic arithmetic */
        case SSOP_ZERO:
            surgescript_var_set_number(t[a.u], 0.0f);
            break;

        case SSOP_INC:
            surgescript_var_set_number(t[a.u], surgescript_var_get_number(t[a.u]) + 1.0f); /* TODO: inc op */
            break;

        case SSOP_DEC:
            surgescript_var_set_number(t[a.u], surgescript_var_get_number(t[a.u]) - 1.0f); /* TODO: dec op */
            break;

        case SSOP_ADD:
            surgescript_var_set_number(t[a.u], surgescript_var_get_number(t[a.u]) + surgescript_var_get_number(t[b.u]));
            break;

        case SSOP_SUB:
            surgescript_var_set_number(t[a.u], surgescript_var_get_number(t[a.u]) - surgescript_var_get_number(t[b.u]));
            break;

        case SSOP_MUL:
            surgescript_var_set_number(t[a.u], surgescript_var_get_number(t[a.u]) * surgescript_var_get_number(t[b.u]));
            break;

        case SSOP_DIV:
            if(fabsf(surgescript_var_get_number(t[b.u])) >= FLT_EPSILON)
                surgescript_var_set_number(t[a.u], surgescript_var_get_number(t[a.u]) / surgescript_var_get_number(t[b.u]));
            else if(surgescript_var_get_number(t[a.u]) >= 0.0f)
                surgescript_var_set_number(t[a.u], INFINITY * sssign(surgescript_var_get_number(t[b.u])));
            else
                surgescript_var_set_number(t[a.u], -INFINITY * sssign(surgescript_var_get_number(t[b.u])));
            break;

        case SSOP_MOD:
            surgescript_var_set_number(t[a.u], fmodf(surgescript_var_get_number(t[a.u]), surgescript_var_get_number(t[b.u])));
            break;

        case SSOP_POW:
            surgescript_var_set_number(t[a.u], powf(surgescript_var_get_number(t[a.u]), surgescript_var_get_number(t[b.u])));
            break;

        case SSOP_NOT:
            if(surgescript_var_get_bool(t[a.u]))
                surgescript_var_set_bool(t[a.u], false);
            else
                surgescript_var_set_bool(t[a.u], true);
            break;

        case SSOP_AND:
            if(surgescript_var_get_bool(t[a.u])) /* short-circuit operator */
                surgescript_var_set_bool(t[a.u], surgescript_var_get_bool(t[b.u]));
            else
                surgescript_var_set_bool(t[a.u], false);
            break;

        case SSOP_OR:
            if(surgescript_var_get_bool(t[a.u])) /* short-circuit operator */
                surgescript_var_set_bool(t[a.u], true);
            else
                surgescript_var_set_bool(t[a.u], surgescript_var_get_bool(t[b.u]));
            break;

        case SSOP_COPY:
            surgescript_var_copy(t[a.u], t[b.u]);
            break;

        case SSOP_NEG:
            surgescript_var_set_number(t[a.u], -surgescript_var_get_number(t[a.u]));
            break;

        /* utility operations */
        case SSOP_TYPEOF:
            surgescript_var_set_string(t[a.u], surgescript_var_typename(t[a.u]));
            break;

        case SSOP_TOBOOL:
            surgescript_var_set_bool(t[a.u], surgescript_var_get_bool(t[a.u]));
            break;

        case SSOP_TONUMBER:
            surgescript_var_set_number(t[a.u], surgescript_var_get_number(t[a.u]));
            break;

        case SSOP_TOSTRING: {
            char* buf = surgescript_var_get_string(t[a.u]);
            surgescript_var_set_string(t[a.u], buf);
            ssfree(buf);
            break;
        }

        case SSOP_STRLEN: {
            char* buf = surgescript_var_get_string(t[a.u]);
            int len = strlen(buf); /* TODO: utf-8 compat */
            surgescript_var_set_number(t[a.u], len);
            ssfree(buf);
            break;
        }

        case SSOP_STRMID: {
            char* substr;
            char* str = surgescript_var_get_string(t[a.u]);
            int start = surgescript_var_get_number(t[b.u]);
            int length = surgescript_var_get_number(t[c.u]);
            int n = strlen(str); /* TODO: utf-8 compat */

            start = ssclamp(start, 0, n);
            length = ssclamp(length, 0, n - start);
            substr = ssmalloc((1 + length) * sizeof(*substr));
            surgescript_util_strncpy(substr, str + start, length);
            surgescript_var_set_string(t[a.u], substr);

            ssfree(substr);
            ssfree(str);
            break;
        }

        case SSOP_STRCAT: {
            char* str1 = surgescript_var_get_string(t[a.u]);
            char* str2 = surgescript_var_get_string(t[b.u]);
            char* str = ssmalloc((1 + strlen(str1) + strlen(str2)) * sizeof(*str));

            surgescript_var_set_string(t[a.u], strcat(strcpy(str, str1), str2));

            ssfree(str);
            ssfree(str2);
            ssfree(str1);
            break;
        }

        case SSOP_STRAT: {
            char* str = surgescript_var_get_string(t[a.u]);
            int index = surgescript_var_get_number(t[b.u]);
            int n = strlen(str); /* TODO: utf-8 compat */

            char buf[2] = { ' ', '\0' };
            if(index >= 0 && index < n)
                buf[0] = str[index];
            surgescript_var_set_string(t[a.u], buf);

            ssfree(str);
            break;
        }

        /* jumping */
        case SSOP_JMP:
            program->ip = program->label[a.u];
            return;

        case SSOP_JMP_IF_TRUE:
            if(surgescript_var_get_bool(t[b.u])) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JMP_IF_EQUAL:
            if(surgescript_var_compare(t[b.u], t[c.u]) == 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JMP_IF_NOTEQUAL:
            if(surgescript_var_compare(t[b.u], t[c.u]) != 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JMP_IF_LOWER:
            if(surgescript_var_compare(t[b.u], t[c.u]) < 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JMP_IF_GREATER:
            if(surgescript_var_compare(t[b.u], t[c.u]) > 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JMP_IF_LOWEROREQUAL:
            if(surgescript_var_compare(t[b.u], t[c.u]) <= 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JMP_IF_GREATEROREQUAL:
            if(surgescript_var_compare(t[b.u], t[c.u]) >= 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JMP_IF_ZERO:
            if(fabsf(surgescript_var_get_number(t[b.u])) <= FLT_EPSILON) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JMP_IF_NOTZERO:
            if(fabsf(surgescript_var_get_number(t[b.u])) > FLT_EPSILON) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        /* function calls */
        /* FIXME: is render needed at all? */

        /*case SSOP_CALL: {
            const surgescript_program_lambda_t* lambda = surgescript_var_get_lambda(t[a.u]);
            if(lambda != NULL) {
                const surgescript_program_t* prog = surgescript_program_lambda_program(lambda);
                int expected_params = prog->arity;
                int actual_params = (int)b.u;
                if(expected_params == actual_params)
                    surgescript_program_lambda_run_update(lambda);
                else
                    ssfatal("Runtime Error: a function called in %s expected %d parameters, but received %d", surgescript_object_name(surgescript_renv_owner(runtime_environment)), expected_params, actual_params);
            }
            else
                ssfatal("Runtime Error: in %s, can't perform a function call on something that isn't a function", surgescript_object_name(surgescript_renv_owner(runtime_environment)));
            break;
        }*/
        case SSOP_CALL: {
            char* program_name = surgescript_var_get_string(t[a.u]);
            unsigned object_handle = surgescript_var_get_objecthandle(t[b.u]);
            int params = c.u;

            surgescript_object_t* object = surgescript_objectmanager_get(surgescript_renv_objectmanager(runtime_environment), object_handle);
            const char* object_name = surgescript_object_name(object);
            surgescript_program_t* prog = surgescript_programpool_get(surgescript_renv_programpool(runtime_environment), object_name, program_name);
            int expected_params = prog->arity;

            if(params == expected_params) {
                /* the parameters are pushed onto the stack (reverse order) */
                surgescript_stack_t* stack = surgescript_renv_stack(runtime_environment);
                surgescript_renv_t* callee_runtime_environment = surgescript_renv_create(
                    object,
                    stack,
                    surgescript_object_heap(object),
                    surgescript_renv_programpool(runtime_environment),
                    surgescript_renv_objectmanager(runtime_environment)
                );

                surgescript_stack_pushenv(stack, prog->num_local_vars);
                surgescript_program_run(prog, callee_runtime_environment);
                surgescript_stack_popenv(stack);

                surgescript_var_copy(t[0], *(surgescript_renv_tmp(callee_runtime_environment)+3)); /* t[0] = prog's return value (if any) */
                surgescript_renv_destroy(callee_runtime_environment);
            }
            else
                ssfatal("Runtime Error: function \"%s.%s\" (called in \"%s\") expects %d parameters, but received %d.", object_name, program_name, surgescript_object_name(surgescript_renv_owner(runtime_environment)), expected_params, params);

            free(program_name);
            break;
        }

        /* modify program->delayedcalls here */
        /* surgescript_program_delayedcalls_t* surgescript_program_delayedcalls_add(surgescript_program_delayedcalls_t* delayedcalls, void (*fun)(surgescript_object_t*, surgescript_var_t**, int), surgescript_var_t** param, int num_params); */
        /* FIXME: is render needed at all? */
        case SSOP_CALL_USERFUN: {
            break;
        }

        case SSOP_HALT:
            program->ip = ssarray_length(program->line);
            return;
    }

    /* next line */
    program->ip++;
}
