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
 * runtime/program.h
 * SurgeScript program
 */

#ifndef _SURGESCRIPT_RUNTIME_PROGRAM_H
#define _SURGESCRIPT_RUNTIME_PROGRAM_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "renv.h"
#include "program_operators.h"
#include "variable.h"
#include "object.h"

/*
 * A program is composed by:
 * 1. operations = a sequence of ( operator, ( operand_a, operand_b ) )
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
#define SURGESCRIPT_PROGRAM_UNDEFINED_LABEL (~0)

/* operators */
typedef enum surgescript_program_operator_t { // let t[a .. c] be the registers (temps) that belong to the object. then,
    #define DECLARE_CODES(x, y) x,
    SURGESCRIPT_PROGRAM_OPERATORS(DECLARE_CODES)
} surgescript_program_operator_t;

/* operands */
typedef union surgescript_program_operand_t {
    double f;
    uint32_t u;
    int32_t i;
    bool b;
    uint64_t u64;
    int64_t i64;
    void* p;
} surgescript_program_operand_t;

#define SSOP()   SSOPu(0)
#define SSOPu(x) surgescript_program_operand_u(x)
#define SSOPf(x) surgescript_program_operand_f(x)
#define SSOPi(x) surgescript_program_operand_i(x)
#define SSOPb(x) surgescript_program_operand_b(x)

static inline surgescript_program_operand_t surgescript_program_operand_u(unsigned u) { surgescript_program_operand_t o = { .u = u }; return o; }
static inline surgescript_program_operand_t surgescript_program_operand_f(double f) { surgescript_program_operand_t o = { .f = f }; return o; }
static inline surgescript_program_operand_t surgescript_program_operand_b(bool b) { surgescript_program_operand_t o = { .u64 = 0 }; o.b = b; return o; }
static inline surgescript_program_operand_t surgescript_program_operand_i(int i) { surgescript_program_operand_t o = { .i = i }; return o; }





/* public methods */


/* life-cycle: create, destroy & run */
surgescript_program_t* surgescript_program_create(int arity); /* create a new program */
surgescript_program_t* surgescript_program_create_native(int arity, surgescript_program_cfunction_t cfunction); /* a native C-program must return a newly-allocated surgescript_var_t*, or NULL */
surgescript_program_t* surgescript_program_destroy(surgescript_program_t* program); /* called by the program pool */
void surgescript_program_call(surgescript_program_t* program, surgescript_renv_t* runtime_environment, int num_params); /* low-level program call; you'll need to push the stack parameters by yourself */

/* write the program */
surgescript_program_label_t surgescript_program_new_label(surgescript_program_t* program); /* creates and returns a new label */
void surgescript_program_add_label(surgescript_program_t* program, surgescript_program_label_t label); /* adds a label to the current line of code in the program */
int surgescript_program_add_line(surgescript_program_t* program, surgescript_program_operator_t op, surgescript_program_operand_t a, surgescript_program_operand_t b); /* adds a line of code to the program */
int surgescript_program_chg_line(surgescript_program_t* program, int line, surgescript_program_operator_t op, surgescript_program_operand_t a, surgescript_program_operand_t b); /* changes an existing line of code of the program */

/* program data */
int surgescript_program_arity(const surgescript_program_t* program); /* what's the arity of this program? (i.e., how many parameters does it take) */
const char* surgescript_program_get_text(const surgescript_program_t* program, int index); /* reads a string literal (text[index]) from the program */
int surgescript_program_add_text(surgescript_program_t* program, const char* text); /* adds a read-only string to the program, returning its index */
int surgescript_program_find_text(const surgescript_program_t* program, const char* text); /* finds the first index such that text[index] == text, or -1 if not found */
int surgescript_program_text_count(const surgescript_program_t* program); /* how many string literals exist in the program? */
void surgescript_program_dump(surgescript_program_t* program, FILE* fp); /* dump the program to a file */
bool surgescript_program_is_native(const surgescript_program_t* program); /* is the program native (i.e., written in C)? */

#endif