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
    int arity; /* config */
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

/* the names of the instructions */
static const char* instruction_name[] = {
    #define PRINT_NAME(x, y) y,
    SURGESCRIPT_PROGRAM_OPERATORS(PRINT_NAME)
};

/* utilities */
static surgescript_program_t* init_program(surgescript_program_t* program, int arity, void (*run_function)(surgescript_program_t*, surgescript_renv_t*));
static void run_program(surgescript_program_t* program, surgescript_renv_t* runtime_environment);
static void run_cprogram(surgescript_program_t* program, surgescript_renv_t* runtime_environment);
static inline void run_instruction(surgescript_program_t* program, surgescript_renv_t* runtime_environment, surgescript_program_operator_t instruction, surgescript_program_operand_t a, surgescript_program_operand_t b);
static void call_object_method(surgescript_renv_t* caller_runtime_environment, unsigned object_handle, const char* program_name, int number_of_given_params, surgescript_var_t* return_value);
static char* hexdump32(unsigned data, char* buf); /* writes 9 bytes to buf */
static void fputs_escaped(const char* str, FILE* fp); /* works like fputs, but escapes the string */
static inline bool is_jump_instruction(surgescript_program_operator_t instruction);
static bool remove_labels(surgescript_program_t* program);
static inline int fast_float_sign(float f);
static inline int fast_float_sign1(float f);


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
    char hex[2][16];
    surgescript_program_operation_t* op;

    remove_labels(program);

    /* print header */
    fprintf(fp,
        "{\n"
        "    \"arity\": %d\n"
        "    \"code\": [\n",
    program->arity);

    /* print code */
    for(i = 0; i < ssarray_length(program->line); i++) {
        op = &(program->line[i]);
        fprintf(fp,
            "        \"%s\t  %s    %s\"%s\n",
            instruction_name[op->instruction],
            hexdump32(op->a.u, hex[0]),
            hexdump32(op->b.u, hex[1]),
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

/* -------------------------------
 * private stuff
 * ------------------------------- */

/* initializes a program */
surgescript_program_t* init_program(surgescript_program_t* program, int arity, void (*run_function)(surgescript_program_t*, surgescript_renv_t*))
{
    program->arity = arity;
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
    remove_labels(program);

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
        param[i-1] = surgescript_stack_at(stack, -i-1);

    /* call C-function */
    return_value = (surgescript_var_t*)(cprogram->cfunction(caller, (const surgescript_var_t**)param, program->arity));
    if(return_value != NULL) {
        surgescript_var_copy(*(surgescript_renv_tmp(runtime_environment) + 3), return_value);
        surgescript_var_destroy(return_value);
    }
    else
        surgescript_var_set_null(*(surgescript_renv_tmp(runtime_environment) + 3));

    /* release parameters */
    if(param)
        ssfree(param);
}

/* runs an instruction */
void run_instruction(surgescript_program_t* program, surgescript_renv_t* runtime_environment, surgescript_program_operator_t instruction, surgescript_program_operand_t a, surgescript_program_operand_t b)
{
    /* temporary variables */
    surgescript_var_t** _t = surgescript_renv_tmp(runtime_environment);
    #define t(k)             _t[(k.u) & 3]

    /* run the instruction */
    switch(instruction) {
        /* NOP */
        case SSOP_NOP:
            break;

        case SSOP_OUT: {
            char* str = surgescript_var_get_string(t(a));
            printf("%s\n", str);
            ssfree(str);
            break;
        }

        case SSOP_IN: {
            char buf[81];
            scanf("%80s", buf);
            surgescript_var_set_string(t(a), buf);
            break;
        }

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

        case SSOP_MOVC: /* move caller object ("this" pointer) */
            surgescript_var_set_objecthandle(t(a), surgescript_object_handle(surgescript_renv_owner(runtime_environment)));
            break;

        case SSOP_MOVT: /* move temp */
            surgescript_var_copy(t(a), t(b));
            break;

        /* heap operations */
        case SSOP_ALOC:
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
            surgescript_var_copy(surgescript_stack_at(surgescript_renv_stack(runtime_environment), b.i),  t(a));
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
            if(fast_float_sign(surgescript_var_get_number(t(b))) != 0)
                surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)) / surgescript_var_get_number(t(b)));
            else if(fast_float_sign(surgescript_var_get_number(t(a))) >= 0)
                surgescript_var_set_number(t(a), INFINITY * fast_float_sign1(surgescript_var_get_number(t(b))));
            else
                surgescript_var_set_number(t(a), -INFINITY * fast_float_sign1(surgescript_var_get_number(t(b))));
            break;

        case SSOP_NEG:
            surgescript_var_set_number(t(a), -surgescript_var_get_number(t(a)));
            break;

        case SSOP_NOT:
            surgescript_var_set_bool(t(a), !surgescript_var_get_bool(t(a)));
            break;

        case SSOP_AND:
            if(surgescript_var_get_bool(t(a)))
                surgescript_var_set_bool(t(a), surgescript_var_get_bool(t(b)));
            else
                surgescript_var_set_bool(t(a), false);
            break;

        case SSOP_OR:
            if(surgescript_var_get_bool(t(a)))
                surgescript_var_set_bool(t(a), true);
            else
                surgescript_var_set_bool(t(a), surgescript_var_get_bool(t(b)));
            break;

        case SSOP_CAT: {
            char* str1 = surgescript_var_get_string(t(a));
            char* str2 = surgescript_var_get_string(t(b));
            char* str = ssmalloc((1 + strlen(str1) + strlen(str2)) * sizeof(*str));
            surgescript_var_set_string(t(a), strcat(strcpy(str, str1), str2));
            ssfree(str);
            ssfree(str2);
            ssfree(str1);
            break;
        }

        /*case SSOP_STRLEN: {
            char* buf = surgescript_var_get_string(t(a));
            int len = strlen(buf); // TODO: utf-8 compat
            surgescript_var_set_number(t(a), len);
            ssfree(buf);
            break;
        }

        case SSOP_STRMID: {
            char* substr;
            char* str = surgescript_var_get_string(t(a));
            int st(a)rt = surgescript_var_get_number(t(b));
            int length = surgescript_var_get_number(t[c.u]);
            int n = strlen(str); // TODO: utf-8 compat

            st(a)rt = ssclamp(st(a)rt, 0, n);
            length = ssclamp(length, 0, n - st(a)rt);
            substr = ssmalloc((1 + length) * sizeof(*substr));
            surgescript_util_strncpy(substr, str + st(a)rt, length);
            surgescript_var_set_string(t(a), substr);

            ssfree(substr);
            ssfree(str);
            break;
        }

        case SSOP_STRAT: {
            char* str = surgescript_var_get_string(t(a));
            int index = surgescript_var_get_number(t(b));
            int n = strlen(str); // TODO: utf-8 compat

            char buf[2] = { ' ', '\0' };
            if(index >= 0 && index < n)
                buf[0] = str[index];
            surgescript_var_set_string(t(a), buf);

            ssfree(str);
            break;
        }
*/

        /* casting */
        case SSOP_BOOL:
            surgescript_var_set_bool(t(a), surgescript_var_get_bool(t(a)));
            break;

        case SSOP_VAL:
            surgescript_var_set_number(t(a), surgescript_var_get_number(t(a)));
            break;

        case SSOP_STR: {
            char* buf = surgescript_var_get_string(t(a));
            surgescript_var_set_string(t(a), buf);
            ssfree(buf);
            break;
            /*
                const char* program_name = "toString";
                unsigned object_handle = surgescript_var_get_objecthandle(t(a));
                call_object_method(runtime_environment, object_handle, program_name, 0, t(a));
            */
        }

        /* comparing */
        case SSOP_CMP:
            surgescript_var_set_number(_t[2], surgescript_var_compare(t(a), t(b)));
            break;

        case SSOP_TEST:
            surgescript_var_set_number(_t[2], surgescript_var_get_rawbits(t(a)) & surgescript_var_get_rawbits(t(b)));
            break;

        case SSOP_TCHK:
            surgescript_var_set_number(_t[2], surgescript_var_typecode(t(a)) - surgescript_var_typecode(t(b)));
            break;

        case SSOP_TCHK0:
            surgescript_var_set_number(_t[2], surgescript_var_typecode(t(a)) - surgescript_var_type2code(NULL));
            break;

        case SSOP_TCHKB:
            surgescript_var_set_number(_t[2], surgescript_var_typecode(t(a)) - surgescript_var_type2code("boolean"));
            break;

        case SSOP_TCHKN:
            surgescript_var_set_number(_t[2], surgescript_var_typecode(t(a)) - surgescript_var_type2code("number"));
            break;

        case SSOP_TCHKS:
            surgescript_var_set_number(_t[2], surgescript_var_typecode(t(a)) - surgescript_var_type2code("string"));
            break;

        case SSOP_TCHKO:
            surgescript_var_set_number(_t[2], surgescript_var_typecode(t(a)) - surgescript_var_type2code("object"));
            break;

        /* jumping */
        case SSOP_JMP:
            program->ip = a.u;
            return;

        case SSOP_JE:
            if(fast_float_sign(surgescript_var_get_number(_t[2])) == 0) {
                program->ip = a.u;
                return;
            }
            else
                break;

        case SSOP_JNE:
            if(fast_float_sign(surgescript_var_get_number(_t[2])) != 0) {
                program->ip = a.u;
                return;
            }
            else
                break;

        case SSOP_JL:
            if(fast_float_sign(surgescript_var_get_number(_t[2])) < 0) {
                program->ip = a.u;
                return;
            }
            else
                break;

        case SSOP_JG:
            if(fast_float_sign(surgescript_var_get_number(_t[2])) > 0) {
                program->ip = a.u;
                return;
            }
            else
                break;

        case SSOP_JLE:
            if(fast_float_sign(surgescript_var_get_number(_t[2])) <= 0) {
                program->ip = a.u;
                return;
            }
            else
                break;

        case SSOP_JGE:
            if(fast_float_sign(surgescript_var_get_number(_t[2])) >= 0) {
                program->ip = a.u;
                return;
            }
            else
                break;

        /* function calls */
        case SSOP_CALL: {
            const char* program_name = (a.u < ssarray_length(program->text)) ? program->text[a.u] : "";
            unsigned object_handle = surgescript_var_get_objecthandle(surgescript_stack_top(surgescript_renv_stack(runtime_environment)));
            unsigned number_of_given_params = b.u;
            surgescript_var_t* return_value = _t[2];
            call_object_method(runtime_environment, object_handle, program_name, number_of_given_params, return_value);
            break;
        }

        case SSOP_RET:
            program->ip = ssarray_length(program->line);
            return;
    }

    /* next line */
    program->ip++;
}

/* calls a method */
void call_object_method(surgescript_renv_t* caller_runtime_environment, unsigned object_handle, const char* program_name, int number_of_given_params, surgescript_var_t* return_value)
{
    surgescript_object_t* object = surgescript_objectmanager_get(surgescript_renv_objectmanager(caller_runtime_environment), object_handle);
    const char* object_name = surgescript_object_name(object);
    surgescript_program_t* prog = surgescript_programpool_get(surgescript_renv_programpool(caller_runtime_environment), object_name, program_name);
    int expected_params = prog->arity;

    if(number_of_given_params == expected_params) {
        /* the parameters are pushed onto the stack (reverse order) */
        surgescript_stack_t* stack = surgescript_renv_stack(caller_runtime_environment);
        surgescript_renv_t* callee_runtime_environment = surgescript_renv_create(
            object,
            stack,
            surgescript_object_heap(object),
            surgescript_renv_programpool(caller_runtime_environment),
            surgescript_renv_objectmanager(caller_runtime_environment)
        );

        /* push an environment and call the program */
        surgescript_stack_pushenv(stack);
        surgescript_program_run(prog, callee_runtime_environment);
        surgescript_stack_popenv(stack);

        /* callee_tmp[3] is the return value of the program */
        surgescript_var_copy(return_value, *(surgescript_renv_tmp(callee_runtime_environment) + 3));
        surgescript_renv_destroy(callee_runtime_environment);
    }
    else
        ssfatal("Runtime Error: function \"%s.%s\" (called in \"%s\") expects %d parameters, but received %d.", object_name, program_name, surgescript_object_name(surgescript_renv_owner(caller_runtime_environment)), expected_params, number_of_given_params);
}

/* writes data to buf, in hex/little-endian format (writes 9 bytes) */
char* hexdump32(unsigned data, char* buf)
{
    snprintf(buf, 9, "%08ux", data);
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
        ssarray_length(program->label) = 0;
        return true;
    }
    else
        return false;
}

/* returns -1 if f < 0, 0 if if == 0, 1 if f > 0 */
int fast_float_sign(float f)
{
    /* IEEE-754 floating-point representation */
    if((*((int*)&f) & 0x7FFFFFFF) == 0)
        return 0;
    else
        return 1 - ((*((int*)&f) & 0x80000000) >> 30);
}

/* returns -1 if f <= -0, 1 if f >= +0 */
int fast_float_sign1(float f)
{
    return 1 - ((*((int*)&f) & 0x80000000) >> 30);
}