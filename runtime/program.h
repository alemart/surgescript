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

#include "renv.h"

/*
 * A program is composed by:
 * 1. operations = a sequence of ( operator, operand_a, operand_b, operand_c )
 * 2. labels = indexes of operations
 * 3. string literals
 */

/* programs */
typedef struct surgescript_program_t surgescript_program_t;
struct surgescript_program_t;

/* labels */
typedef unsigned surgescript_program_label_t;

/* operators */
typedef enum surgescript_program_operator_t surgescript_program_operator_t;
enum surgescript_program_operator_t { // let t[a .. c] be the registers (temps) that belong to the object. then,
    SSOP_NOP = 0x0,                 // nop
    SSOP_OUT,                       // print t[a] (used for debug purposes)
    SSOP_IN,                        // read t[a] (used for debug purposes)

    SSOP_ASSIGN_NULL = 0x10,        // t[a] = null
    SSOP_ASSIGN_BOOL,               // t[a] = b
    SSOP_ASSIGN_NUMBER,             // t[a] = b
    SSOP_ASSIGN_STRING,             // t[a] = text[b]
    SSOP_ASSIGN_OBJECTHANDLE,       // t[a] = b

    SSOP_STORE = 0x20,              // *(t[a]) = t[b]
    SSOP_LOAD,                      // t[a] = *(t[b])
    SSOP_MALLOC,                    // t[a] = malloc()
    SSOP_FREE,                      // free(t[a])

    SSOP_PUSH = 0x30,               // push t[a]
    SSOP_POP,                       // pop t[a]
    SSOP_PEEK,                      // t[a] = stack[base + b]
    SSOP_POKE,                      // stack[base + a] = t[b]

    SSOP_MOV = 0x40,                // t[a] = t[b]
    SSOP_ZERO,                      // t[a] = 0
    SSOP_INC,                       // t[a] = t[a] + 1
    SSOP_DEC,                       // t[a] = t[a] - 1
    SSOP_ADD,                       // t[a] = t[a] + t[b]
    SSOP_SUB,                       // t[a] = t[a] - t[b]
    SSOP_MUL,                       // t[a] = t[a] * t[b]
    SSOP_DIV,                       // t[a] = t[a] / t[b]
    SSOP_MOD,                       // t[a] = t[a] mod t[b]
    SSOP_POW,                       // t[a] = t[a] ^ t[b]
    SSOP_NOT,                       // t[a] = not t[a]
    SSOP_AND,                       // t[a] = t[a] and t[b]
    SSOP_OR,                        // t[a] = t[a] or t[b]
    SSOP_NEG,                       // t[a] = -t[a]

/* -- move to userfun
    SSOP_EXP = 0x50,                // t[a] = exp(t[a])
    SSOP_LOG,                       // t[a] = ln(t[a])
    SSOP_SIN,                       // t[a] = sin(t[a])
    SSOP_COS,                       // t[a] = cos(t[a])
    SSOP_TAN,                       // t[a] = tan(t[a])
    SSOP_ASIN,                      // t[a] = asin(t[a])
    SSOP_ACOS,                      // t[a] = acos(t[a])
    SSOP_ATAN,                      // t[a] = atan(t[a])
    SSOP_ATAN2,                     // t[a] = atan2(t[a], t[b])
    SSOP_CEIL,                      // t[a] = ceil(t[a])
    SSOP_FLOOR,                     // t[a] = floor(t[a]])
*/

    SSOP_TYPEOF = 0x60,             // t[a] = typeof(t[a])
    SSOP_STRLEN,                    // t[a] = strlen(t[a]) -- FIXME: move to userfun
    SSOP_STRMID,                    // t[a] = substr(t[a], t[b], t[c])
    SSOP_STRCAT,                    // t[a] = strcat(t[a], t[b])
    SSOP_STRAT,                     // t[a] = t[a][t[b]]
    SSOP_TOBOOL,                    // t[a] = bool(t[a])
    SSOP_TONUMBER,                  // t[a] = number(t[a])
    SSOP_TOSTRING,                  // t[a] = string(t[a])

    SSOP_JMP = 0x70,                // ip = label[a]
    SSOP_JMP_IF_TRUE,               // ip = label[a] if t[b]
    SSOP_JMP_IF_EQUAL,              // ip = label[a] if t[b] == t[c]
    SSOP_JMP_IF_NOTEQUAL,           // ip = label[a] if t[b] != t[c]
    SSOP_JMP_IF_LOWER,              // ip = label[a] if t[b] < t[c]
    SSOP_JMP_IF_GREATER,            // ip = label[a] if t[b] > t[c]
    SSOP_JMP_IF_LOWEROREQUAL,       // ip = label[a] if t[b] <= t[c]
    SSOP_JMP_IF_GREATEROREQUAL,     // ip = label[a] if t[b] >= t[c]
    SSOP_JMP_IF_ZERO,               // ip = label[a] if t[b] == 0
    SSOP_JMP_IF_NOTZERO,            // ip = label[a] if t[b] != 0

    SSOP_CALL = 0x80,               // t[a] = call program named text[t[a]] of the object having handle t[b] with n = c parameters
    SSOP_RET,                       // returns, halting the program

    SSOP_ROOT_HANDLE,               // t[a] = handle to the root object in the object manager
    SSOP_SELF_HANDLE,               // t[a] = handle to this object (self) in the object manager
    SSOP_STATE,                     // t[a] = name of the current state of this object
    // get handles to: self, root, parent
    SSOP_DESTROY,                   // destroys the object (i.e., schedules a destroy)
};

/* C-functions can also be encapsulated in programs */
typedef struct surgescript_var_t* (*surgescript_program_cfunction_t)(struct surgescript_object_t*, const struct surgescript_var_t**, int);

/* operands */
typedef union surgescript_program_operand_t surgescript_program_operand_t;
union surgescript_program_operand_t {
    unsigned u;
    float f;
};
#define SSNOP surgescript_program_operand_u(0)
#define SSOP(x) surgescript_program_operand_u(x)
#define SSOPf(x) surgescript_program_operand_f(x)




/* public methods */


/* life-cycle: create, destroy & run */
surgescript_program_t* surgescript_program_create(int arity, int num_local_vars);
surgescript_program_t* surgescript_program_cfunction_create(int arity, surgescript_program_cfunction_t cfunction);
surgescript_program_t* surgescript_program_destroy(surgescript_program_t* program);
void surgescript_program_run(surgescript_program_t* program, struct surgescript_renv_t* runtime_environment);

/* write the program */
surgescript_program_label_t surgescript_program_new_label(surgescript_program_t* program); /* creates and returns a new label */
void surgescript_program_add_label(surgescript_program_t* program, surgescript_program_label_t label); /* adds a label to the current line of code in the program */
void surgescript_program_add_line(surgescript_program_t* program, surgescript_program_operator_t op, surgescript_program_operand_t a, surgescript_program_operand_t b, surgescript_program_operand_t c); /* adds a line of code to the program */
int surgescript_program_add_text(surgescript_program_t* program, const char* text); /* adds a read-only string to the program, returning its index */

/* program data */
int surgescript_program_arity(const surgescript_program_t* program); /* what's the arity of this program? (i.e., how many parameters does it take) */
const char* surgescript_program_get_text(const surgescript_program_t* program, int index); /* reads a string literal (text[index]) from the program */
int surgescript_program_find_text(const surgescript_program_t* program, const char* text); /* finds the first index such that text[index] == text, or -1 if not found */
int surgescript_program_text_count(const surgescript_program_t* program); /* how many string literals exist in the program? */

/* operand helpers */
surgescript_program_operand_t surgescript_program_operand_u(unsigned u);
surgescript_program_operand_t surgescript_program_operand_f(float f);

#endif
