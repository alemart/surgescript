/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/codegen.c
 * SurgeScript Compiler: Code Generator
 */

#include "codegen.h"
#include "symtable.h"
#include "../runtime/program.h"
#include "../runtime/program_pool.h"

#ifdef F
#undef F
#endif
#ifdef U
#undef U
#endif
#ifdef B
#undef B
#endif
#ifdef I
#undef I
#endif
#ifdef L
#undef L
#endif
#ifdef _____
#undef _____
#endif

/* utility macros */
#define SSASM(op, a, b)                 surgescript_program_add_line(context.program, (op), (a), (b))
#define LABEL(label)                    surgescript_program_add_label(context.program, (label))
#define TEXT(text)                      U(surgescript_program_add_text(context.program, (text)))
#define F(x)                            surgescript_program_operand_f(x)
#define U(x)                            surgescript_program_operand_u(x)
#define B(x)                            surgescript_program_operand_b(x)
#define I(x)                            surgescript_program_operand_i(x)
#define L(l)                            U(l)
#define _____                           U(0)
#define NEWLABEL()                      surgescript_program_new_label(context.program)


/* objects */
void emit_object_header(surgescript_nodecontext_t context, surgescript_program_label_t start, surgescript_program_label_t end)
{
    SSASM(SSOP_JMP, L(end), _____);
    LABEL(start);
}

void emit_object_footer(surgescript_nodecontext_t context, surgescript_program_label_t start, surgescript_program_label_t end)
{
    surgescript_program_label_t aloc = NEWLABEL();

    SSASM(SSOP_RET, _____, _____);
    LABEL(end);
        SSASM(SSOP_MOVF, U(2), F(surgescript_symtable_count(context.symbol_table)));
        SSASM(SSOP_JE, L(start), _____);
        LABEL(aloc);
            SSASM(SSOP_ALOC, _____, _____);
            SSASM(SSOP_DEC, U(2), _____);
            SSASM(SSOP_JNE, L(aloc), _____);
        SSASM(SSOP_JMP, L(start), _____);
}


/* constants */
void emit_null(surgescript_nodecontext_t context)
{
    SSASM(SSOP_MOVN, U(0), _____);
}

void emit_bool(surgescript_nodecontext_t context, bool value)
{
    SSASM(SSOP_MOVB, U(0), B(value));
}

void emit_number(surgescript_nodecontext_t context, float value)
{
    SSASM(SSOP_MOVF, U(0), F(value));
}

void emit_string(surgescript_nodecontext_t context, const char* value)
{
    SSASM(SSOP_MOVS, U(0), TEXT(value));
}