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

/* an operation / command */
typedef struct surgescript_program_operation_t surgescript_program_operation_t;
struct surgescript_program_operation_t
{
    surgescript_program_operator_t instruction;
    surgescript_program_operand_t a, b;
};

/* the program structure */
struct surgescript_program_t
{
    int arity, num_local_vars; /* config */
    int ip; /* instruction pointer */
    void (*run)(surgescript_program_t*, surgescript_renv_t*); /* run function; strategy pattern */

    SSARRAY(surgescript_program_operation_t, line); /* a set of operations (or lines of code) */
    SSARRAY(surgescript_program_label_t, label); /* labels (label[j] is the index of a line of code, j is a label) */
    SSARRAY(char*, text); /* read-only text data */
};

/* a program that encapsulates a C-function */
typedef struct surgescript_cprogram_t surgescript_cprogram_t;
struct surgescript_cprogram_t
{
    surgescript_program_t program; /* base class */
    surgescript_program_cfunction_t cfunction; /* pointer to the C-function */
};

/* utilities */
static surgescript_program_t* init_program(surgescript_program_t* program, int arity, int num_local_vars, void (*run_function)(surgescript_program_t*, surgescript_renv_t*));
static void run_program(surgescript_program_t* program, surgescript_renv_t* runtime_environment);
static void run_cprogram(surgescript_program_t* program, surgescript_renv_t* runtime_environment);
static inline void run_instruction(surgescript_program_t* program, surgescript_renv_t* runtime_environment, surgescript_program_operator_t instruction, surgescript_program_operand_t a, surgescript_program_operand_t b);

/* inlines */
surgescript_program_operand_t surgescript_program_operand_u(unsigned u);
surgescript_program_operand_t surgescript_program_operand_f(float f);
surgescript_program_operand_t surgescript_program_operand_b(bool b);
surgescript_program_operand_t surgescript_program_operand_i(int i);


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
    return init_program(program, arity, num_local_vars, run_program);
}

/*
 * surgescript_cprogram_create()
 * Creates a program that encapsulates a C-function
 */
surgescript_program_t* surgescript_cprogram_create(int arity, surgescript_program_cfunction_t cfunction)
{
    surgescript_cprogram_t* cprogram = ssmalloc(sizeof *cprogram);
    cprogram->cfunction = cfunction;
    return init_program((surgescript_program_t*)cprogram, arity, 0, run_cprogram);
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
 * surgescript_program_add_line()
 * Adds a line of code to a program
 */
void surgescript_program_add_line(surgescript_program_t* program, surgescript_program_operator_t op, surgescript_program_operand_t a, surgescript_program_operand_t b)
{
    surgescript_program_operation_t line = { op, a, b };
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
    program->run(program, runtime_environment);
}

/* -------------------------------
 * private stuff
 * ------------------------------- */

/* initializes a program */
surgescript_program_t* init_program(surgescript_program_t* program, int arity, int num_local_vars, void (*run_function)(surgescript_program_t*, surgescript_renv_t*))
{
    program->arity = arity;
    program->num_local_vars = num_local_vars;
    program->ip = 0;
    program->run = run_function;

    ssarray_init(program->line);
    ssarray_init(program->label);
    ssarray_init(program->text);

    return program;
}

/* runs a program */
void run_program(surgescript_program_t* program, surgescript_renv_t* runtime_environment)
{
    program->ip = 0;
    while(program->ip < ssarray_length(program->line))
        run_instruction(program, runtime_environment, program->line[program->ip].instruction, program->line[program->ip].a, program->line[program->ip].b);
}

/* runs a C-program */
void run_cprogram(surgescript_program_t* program, surgescript_renv_t* runtime_environment)
{
    surgescript_cprogram_t* cprogram = (surgescript_cprogram_t*)program;
    surgescript_object_t* caller = surgescript_renv_owner(runtime_environment);
    surgescript_stack_t* stack = surgescript_renv_stack(runtime_environment);
    surgescript_var_t** param = program->arity > 0 ? ssmalloc(program->arity * sizeof(*param)) : NULL;
    surgescript_var_t* return_value = NULL;

    /* grab parameters from the stack (stacked in reverse order) */
    for(int i = 1; i <= program->arity; i++)
        param[i-1] = surgescript_stack_at(stack, -i);

    /* call C-function */
    return_value = (surgescript_var_t*)(cprogram->cfunction(caller, (const surgescript_var_t**)param, program->arity));
    if(return_value != NULL) {
        surgescript_var_copy(*(surgescript_renv_tmp(runtime_environment) + 3), return_value);
        surgescript_var_destroy(return_value);
    }

    /* release parameters */
    if(param)
        ssfree(param);
}

/* runs an instruction */
void run_instruction(surgescript_program_t* program, surgescript_renv_t* runtime_environment, surgescript_program_operator_t instruction, surgescript_program_operand_t a, surgescript_program_operand_t b)
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
        case SSOP_MOVN: /* move null */
            surgescript_var_set_null(t[a.u]);
            break;

        case SSOP_MOVB: /* move boolean */
            surgescript_var_set_bool(t[a.u], b.b);
            break;

        case SSOP_MOVF: /* move number (float) */
            surgescript_var_set_number(t[a.u], b.f);
            break;

        case SSOP_MOVS: /* move string */
            surgescript_var_set_string(t[a.u], program->text[b.u]);
            break;

        case SSOP_MOVH: /* move handle (object handle) */
            surgescript_var_set_objecthandle(t[a.u], b.u);
            break;

        case SSOP_MOVC: /* move caller object ("this" pointer) */
            surgescript_var_set_objecthandle(t[a.u], surgescript_object_handle(surgescript_renv_owner(runtime_environment)));
            break;

        case SSOP_MOVT: /* move temp */
            surgescript_var_copy(t[a.u], t[b.u]);
            break;

        /* heap operations */
        case SSOP_STORE:
            surgescript_var_copy(surgescript_heap_at(surgescript_renv_heap(runtime_environment), (surgescript_heapptr_t)surgescript_var_get_number(t[a.u])), t[b.u]);
            break;

        case SSOP_LOAD:
            surgescript_var_copy(t[a.u], surgescript_heap_at(surgescript_renv_heap(runtime_environment), (surgescript_heapptr_t)surgescript_var_get_number(t[b.u])));
            break;

        /*case SSOP_MALLOC:
            surgescript_var_set_number(t[a.u], surgescript_heap_malloc(surgescript_renv_heap(runtime_environment)));
            break;

        case SSOP_FREE:
            surgescript_heap_free(surgescript_renv_heap(runtime_environment), (surgescript_heapptr_t)surgescript_var_get_number(t[a.u]));
            break;*/

        /* stack operations */
        case SSOP_PUSH:
            surgescript_var_copy(surgescript_stack_push(surgescript_renv_stack(runtime_environment), surgescript_var_create()), t[a.u]);
            break;

        case SSOP_POP:
            surgescript_var_copy(t[a.u], surgescript_stack_top(surgescript_renv_stack(runtime_environment)));
            surgescript_stack_pop(surgescript_renv_stack(runtime_environment));
            break;

        case SSOP_SPEEK:
            surgescript_var_copy(t[a.u], surgescript_stack_at(surgescript_renv_stack(runtime_environment), b.i));
            break;

        case SSOP_SPOKE:
            surgescript_var_copy(surgescript_stack_at(surgescript_renv_stack(runtime_environment), b.i),  t[a.u]);
            break;

        /* basic arithmetic */
        /*case SSOP_ZERO:
            surgescript_var_set_number(t[a.u], 0.0f);
            break;*/

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

        /*case SSOP_MOD:
            surgescript_var_set_number(t[a.u], fmodf(surgescript_var_get_number(t[a.u]), surgescript_var_get_number(t[b.u])));
            break;*/

        /*case SSOP_POW:
            surgescript_var_set_number(t[a.u], powf(surgescript_var_get_number(t[a.u]), surgescript_var_get_number(t[b.u])));
            break;*/

        case SSOP_NOT:
            surgescript_var_set_bool(t[a.u], !surgescript_var_get_bool(t[a.u]));
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

        case SSOP_NEG:
            surgescript_var_set_number(t[a.u], -surgescript_var_get_number(t[a.u]));
            break;

        /* utility operations */
/*
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
            int len = strlen(buf); // TODO: utf-8 compat
            surgescript_var_set_number(t[a.u], len);
            ssfree(buf);
            break;
        }

        case SSOP_STRMID: {
            char* substr;
            char* str = surgescript_var_get_string(t[a.u]);
            int start = surgescript_var_get_number(t[b.u]);
            int length = surgescript_var_get_number(t[c.u]);
            int n = strlen(str); // TODO: utf-8 compat

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
            int n = strlen(str); // TODO: utf-8 compat

            char buf[2] = { ' ', '\0' };
            if(index >= 0 && index < n)
                buf[0] = str[index];
            surgescript_var_set_string(t[a.u], buf);

            ssfree(str);
            break;
        }
*/
        /* jumping */
        case SSOP_JMP:
            program->ip = program->label[a.u];
            return;

        case SSOP_CMP:
            surgescript_var_set_number(t[2], surgescript_var_compare(t[a.u], t[b.u]));
            break;

        case SSOP_JE:
            if(surgescript_var_get_number(t[2]) == 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JNE:
            if(surgescript_var_get_number(t[2]) != 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JL:
            if(surgescript_var_get_number(t[2]) < 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JG:
            if(surgescript_var_get_number(t[2]) > 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JLE:
            if(surgescript_var_get_number(t[2]) <= 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        case SSOP_JGE:
            if(surgescript_var_get_number(t[2]) >= 0) {
                program->ip = program->label[a.u];
                return;
            }
            else
                break;

        /* function calls */
        case SSOP_CALL: {
            char* program_name = surgescript_var_get_string(t[a.u]);
            unsigned object_handle = surgescript_var_get_objecthandle(t[b.u]);
            int params = (int)surgescript_var_get_number(t[2]);

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

                /* push an environment and call the program */
                surgescript_stack_pushenv(stack, prog->num_local_vars);
                surgescript_program_run(prog, callee_runtime_environment);
                surgescript_stack_popenv(stack);

                /* callee_tmp[3] is the return value of the program */
                surgescript_var_copy(t[2], *(surgescript_renv_tmp(callee_runtime_environment) + 3));
                surgescript_renv_destroy(callee_runtime_environment);
            }
            else
                ssfatal("Runtime Error: function \"%s.%s\" (called in \"%s\") expects %d parameters, but received %d.", object_name, program_name, surgescript_object_name(surgescript_renv_owner(runtime_environment)), expected_params, params);

            free(program_name);
            break;
        }

        case SSOP_RET:
            program->ip = ssarray_length(program->line);
            return;
    }

    /* next line */
    program->ip++;
}