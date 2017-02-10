/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/program.h
 * SurgeScript program
 */

#ifndef _SURGESCRIPT_RUNTIME_PROGRAM_H
#define _SURGESCRIPT_RUNTIME_PROGRAM_H

#include <stdbool.h>
#include "renv.h"
#include "program_operators.h"
#include "variable.h"
#include "object.h"

/*
 * A program is composed by:
 * 1. operations = a sequence of ( operator, operand_a, operand_b, operand_c )
 * 2. labels = indexes of operations
 * 3. string literals
 */

/* programs */
typedef struct surgescript_program_t surgescript_program_t;
struct surgescript_program_t;

/* C-functions can also be encapsulated in programs */
typedef surgescript_var_t* (*surgescript_program_cfunction_t)(surgescript_object_t*, const surgescript_var_t**, int);

/* labels */
typedef unsigned surgescript_program_label_t;

/* operators */
typedef enum surgescript_program_operator_t { // let t[a .. c] be the registers (temps) that belong to the object. then,
    #define DECLARE_CODES(x, y) x,
    SURGESCRIPT_PROGRAM_OPERATORS(DECLARE_CODES)
} surgescript_program_operator_t;

/* operands */
typedef union surgescript_program_operand_t {
    unsigned u;
    float f;
    bool b;
    int i;
} surgescript_program_operand_t;

#define SSOP()   SSOPu(0)
#define SSOPu(x) surgescript_program_operand_u(x)
#define SSOPf(x) surgescript_program_operand_f(x)
#define SSOPi(x) surgescript_program_operand_i(x)
#define SSOPb(x) surgescript_program_operand_b(x)

inline surgescript_program_operand_t surgescript_program_operand_u(unsigned u) { surgescript_program_operand_t o = { .u = u }; return o; }
inline surgescript_program_operand_t surgescript_program_operand_f(float f) { surgescript_program_operand_t o = { .f = f }; return o; }
inline surgescript_program_operand_t surgescript_program_operand_b(bool b) { surgescript_program_operand_t o = { .b = b }; return o; }
inline surgescript_program_operand_t surgescript_program_operand_i(int i) { surgescript_program_operand_t o = { .i = i }; return o; }





/* public methods */


/* life-cycle: create, destroy & run */
surgescript_program_t* surgescript_program_create(int arity, int num_local_vars);
surgescript_program_t* surgescript_cprogram_create(int arity, surgescript_program_cfunction_t cfunction); /* a C-program must return a newly-allocated surgescript_var_t*, or NULL */
surgescript_program_t* surgescript_program_destroy(surgescript_program_t* program); /* called by the program pool */
void surgescript_program_run(surgescript_program_t* program, struct surgescript_renv_t* runtime_environment);

/* write the program */
surgescript_program_label_t surgescript_program_new_label(surgescript_program_t* program); /* creates and returns a new label */
void surgescript_program_add_label(surgescript_program_t* program, surgescript_program_label_t label); /* adds a label to the current line of code in the program */
void surgescript_program_add_line(surgescript_program_t* program, surgescript_program_operator_t op, surgescript_program_operand_t a, surgescript_program_operand_t b); /* adds a line of code to the program */
int surgescript_program_add_text(surgescript_program_t* program, const char* text); /* adds a read-only string to the program, returning its index */

/* program data */
int surgescript_program_arity(const surgescript_program_t* program); /* what's the arity of this program? (i.e., how many parameters does it take) */
const char* surgescript_program_get_text(const surgescript_program_t* program, int index); /* reads a string literal (text[index]) from the program */
int surgescript_program_find_text(const surgescript_program_t* program, const char* text); /* finds the first index such that text[index] == text, or -1 if not found */
int surgescript_program_text_count(const surgescript_program_t* program); /* how many string literals exist in the program? */

#endif