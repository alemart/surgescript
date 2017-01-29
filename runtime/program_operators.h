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
    F( SSOP_MOVC, "movc" )    /* t[a] = handle to caller object ("this") */ \
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

#endif