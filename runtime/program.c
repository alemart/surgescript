/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/program.c
 * SurgeScript program
 */

#include <assert.h>
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
    int arity; /* config */
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

/* the names of the instructions */
static const char* instruction_name[] = {
    #define PRINT_NAME(x, y) y,
    SURGESCRIPT_PROGRAM_OPERATORS(PRINT_NAME)
};

/* utilities */
static surgescript_program_t* init_program(surgescript_program_t* program, int arity, void (*run_function)(surgescript_program_t*, surgescript_renv_t*));
static void run_program(surgescript_program_t* program, surgescript_renv_t* runtime_environment);
static void run_cprogram(surgescript_program_t* program, surgescript_renv_t* runtime_environment);
static inline void run_instruction(surgescript_program_t* program, surgescript_renv_t* runtime_environment, surgescript_program_operator_t instruction, surgescript_program_operand_t a, surgescript_program_operand_t b, int* ip);
static inline void call_program(surgescript_renv_t* caller_runtime_environment, const char* program_name, int number_of_given_params);
static inline bool is_jump_instruction(surgescript_program_operator_t instruction);
static inline bool remove_labels(surgescript_program_t* program);
static char* hexdump(unsigned data, char* buf); /* writes the bytes stored in data to buf, in hex format */
static void fputs_escaped(const char* str, FILE* fp); /* works like fputs, but escapes the string */
static inline int fast_float_sign(float f);
static inline int fast_float_sign1(float f);
static inline int fast_float_notzero(float f);

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
 * surgescript_cprogram_create()
 * Creates a program that encapsulates a C-function
 * Please note that this C-function must return a newly-allocated surgescript_var_t*, or NULL
 */
surgescript_program_t* surgescript_cprogram_create(int arity, surgescript_program_cfunction_t cfunction)
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
int surgescript_program_add_line(surgescript_program_t* program, surgescript_program_operator_t op, surgescript_program_operand_t a, surgescript_program_operand_t b)
{
    surgescript_program_operation_t line = { op, a, b };
    ssarray_push(program->line, line);
    return ssarray_length(program->line) - 1;
}

/*
 * surgescript_program_chg_line()
 * changes an existing line of code of the program
 */
int surgescript_program_chg_line(surgescript_program_t* program, int line, surgescript_program_operator_t op, surgescript_program_operand_t a, surgescript_program_operand_t b)
{
    surgescript_program_operation_t newline = { op, a, b };
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
 * surgescript_program_run()
 * Runs a written program
 */
void surgescript_program_run(surgescript_program_t* program, surgescript_renv_t* runtime_environment)
{
    program->run(program, runtime_environment);
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

    /* print labels */
    /*fprintf(fp,
        "    ],\n"
        "    \"labels\": [ "
    );

    for(i = 0; i < ssarray_length(program->label); i++)
        fprintf(fp, "%d%s ", program->label[i], (i < ssarray_length(program->label) - 1) ? "," : "");*/

    /* print footer */
    /*fprintf(fp, "]\n}\n");*/
}


/*
 * surgescript_program_lowcall()
 * Low-level SurgeScript function call.
 * you'll need to manage the stack yourself (prefer using surgescript_object_call_program() instead)
*/
void surgescript_program_lowcall(surgescript_renv_t* runtime_environment, const char* program_name, int num_params)
{
    call_program(runtime_environment, program_name, num_params);
}



/* -------------------------------
 * private stuff
 * ------------------------------- */

/* initializes a program */
surgescript_program_t* init_program(surgescript_program_t* program, int arity, void (*run_function)(surgescript_program_t*, surgescript_renv_t*))
{
    program->arity = arity;
    program->run = run_function;

    ssarray_init(program->line);
    ssarray_init(program->label);
    ssarray_init(program->text);

    return program;
}

/* runs a program */
void run_program(surgescript_program_t* program, surgescript_renv_t* runtime_environment)
{
    int ip = 0; /* instruction pointer */
    remove_labels(program);

    while(ip < ssarray_length(program->line))
        run_instruction(program, runtime_environment, program->line[ip].instruction, program->line[ip].a, program->line[ip].b, &ip);
}

/* runs a C-program */
void run_cprogram(surgescript_program_t* program, surgescript_renv_t* runtime_environment)
{
    surgescript_cprogram_t* cprogram = (surgescript_cprogram_t*)program;
    surgescript_object_t* caller = surgescript_renv_owner(runtime_environment);
    surgescript_stack_t* stack = surgescript_renv_stack(runtime_environment);
    surgescript_var_t** param = program->arity > 0 ? ssmalloc(program->arity * sizeof(*param)) : NULL;
    surgescript_var_t* return_value = NULL;

    /* grab parameters from the stack (stacked in left-to-right order) */
    for(int i = 1; i <= program->arity; i++)
        param[program->arity-i] = surgescript_stack_at(stack, -i);

    /* call C-function */
    return_value = (surgescript_var_t*)(cprogram->cfunction(caller, (const surgescript_var_t**)param, program->arity));
    if(return_value != NULL) {
        surgescript_var_copy(*(surgescript_renv_tmp(runtime_environment) + 0), return_value);
        surgescript_var_destroy(return_value);
    }
    else
        surgescript_var_set_null(*(surgescript_renv_tmp(runtime_environment) + 0));

    /* release parameters */
    if(param)
        ssfree(param);
}

/* runs an instruction */
void run_instruction(surgescript_program_t* program, surgescript_renv_t* runtime_environment, surgescript_program_operator_t instruction, surgescript_program_operand_t a, surgescript_program_operand_t b, int* ip)
{
    /* temporary variables */
    surgescript_var_t** _t = surgescript_renv_tmp(runtime_environment);

    /* helper macro */
    #ifdef t
    #undef t
    #endif
    #define t(k)             _t[(k).u & 3]

    /* debug mode */
    //#define SSDEBUG
    #ifdef SSDEBUG
    do {
        char hex[2][1 + 2 * sizeof(unsigned)];
        if(instruction == SSOP_NOP && a.i == -1) {
            int i;
            const char* title = surgescript_program_get_text(program, b.u);
            surgescript_var_t* ptr = surgescript_stack_at(surgescript_renv_stack(runtime_environment), 0);
            surgescript_var_t* top = surgescript_stack_top(surgescript_renv_stack(runtime_environment));
            char* contents_of_t[] = {
                surgescript_var_get_string(_t[0]),
                surgescript_var_get_string(_t[1]),
                surgescript_var_get_string(_t[2]),
                surgescript_var_get_string(_t[3])
            };

            /* print temps */
            printf(".. BREAKPOINT %s\n", title);
            for(i = 0; i < 4; i++) {
                printf("..\t%d\t%08X\t%s\n", i, (unsigned)surgescript_var_get_rawbits(_t[i]), contents_of_t[i]);
                ssfree(contents_of_t[i]);
            }

            /* print stack */
            printf("..\tstack\t");
            for(i = -program->arity; ptr != top; i++) {
                if(i != 0) {
                    char* contents = surgescript_var_get_string(
                        ptr = surgescript_stack_at(surgescript_renv_stack(runtime_environment), i)
                    );
                    printf("%s ", contents);
                    ssfree(contents);
                }
                else
                    printf("prev_bp ");
            }
            printf("\n..");

            /* done! */
            getchar();
        }
        else
            printf("..\t%s\t%s\t%s\n", instruction_name[instruction], hexdump(a.u, hex[0]), hexdump(b.u, hex[1]));
    } while(0);
    #endif

    /* run the instruction */
    switch(instruction) {
        /* basics */
        case SSOP_NOP: /* no-operation */
            break;

        case SSOP_SELF: /* move caller object ("this" pointer) */
            surgescript_var_set_objecthandle(t(a), surgescript_object_handle(surgescript_renv_owner(runtime_environment)));
            break;

        case SSOP_STATE: /* t[a] receives the current state. If b == -1, then the current state is set to t[a] instead. */
            if(b.i == -1) {
                char state[256];
                surgescript_var_to_string(t(a), state, sizeof(state));
                surgescript_object_set_state(surgescript_renv_owner(runtime_environment), state);
            }
            else
                surgescript_var_set_string(t(a), surgescript_object_state(surgescript_renv_owner(runtime_environment)));
            break;

        /* assignment operations */
        case SSOP_MOVN: /* move null */
            surgescript_var_set_null(t(a));
            break;

        case SSOP_MOVB: /* move boolean */
            surgescript_var_set_bool(t(a), b.b);
            break;

        case SSOP_MOVF: /* move number (float) */
            surgescript_var_set_number(t(a), b.f);
            break;

        case SSOP_MOVS: /* move string */
            if(b.u < ssarray_length(program->text))
                surgescript_var_set_string(t(a), program->text[b.u]);
            break;

        case SSOP_MOVO: /* move object handle */
            surgescript_var_set_objecthandle(t(a), b.u);
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

        case SSOP_STORE:
            surgescript_var_copy(surgescript_heap_at(surgescript_renv_heap(runtime_environment), (surgescript_heapptr_t)surgescript_var_get_number(t(b))), t(a));
            break;

        case SSOP_LOAD:
            surgescript_var_copy(t(a), surgescript_heap_at(surgescript_renv_heap(runtime_environment), (surgescript_heapptr_t)surgescript_var_get_number(t(b))));
            break;

        case SSOP_PEEK:
            surgescript_var_copy(t(a), surgescript_heap_at(surgescript_renv_heap(runtime_environment), b.u));
            break;

        case SSOP_POKE:
            surgescript_var_copy(surgescript_heap_at(surgescript_renv_heap(runtime_environment), b.u), t(a));
            break;

        /* stack operations */
        case SSOP_PUSH:
            surgescript_var_copy(surgescript_stack_push(surgescript_renv_stack(runtime_environment), surgescript_var_create()), t(a));
            break;

        case SSOP_POP:
            surgescript_var_copy(t(a), surgescript_stack_top(surgescript_renv_stack(runtime_environment)));
            surgescript_stack_pop(surgescript_renv_stack(runtime_environment));
            break;

        case SSOP_SPEEK:
            surgescript_var_copy(t(a), surgescript_stack_at(surgescript_renv_stack(runtime_environment), b.i));
            break;

        case SSOP_SPOKE:
            surgescript_var_copy(surgescript_stack_at(surgescript_renv_stack(runtime_environment), b.i), t(a));
            break;

        case SSOP_PUSHN:
            surgescript_stack_pushn(surgescript_renv_stack(runtime_environment), a.u);
            break;

        case SSOP_POPN:
            surgescript_stack_popn(surgescript_renv_stack(runtime_environment), a.u);
            break;

        /* basic arithmetic */
        case SSOP_INC:
            surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)) + 1.0f); /* TODO: inc op */
            break;

        case SSOP_DEC:
            surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)) - 1.0f); /* TODO: dec op */
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
            if(fast_float_notzero(surgescript_var_get_number(t(b))))
                surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)) / surgescript_var_get_number(t(b)));
            else if(fast_float_sign(surgescript_var_get_number(t(a))) >= 0)
                surgescript_var_set_number(t(a), INFINITY * fast_float_sign1(surgescript_var_get_number(t(b))));
            else
                surgescript_var_set_number(t(a), -INFINITY * fast_float_sign1(surgescript_var_get_number(t(b))));
            break;

        case SSOP_NEG:
            surgescript_var_set_number(t(a), -surgescript_var_get_number(t(b)));
            break;

        case SSOP_LNOT:
            surgescript_var_set_bool(t(a), !surgescript_var_get_bool(t(b)));
            break;

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

        case SSOP_TCMP:
            surgescript_var_set_rawbits(_t[2], surgescript_var_typecode(t(a)) - surgescript_var_typecode(t(b)));
            break;

        case SSOP_CMP:
            surgescript_var_set_rawbits(_t[2], surgescript_var_compare(t(a), t(b)));
            break;

        /* jumping */
        case SSOP_JMP:
            *ip = a.u;
            return;

        case SSOP_JE:
            if(!fast_float_notzero(surgescript_var_get_number(_t[2]))) {
                *ip = a.u;
                return;
            }
            else
                break;

        case SSOP_JNE:
            if(fast_float_notzero(surgescript_var_get_number(_t[2]))) {
                *ip = a.u;
                return;
            }
            else
                break;

        case SSOP_JL:
            if(fast_float_sign(surgescript_var_get_number(_t[2])) < 0) {
                *ip = a.u;
                return;
            }
            else
                break;

        case SSOP_JG:
            if(fast_float_sign(surgescript_var_get_number(_t[2])) > 0) {
                *ip = a.u;
                return;
            }
            else
                break;

        case SSOP_JLE:
            if(fast_float_sign(surgescript_var_get_number(_t[2])) <= 0) {
                *ip = a.u;
                return;
            }
            else
                break;

        case SSOP_JGE:
            if(fast_float_sign(surgescript_var_get_number(_t[2])) >= 0) {
                *ip = a.u;
                return;
            }
            else
                break;

        /* function calls */
        case SSOP_CALL: {
            const char* program_name = (a.u < ssarray_length(program->text)) ? program->text[a.u] : "";
            unsigned number_of_given_params = b.u;
            call_program(runtime_environment, program_name, number_of_given_params);
            break;
        }

        case SSOP_RET:
            *ip = ssarray_length(program->line);
            return;
    }

    /* next line */
    ++(*ip);
}

/* calls a program */
void call_program(surgescript_renv_t* caller_runtime_environment, const char* program_name, int number_of_given_params)
{
    /* preparing the stack */
    surgescript_stack_t* stack = surgescript_renv_stack(caller_runtime_environment);
    surgescript_stack_pushenv(stack);

    /* there is a program to be called */
    if(*program_name) {
        surgescript_objectmanager_t* manager = surgescript_renv_objectmanager(caller_runtime_environment);
        surgescript_var_t* callee = surgescript_stack_at(stack, -1 - number_of_given_params); /* 1st param, left-to-right */
        const int callee_type = surgescript_var_typecode(callee);
        unsigned object_handle = 0;

        /* surgescript can also call programs on primitive types */
        switch(callee_type) {
            case 's': /* for the sake of efficiency... we replaced surgescript_var_type2code() for constants */
                object_handle = surgescript_objectmanager_system_object(manager, "String");
                number_of_given_params++;
                break;

            case 'n':
                object_handle = surgescript_objectmanager_system_object(manager, "Number");
                number_of_given_params++;
                break;

            case 'b':
                object_handle = surgescript_objectmanager_system_object(manager, "Boolean");
                number_of_given_params++;
                break;

            default: /* this is not a primitive type */
                object_handle = surgescript_var_get_objecthandle(callee);
                break;
        }

        /* finds the program */
        if(surgescript_objectmanager_exists(manager, object_handle)) {
            surgescript_object_t* object = surgescript_objectmanager_get(manager, object_handle);
            const char* object_name = surgescript_object_name(object);
            surgescript_program_t* program = surgescript_programpool_get(surgescript_renv_programpool(caller_runtime_environment), object_name, program_name);
            
            /* does the selected program exist? */
            if(program) {
                if(number_of_given_params == program->arity) {
                    /* the parameters are pushed onto the stack (left-to-right) */
                    surgescript_renv_t* callee_runtime_environment = surgescript_renv_create(
                        object,
                        stack,
                        surgescript_object_heap(object),
                        surgescript_renv_programpool(caller_runtime_environment),
                        manager,
                        surgescript_renv_tmp(caller_runtime_environment)
                    );

                    /* call the program */
                    surgescript_program_run(program, callee_runtime_environment);

                    /* callee_tmp[0] = caller_tmp[0] is the return value of the program (so, no need to copy anything) */
                    /*surgescript_var_copy(*surgescript_renv_tmp(caller_runtime_environment), *surgescript_renv_tmp(callee_runtime_environment));*/
                    surgescript_renv_destroy(callee_runtime_environment);
                }
                else
                    ssfatal("Runtime Error: function %s.%s (called in \"%s\") expects %d parameters, but received %d.", object_name, program_name, surgescript_object_name(surgescript_renv_owner(caller_runtime_environment)), program->arity, number_of_given_params);
            }
            else
                ssfatal("Runtime Error: can't find function %s.%s (called in \"%s\").", object_name, program_name, surgescript_object_name(surgescript_renv_owner(caller_runtime_environment)));
        }
        else
            ssfatal("Runtime Error: null pointer exception - can't call function %s (called in \"%s\").", program_name, surgescript_object_name(surgescript_renv_owner(caller_runtime_environment)));
    }

    /* clean up */
    surgescript_stack_popenv(stack); /* clear stack frame, including a unknown number of local variables */
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
   on all jump instructions. Returns true if there were removed labels. */
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


/* --------------- float utilities --------------- */
#define FAST_FLOAT_FUNCTIONS
#ifdef FAST_FLOAT_FUNCTIONS

static_assert(
    sizeof(float) == 4 && sizeof(float) == sizeof(int),
    "Code relies on float taking 4 bytes under the IEEE-754 floating-point representation. "
    "Undefine FAST_FLOAT_FUNCTIONS in " __FILE__ " to fix this."
);

/* returns -1 if f < 0, 0 if if == 0, 1 if f > 0 */
int fast_float_sign(float f)
{
    return (*((int*)&f) & 0x7FFFFFFF) ? (1 - ((*((int*)&f) & 0x80000000) >> 30)) : 0;
}

/* returns -1 if f <= -0, 1 if f >= +0 */
int fast_float_sign1(float f)
{
    return 1 - ((*((int*)&f) & 0x80000000) >> 30);
}

/* returns "true" iff f != +0 and f != -0 */
int fast_float_notzero(float f)
{
    return (*((int*)&f) & 0x7FFFFFFF);
}

#else

/* returns -1 if f < 0, 0 if if == 0, 1 if f > 0 */
int fast_float_sign(float f)
{
    return fast_float_notzero(f) ? fast_float_sign1(f) : 0;
}

/* returns -1 if f <= -0, 1 if f >= +0 */
int fast_float_sign1(float f)
{
    return signbit(f) == 0 ? 1 : -1;
}

/* returns "true" iff f != +0 and f != -0 */
int fast_float_notzero(float f)
{
    return fpclassify(f) != FP_ZERO;
}

#endif