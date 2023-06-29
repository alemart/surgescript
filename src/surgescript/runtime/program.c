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

/* require alloca */
#if !(defined(__APPLE__) || defined(MACOSX) || defined(macintosh) || defined(Macintosh))
#include <malloc.h>
#if defined(__linux__) || defined(__linux) || defined(__EMSCRIPTEN__)
#include <alloca.h>
#endif
#endif

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
    int arity; /* config */
    bool executed; /* has this program ever been executed? */
    void (*run)(surgescript_program_t*, const surgescript_renv_t*); /* run function; strategy pattern */
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

/* the names of the instructions */
static const char* instruction_name[] = {
    #define PRINT_NAME(x, y) y,
    SURGESCRIPT_PROGRAM_OPERATORS(PRINT_NAME)
};

/* utilities */
static surgescript_program_t* init_program(surgescript_program_t* program, int arity, void (*run_function)(surgescript_program_t*, const surgescript_renv_t*));
static void run_program(surgescript_program_t* program, const surgescript_renv_t* runtime_environment);
static void run_cprogram(surgescript_program_t* program, const surgescript_renv_t* runtime_environment);
static inline unsigned int run_instruction(surgescript_program_t* program, const surgescript_renv_t* runtime_environment, surgescript_program_operation_t* operation, unsigned int ip);
static inline surgescript_program_t* call_program(const surgescript_renv_t* caller_runtime_environment, int number_of_given_params, const char* program_name, surgescript_program_t* cached_program, surgescript_objectclassid_t* out_class_id);
static inline bool is_jump_instruction(surgescript_program_operator_t instruction);
static inline bool remove_labels(surgescript_program_t* program);
static char* hexdump(unsigned data, char* buf); /* writes the bytes stored in data to buf, in hex format */
static void fputs_escaped(const char* str, FILE* fp); /* works like fputs, but escapes the string */
static inline int fast_sign(double f);
static inline int fast_sign1(double f);
static inline int fast_notzero(double f);
static const int MAX_PROGRAM_ARITY = 256;

/* debug mode? */
#define SURGESCRIPT_DEBUG_MODE 0
#if SURGESCRIPT_DEBUG_MODE
static inline void debug(surgescript_program_t* program, surgescript_renv_t* runtime_environment, surgescript_program_operator_t instruction, surgescript_program_operand_t a, surgescript_program_operand_t b, surgescript_var_t** _t);
#endif

/* optimizations */
#define WANT_OPTIMIZED_PROGRAM_CALLS 1
#define OPTIMIZED_CALL_THRESHOLD 8

/* -------------------------------
 * public methods
 * ------------------------------- */

/*
 * surgescript_program_create()
 * Creates a new program (aka function call)
 */
surgescript_program_t* surgescript_program_create(int arity)
{
    surgescript_program_t* program = ssmalloc(sizeof *program);
    return init_program(program, arity, run_program);
}

/*
 * surgescript_program_create_native()
 * Creates a program that encapsulates a C-function
 * Please note that this C-function must return a newly-allocated surgescript_var_t*, or NULL
 */
surgescript_program_t* surgescript_program_create_native(int arity, surgescript_program_cfunction_t cfunction)
{
    surgescript_cprogram_t* cprogram = ssmalloc(sizeof *cprogram);
    cprogram->cfunction = cfunction;
    return init_program((surgescript_program_t*)cprogram, arity, run_cprogram);
}

/*
 * surgescript_program_destroy()
 * Destroys an existing program
 */
surgescript_program_t* surgescript_program_destroy(surgescript_program_t* program)
{
    for(int j = 0; j < ssarray_length(program->text); j++)
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
int surgescript_program_add_line(surgescript_program_t* program, surgescript_program_operator_t op, surgescript_program_operand_t a, surgescript_program_operand_t b)
{
    surgescript_program_operation_t line = { op, a, b };
    ssarray_push(program->line, line);

#if WANT_OPTIMIZED_PROGRAM_CALLS
    /* we add two NOPs after every CALL as a trick to help
       the program optimize itself during its own execution */
    if(op == SSOP_CALL) {
        surgescript_program_operand_t zero = surgescript_program_operand_u(0);
        surgescript_program_operation_t nop = { SSOP_NOP, zero, zero };

        ssarray_push(program->line, nop);
        ssarray_push(program->line, nop);
    }
#endif

    return ssarray_length(program->line) - 1;
}

/*
 * surgescript_program_chg_line()
 * Changes an existing line of code of the program
 */
int surgescript_program_chg_line(surgescript_program_t* program, int line, surgescript_program_operator_t op, surgescript_program_operand_t a, surgescript_program_operand_t b)
{
    surgescript_program_operation_t newline = { op, a, b };
    ssassert(op != SSOP_CALL); /* can't change the line do CALL due to the NOP optimization trick in surgescript_program_add_line(); won't change the labels */

    if(line >= 0 && line < ssarray_length(program->line)) {
        program->line[line] = newline;
        return line;
    }
    else
        return -1;
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
    int idx = surgescript_program_find_text(program, text);
    if(idx < 0) { /* if the text isn't already there */
        ssarray_push(program->text, ssstrdup(text));
        return ssarray_length(program->text) - 1;
    }
    else
        return idx;
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
 * What's the arity of this program?
 */
int surgescript_program_arity(const surgescript_program_t* program)
{
    return program->arity;
}

/*
 * surgescript_program_call()
 * Low-level SurgeScript program call.
 * You'll need to push the stack parameters yourself (prefer using surgescript_object_call_function() instead)
*/
void surgescript_program_call(surgescript_program_t* program, surgescript_renv_t* runtime_environment, int num_params)
{
    if(num_params == program->arity) {
        surgescript_stack_t* stack = surgescript_renv_stack(runtime_environment);
        surgescript_stack_pushenv(stack);
        program->run(program, runtime_environment);
        surgescript_stack_popenv(stack);
    }
    else {
        surgescript_object_t* owner = surgescript_renv_owner(runtime_environment);
        ssfatal("Runtime Error: internal program call - function of object \"%s\" expects %d parameters, but received %d.", surgescript_object_name(owner), program->arity, num_params);
    }
}


/*
 * surgescript_program_is_native()
 * Is the program native (i.e., written in C)?
 */
bool surgescript_program_is_native(const surgescript_program_t* program)
{
    return program->run == run_cprogram;
}

/* has this program ever been executed? */
bool surgescript_program_executed(const surgescript_program_t* program)
{
    return program->executed;
}

/* dump the program to a file */
void surgescript_program_dump(surgescript_program_t* program, FILE* fp)
{
    int i;
    char hex[2][1 + 2 * sizeof(unsigned)];
    surgescript_program_operation_t* op;

    remove_labels(program);

    /* print header */
    fprintf(fp,
        "{\n"
        "    \"arity\": %d,\n"
        "    \"code\": [\n",
    program->arity);

    /* print code */
    for(i = 0; i < ssarray_length(program->line); i++) {
        op = &(program->line[i]);
        fprintf(fp,
            "        \"%s\t  %s    %s\"%s\n",
            instruction_name[op->instruction],
            hexdump(op->a.u, hex[0]),
            hexdump(op->b.u, hex[1]),
            (i < ssarray_length(program->line) - 1) ? "," : ""
        );
    }

    /* print text section */
    fprintf(fp,
        "    ],\n"
        "    \"text\": [\n"
    );

    for(i = 0; i < ssarray_length(program->text); i++) {
        fputs("        \"", fp);
        fputs_escaped(program->text[i], fp);
        fputs((i < ssarray_length(program->text) - 1) ? "\",\n" : "\"\n", fp);
    }

    /* print footer */
    fprintf(fp, "    ]\n}\n");
}



/* -------------------------------
 * private stuff
 * ------------------------------- */

/* initializes a program */
surgescript_program_t* init_program(surgescript_program_t* program, int arity, void (*run_function)(surgescript_program_t*, const surgescript_renv_t*))
{
    /* sanity check */
    if(arity > MAX_PROGRAM_ARITY)
        ssfatal("MAX_PROGRAM_ARITY (%d) exceeded.", MAX_PROGRAM_ARITY); /* really?? */

    /* initialization */
    program->arity = ssmax(0, arity);
    program->executed = false;
    program->run = run_function;

    ssarray_init(program->line);
    ssarray_init(program->label);
    ssarray_init(program->text);

    return program;
}

/* runs a program */
void run_program(surgescript_program_t* program, const surgescript_renv_t* runtime_environment)
{
    unsigned int ip = 0; /* instruction pointer */

    program->executed = true;
    remove_labels(program);

    while(ip < ssarray_length(program->line))
        ip = run_instruction(program, runtime_environment, program->line + ip, ip);
}

/* runs a C-program */
void run_cprogram(surgescript_program_t* program, const surgescript_renv_t* runtime_environment)
{
    surgescript_cprogram_t* cprogram = (surgescript_cprogram_t*)program;
    surgescript_object_t* object = surgescript_renv_owner(runtime_environment);
    surgescript_stack_t* stack = surgescript_renv_stack(runtime_environment);
    const surgescript_var_t** param = program->arity > 0 ? alloca(program->arity * sizeof(*param)) : NULL;
    surgescript_var_t* return_value = NULL;

    /* set the execution flag */
    program->executed = true;

    /* grab parameters from the stack (stacked in left-to-right order) */
    for(int i = 1; i <= program->arity; i++)
        param[program->arity-i] = surgescript_stack_peek(stack, -i);

    /* call C-function */
    return_value = cprogram->cfunction(object, param, program->arity);
    if(return_value != NULL) {
        surgescript_var_copy(*(surgescript_renv_tmp(runtime_environment) + 0), return_value);
        surgescript_var_destroy(return_value);
    }
    else
        surgescript_var_set_null(*(surgescript_renv_tmp(runtime_environment) + 0));
}

/* runs an instruction and returns a new value for the instruction pointer */
unsigned int run_instruction(surgescript_program_t* program, const surgescript_renv_t* runtime_environment, surgescript_program_operation_t* operation, unsigned int ip)
{
    /* helper macro */
    #ifdef t
    #undef t
    #endif
    #define t(k)             _t[(k).u & 3]

    /* temporary variables */
    surgescript_var_t** _t = surgescript_renv_tmp(runtime_environment);

    /* read the operands */
    surgescript_program_operand_t a = operation->a;
    surgescript_program_operand_t b = operation->b;

    /* debug mode */
    #if SURGESCRIPT_DEBUG_MODE
    debug(program, runtime_environment, operation->instruction, a, b, _t);
    #endif

    /* run the instruction */
    switch(operation->instruction) {
        /* basics */
        case SSOP_NOP: /* no-operation */
            break;

        case SSOP_SELF: /* owner object ("this" pointer) */
            surgescript_var_set_objecthandle(t(a), surgescript_object_handle(surgescript_renv_owner(runtime_environment)));
            break;

        case SSOP_STATE: /* t[a] receives the current state. If b == -1, then the current state is set to t[a] instead. */
            if(b.i == -1) {
                char state[256] = "";
                surgescript_var_to_string(t(a), state, sizeof(state));
                surgescript_object_set_state(surgescript_renv_owner(runtime_environment), state);
            }
            else
                surgescript_var_set_string(t(a), surgescript_object_state(surgescript_renv_owner(runtime_environment)));
            break;

        case SSOP_CALLER: /* caller object */
            surgescript_var_set_objecthandle(t(a), surgescript_renv_caller(runtime_environment));
            break;

        /* assignment operations */
        case SSOP_MOVN: /* move null */
            surgescript_var_set_null(t(a));
            break;

        case SSOP_MOVB: /* move boolean */
            surgescript_var_set_bool(t(a), b.b);
            break;

        case SSOP_MOVF: /* move number */
            surgescript_var_set_number(t(a), b.f);
            break;

        case SSOP_MOVS: /* move string */
            if(b.u < ssarray_length(program->text))
                surgescript_var_set_string(t(a), program->text[b.u]);
            break;

        case SSOP_MOVO: /* move object handle */
            surgescript_var_set_objecthandle(t(a), b.u);
            break;

        case SSOP_MOVX: /* move int64 */
            surgescript_var_set_rawbits(t(a), b.i64);
            break;

        case SSOP_MOV: /* move temp */
            surgescript_var_copy(t(a), t(b));
            break;

        case SSOP_XCHG: /* fast exchange */
            surgescript_var_swap(t(a), t(b));
            break;

        /* heap operations */
        case SSOP_ALLOC:
            surgescript_var_set_number(t(a), surgescript_heap_malloc(surgescript_renv_heap(runtime_environment)));
            break;

        case SSOP_PEEK:
            surgescript_var_copy(t(a), surgescript_heap_at(surgescript_renv_heap(runtime_environment), b.u));
            break;

        case SSOP_POKE:
            surgescript_var_copy(surgescript_heap_at(surgescript_renv_heap(runtime_environment), b.u), t(a));
            break;

        /* stack operations */
        case SSOP_PUSH:
            surgescript_stack_push(surgescript_renv_stack(runtime_environment), surgescript_var_clone(t(a)));
            break;

        case SSOP_POP:
            surgescript_var_copy(t(a), surgescript_stack_top(surgescript_renv_stack(runtime_environment)));
            surgescript_stack_pop(surgescript_renv_stack(runtime_environment));
            break;

        case SSOP_SPEEK:
            surgescript_var_copy(t(a), surgescript_stack_peek(surgescript_renv_stack(runtime_environment), b.i));
            break;

        case SSOP_SPOKE:
            surgescript_stack_poke(surgescript_renv_stack(runtime_environment), b.i, t(a));
            break;

        case SSOP_PUSHN:
            surgescript_stack_pushn(surgescript_renv_stack(runtime_environment), a.u);
            break;

        case SSOP_POPN:
            surgescript_stack_popn(surgescript_renv_stack(runtime_environment), a.u);
            break;

        /* basic arithmetic */
        case SSOP_INC:
            if(a.u != 2)
                surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)) + 1);
            else
                surgescript_var_set_rawbits(t(a), surgescript_var_get_rawbits(t(a)) + 1);
            break;

        case SSOP_DEC:
            if(a.u != 2)
                surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)) - 1);
            else
                surgescript_var_set_rawbits(t(a), surgescript_var_get_rawbits(t(a)) - 1);
            break;

        case SSOP_ADD:
            surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)) + surgescript_var_get_number(t(b)));
            break;

        case SSOP_SUB:
            surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)) - surgescript_var_get_number(t(b)));
            break;

        case SSOP_MUL:
            surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)) * surgescript_var_get_number(t(b)));
            break;

        case SSOP_DIV:
            /* division by zero should follow the IEEE-754 */
            surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)) / surgescript_var_get_number(t(b)));
            break;

        case SSOP_MOD:
            surgescript_var_set_number(t(a), fmod(surgescript_var_get_number(t(a)), surgescript_var_get_number(t(b))));
            break;

        case SSOP_NEG:
            surgescript_var_set_number(t(a), -surgescript_var_get_number(t(b)));
            break;

        case SSOP_LNOT:
            surgescript_var_set_bool(t(a), !surgescript_var_get_bool(t(b)));
            break;

        case SSOP_LNOT2:
            surgescript_var_set_bool(t(a), surgescript_var_get_bool(t(b)));
            break;

        /* bitwise operations */
        case SSOP_NOT:
            surgescript_var_set_rawbits(t(a), ~surgescript_var_get_rawbits(t(b)));
            break;

        case SSOP_AND:
            surgescript_var_set_rawbits(t(a), surgescript_var_get_rawbits(t(a)) & surgescript_var_get_rawbits(t(b)));
            break;

        case SSOP_OR:
            surgescript_var_set_rawbits(t(a), surgescript_var_get_rawbits(t(a)) | surgescript_var_get_rawbits(t(b)));
            break;

        case SSOP_XOR:
            surgescript_var_set_rawbits(t(a), surgescript_var_get_rawbits(t(a)) ^ surgescript_var_get_rawbits(t(b)));
            break;

        /* comparing & testing */
        case SSOP_TEST:
            surgescript_var_set_rawbits(_t[2], surgescript_var_get_rawbits(t(a)) & surgescript_var_get_rawbits(t(b)));
            break;

        case SSOP_TCHK:
            surgescript_var_set_rawbits(_t[2], surgescript_var_typecheck(t(a), b.i));
            break;

        case SSOP_TC01:
            surgescript_var_set_rawbits(_t[2], surgescript_var_typecheck(_t[0], a.i) & surgescript_var_typecheck(_t[1], a.i));
            break;

        case SSOP_TCMP:
            surgescript_var_set_rawbits(_t[2], surgescript_var_typecode(t(a)) ^ surgescript_var_typecode(t(b)));
            break;

        case SSOP_CMP:
            surgescript_var_set_rawbits(_t[2], surgescript_var_compare(t(a), t(b)));
            break;

        /* jumping */
        case SSOP_JMP:
            return a.u;

        case SSOP_JE:
            if(!surgescript_var_get_rawbits(_t[2]))
                return a.u;
            break;

        case SSOP_JNE:
            if(surgescript_var_get_rawbits(_t[2]))
                return a.u;
            break;

        case SSOP_JL:
            if(surgescript_var_get_rawbits(_t[2]) < 0)
                return a.u;
            break;

        case SSOP_JG:
            if(surgescript_var_get_rawbits(_t[2]) > 0)
                return a.u;
            break;

        case SSOP_JLE:
            if(surgescript_var_get_rawbits(_t[2]) <= 0)
                return a.u;
            break;

        case SSOP_JGE:
            if(surgescript_var_get_rawbits(_t[2]) >= 0)
                return a.u;
            break;

        /* function calls */
        case SSOP_RET:
            return ssarray_length(program->line);

        case SSOP_CALL:
            if(a.u < ssarray_length(program->text)) {
                surgescript_objectclassid_t class_id = 0;
                surgescript_program_t* callee_program = call_program(runtime_environment, b.u, program->text[a.u], NULL, &class_id);

#if WANT_OPTIMIZED_PROGRAM_CALLS
                /* count the number of consecutive times the program has been
                   executed with this same callee or equivalent object of the
                   same class (a == class_id; b == count) */
                if(operation[1].a.u == class_id) {
                    if(++operation[1].b.i >= OPTIMIZED_CALL_THRESHOLD) {
                        /* the program has run enough consecutive times with
                           the same or equivalent callee. Let's optimize. */

                        /* save the current operands */
                        operation[2].a = a;
                        operation[2].b = b;

                        /* let's change this instruction */
                        operation->instruction = SSOP_OPTCALL;
                        operation->a.p = callee_program;
                        operation->b.u = b.u;
                    }
                }
                else {
                    /* new class. Reset the counter */
                    operation[1].a.u = class_id;
                    operation[1].b.i = 1;
                }

                /* skip the two NOPs placed after every CALL */
                return ip += 3;
#endif
            }
            break;

        case SSOP_OPTCALL:
#if WANT_OPTIMIZED_PROGRAM_CALLS
            /* run the cached program. We can afford to cache because
               surgescript_program_t* entries of the program pool will not
               change after execution */
            if(!call_program(runtime_environment, b.u, NULL, a.p, &operation[1].a.u)) {

                /* Got a different callee. Execution was aborted! */

                /* restore the original CALL */
                operation->instruction = SSOP_CALL;
                operation->a = operation[2].a;
                operation->b = operation[2].b;

                /* reset the counter */
                operation[1].b.i = 0;

                /* run the same instruction again */
                return ip;

            }
            else {

                /* Execution was successful! */

                /* skip the two NOPs placed after every CALL */
                return ip += 3;

            }
#else
            break;
#endif
    }

    /* next line */
    return ip + 1;

    /* done */
    #undef t
}

/* calls a program */
surgescript_program_t* call_program(const surgescript_renv_t* caller_runtime_environment, int number_of_given_params, const char* program_name, surgescript_program_t* cached_program, surgescript_objectclassid_t* inout_class_id)
{
    surgescript_program_t* program = NULL;

    /* preparing the stack */
    surgescript_stack_t* stack = surgescript_renv_stack(caller_runtime_environment);
    surgescript_stack_pushenv(stack);

    /* there is a program to be called */
    surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(caller_runtime_environment);
    const surgescript_var_t* callee = surgescript_stack_peek(stack, -1 - number_of_given_params); /* 1st param, left-to-right */
    surgescript_objecthandle_t object_handle = surgescript_var_get_objecthandle(callee);

    /* surgescript can also call programs on primitive types */
    if(!surgescript_var_is_objecthandle(callee)) /* callee is of a primitive type */
        number_of_given_params++; /* object_handle points to the appropriate wrapper */

    /* find the program */
    if(surgescript_objectmanager_exists(manager, object_handle)) {
        surgescript_programpool_t* pool = surgescript_renv_programpool(caller_runtime_environment);
        surgescript_object_t* object = surgescript_objectmanager_get(manager, object_handle);
        const char* object_name = surgescript_object_name(object);
        surgescript_objectclassid_t class_id = surgescript_object_class_id(object);

        if(cached_program == NULL) {
            /* do a program lookup. this is a bottleneck!
               use cached_program if possible */
            program = surgescript_programpool_get(pool, object_name, program_name);
            *inout_class_id = class_id;
        }
        else {
            /* we're using a cached program.
               Abort if callee is incompatible */
            program = cached_program;
            if(class_id != *inout_class_id) {
                program = NULL;
                goto cleanup;
            }
        }
        
        /* does the selected program exist? */
        if(program != NULL) {
            if(number_of_given_params == program->arity) {
                /* the parameters are pushed onto the stack (left-to-right) */
                surgescript_renv_t callee_runtime_environment = {
                    object,
                    stack,
                    surgescript_object_heap(object),
                    pool,
                    manager,
                    surgescript_renv_tmp(caller_runtime_environment),
                    NULL,
                    surgescript_object_handle(surgescript_renv_owner(caller_runtime_environment))
                };

                /* call the program */
                program->run(program, &callee_runtime_environment);

                /* callee_tmp[0] = caller_tmp[0] is the return value of the program (so, no need to copy anything) */
                /*surgescript_var_copy(*surgescript_renv_tmp(caller_runtime_environment), *surgescript_renv_tmp(&callee_runtime_environment));*/
            }
            else
                ssfatal("Runtime Error: function %s.%s (called in \"%s\") expects %d parameters, but received %d.", object_name, program_name, surgescript_object_name(surgescript_renv_owner(caller_runtime_environment)), program->arity, number_of_given_params);
        }
        else
            ssfatal("Runtime Error: can't find function %s.%s (called in \"%s\").", object_name, program_name, surgescript_object_name(surgescript_renv_owner(caller_runtime_environment)));
    }
    else
        ssfatal("Runtime Error: null pointer exception - can't call function %s (called in \"%s\").", program_name, surgescript_object_name(surgescript_renv_owner(caller_runtime_environment)));

    /* clean up */
    cleanup:
    surgescript_stack_popenv(stack); /* clear stack frame, including a unknown number of local variables */

    /* done */
    return program;
}

/* writes data to buf, in hex/big-endian format (writes (1 + 2 * sizeof(unsigned)) bytes to buf) */
char* hexdump(unsigned data, char* buf)
{
    const char* bytes = (const char*)(&data);
    char n1, n2, *p = buf;
    int i;

    data = surgescript_util_htob(data);
    for(i = 0; i < sizeof(data); i++) {
        n1 = (bytes[i] >> 4) & 0xF;
        n2 = bytes[i] & 0xF;
        *(p++) = n1 + (n1 <= 9 ? '0' : 'a' - 10);
        *(p++) = n2 + (n2 <= 9 ? '0' : 'a' - 10);
    }

    *p = 0;
    return buf;
}

/* works like fputs, but escapes the string */
void fputs_escaped(const char* str, FILE* fp)
{
    for(const char* p = str; p && *p; p++) {
        switch(*p) {
            case '\\': fputs("\\\\", fp); break;
            case '\"': fputs("\\\"", fp); break;
            case '\'': fputs("\\'", fp); break;
            case '\n': fputs("\\n", fp); break;
            case '\r': fputs("\\r", fp); break;
            case '\t': fputs("\\t", fp); break;
            case '\v': fputs("\\v", fp); break;
            case '\f': fputs("\\f", fp); break;
            default: fputc(*p, fp); break;
        }
    }
}

/* is this a jump instruction? */
bool is_jump_instruction(surgescript_program_operator_t instruction)
{
    switch(instruction)
    {
        case SSOP_JMP:
        case SSOP_JE:
        case SSOP_JNE:
        case SSOP_JG:
        case SSOP_JGE:
        case SSOP_JL:
        case SSOP_JLE:
            return true;
        default:
            return false;
    }
}

/* removes all labels from the program, placing the correct line numbers
   on all jump instructions. Returns true if there were any removed labels. */
bool remove_labels(surgescript_program_t* program)
{
    if(ssarray_length(program->label) > 0) {
        /* correct all jump instructions */
        for(int i = 0; i < ssarray_length(program->line); i++) {
            if(is_jump_instruction(program->line[i].instruction)) {
                surgescript_program_label_t label = program->line[i].a.u;
                if(label >= 0 && label < ssarray_length(program->label))
                    program->line[i].a.u = program->label[label];
                else
                    ssfatal("Runtime Error: invalid jump instruction - unknown label.");
            }
        }

        /* no more labels */
        ssarray_reset(program->label);
        return true;
    }
    else
        return false;
}

/* debug mode */
#if SURGESCRIPT_DEBUG_MODE
void debug(surgescript_program_t* program, surgescript_renv_t* runtime_environment, surgescript_program_operator_t instruction, surgescript_program_operand_t a, surgescript_program_operand_t b, surgescript_var_t** _t)
{
    int i;
    char hex[2][1 + 2 * sizeof(unsigned)];

    if(instruction == SSOP_NOP && a.i == -1) {
        const char* title = surgescript_program_get_text(program, b.u);
        const surgescript_var_t* ptr = surgescript_stack_peek(surgescript_renv_stack(runtime_environment), 0);
        const surgescript_var_t* top = surgescript_stack_top(surgescript_renv_stack(runtime_environment));
        char* contents_of_t[] = {
            surgescript_var_get_string(_t[0], NULL),
            surgescript_var_get_string(_t[1], NULL),
            surgescript_var_get_string(_t[2], NULL),
            surgescript_var_get_string(_t[3], NULL)
        };

        /* breakpoint! */
        printf(".. BREAKPOINT %s\n", title);

        /* print stack */
        printf("..\tSTACK\t");
        for(i = -program->arity; ptr != top; i++) {
            if(i != 0) {
                char* contents = surgescript_var_get_string(
                    ptr = surgescript_stack_peek(surgescript_renv_stack(runtime_environment), i),
                    surgescript_renv_objectmanager(runtime_environment)
                );
                printf("%s || ", contents);
                ssfree(contents);
            }
            else
                printf("|| prev_bp || ");
        }

        /* print temps */
        for(i = 0; i < 4; i++) {
            printf("\n..\tT%d\t%08X\t%s", i, (unsigned)surgescript_var_get_rawbits(_t[i]), contents_of_t[i]);
            ssfree(contents_of_t[i]);
        }

        /* print text data */
        for(i = 0; i < ssarray_length(program->text); i++)
            printf("\n..\tTXT%d\t%s", i, program->text[i]);
        printf("\n..");

        /* done! */
        getchar();
    }
    else
        printf("..\t%s\t%s\t%s\n", instruction_name[instruction], hexdump(a.u, hex[0]), hexdump(b.u, hex[1]));
}
#endif



/* --------------- numeric utilities --------------- */

/* returns -1 if f < 0, 0 if if == 0, 1 if f > 0 */
int fast_sign(double f)
{
    return (f > 0.0) - (f < 0.0);
}

/* returns -1 if f <= -0, 1 if f >= +0 */
int fast_sign1(double f)
{
    return signbit(f) == 0 ? 1 : -1;
}

/* returns "true" iff f != +0 and f != -0 */
int fast_notzero(double f)
{
    return fpclassify(f) != FP_ZERO;
}
