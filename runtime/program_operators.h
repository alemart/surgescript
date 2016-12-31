/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/program_operators.h
 * SurgeScript program operators table
 */

#ifndef _SURGESCRIPT_RUNTIME_PROGRAM_OPERATORS_H
#define _SURGESCRIPT_RUNTIME_PROGRAM_OPERATORS_H

/*
    conventions:
        let 'a' and 'b' be the arguments of each operation
        and t[0 .. 3] be the temps (aka "registers")

    now,
        a, b are constants
        t[0], t[1] are temps holding different values throughout the program
        t[2] holds utilities, such as the results of comparisons
        t[3] holds the return value of the program
*/
#define SURGESCRIPT_PROGRAM_OPERATORS(F) \
    F( SSOP_NOP, "nop" )                                 /* no-operation */ \
    F( SSOP_OUT, "print" )                                 /* print t[a] */ \
    F( SSOP_IN, "read" )                                    /* read t[a] */ \
                                                                            \
    F( SSOP_MOVN, "movn" )                                /* t[a] = null */ \
    F( SSOP_MOVB, "movb" )                             /* t[a] = (bool)b */ \
    F( SSOP_MOVF, "movf" )                           /* t[a] = (number)b */ \
    F( SSOP_MOVS, "movs" )                             /* t[a] = text[b] */ \
    F( SSOP_MOVH, "movh" )                           /* t[a] = (object)b */ \
    F( SSOP_MOVC, "movc" )             /* t[a] = handle to caller object */ \
    F( SSOP_MOVT, "movt" )                                /* t[a] = t[b] */ \
                                                                            \
    F( SSOP_STORE, "store" )                           /* *(t[a]) = t[b] */ \
    F( SSOP_LOAD, "load" )                             /* t[a] = *(t[b]) */ \
                                                                            \
    F( SSOP_PUSH, "push" )                                  /* push t[a] */ \
    F( SSOP_POP, "pop" )                                     /* pop t[a] */ \
    F( SSOP_SPEEK, "speek" )                   /* t[a] = stack[base + b] */ \
    F( SSOP_SPOKE, "spoke" )                   /* stack[base + b] = t[a] */ \
                                                                            \
    F( SSOP_INC, "inc" )                                       /* t[a]++ */ \
    F( SSOP_DEC, "dec" )                                       /* t[a]-- */ \
    F( SSOP_ADD, "add" )                                 /* t[a] += t[b] */ \
    F( SSOP_SUB, "sub" )                                 /* t[a] -= t[b] */ \
    F( SSOP_MUL, "mul" )                                 /* t[a] *= t[b] */ \
    F( SSOP_DIV, "div" )                                 /* t[a] /= t[b] */ \
    F( SSOP_NEG, "neg" )                                 /* t[a] = -t[a] */ \
    F( SSOP_NOT, "not" )                              /* t[a] = not t[a] */ \
    F( SSOP_AND, "and" )                         /* t[a] = t[a] and t[b] */ \
    F( SSOP_OR, "or" )                            /* t[a] = t[a] or t[b] */ \
                                                                            \
    F( SSOP_JMP, "jmp" )                             /* jump to label[a] */ \
    F( SSOP_CMP, "cmp" )                    /* t[2] = compare t[a], t[b] */ \
    F( SSOP_JE, "je" )                  /* jump to label[a] if t[2] == 0 */ \
    F( SSOP_JNE, "jne" )                /* jump to label[a] if t[2] != 0 */ \
    F( SSOP_JG, "jg" )                   /* jump to label[a] if t[2] > 0 */ \
    F( SSOP_JGE, "jge" )                /* jump to label[a] if t[2] >= 0 */ \
    F( SSOP_JL, "jl" )                   /* jump to label[a] if t[2] < 0 */ \
    F( SSOP_JLE, "jle" )                /* jump to label[a] if t[2] <= 0 */ \
                                                                            \
    F( SSOP_CALL, "call" )              /* call program named text[t[a]] */ \
                                     /* of the object having handle t[b] */ \
                                    /* with n = t[2] parameters, storing */ \
                              /* in t[2] the return value of the program */ \
    F( SSOP_RET, "ret" )                 /* returns, halting the program */

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

    SSOP_TYPEOF = 0x60,             // t[a] = typeof(t[a])
    SSOP_STRLEN,                    // t[a] = strlen(t[a]) -- FIXME: move to userfun
    SSOP_STRMID,                    // t[a] = substr(t[a], t[b], t[c])
    SSOP_STRCAT,                    // t[a] = strcat(t[a], t[b])
    SSOP_STRAT,                     // t[a] = t[a][t[b]]
    SSOP_TOBOOL,                    // t[a] = bool(t[a])
    SSOP_TONUMBER,                  // t[a] = number(t[a])
    SSOP_TOSTRING,                  // t[a] = string(t[a])

    handle to root, parent and so on...
*/

#endif