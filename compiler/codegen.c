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
#include "../util/util.h"

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
#define _SSASMX(_0, _1, _2, X, ...)     X
#define _SSASM2(op, a, b)               surgescript_program_add_line(context.program, (op), (a), (b))
#define _SSASM1(op, a)                  surgescript_program_add_line(context.program, (op), (a), U(0))
#define _SSASM0(op)                     surgescript_program_add_line(context.program, (op), U(0), U(0))

/* utility macros */
#define SSASM(...)                      _SSASMX(__VA_ARGS__, _SSASM2, _SSASM1, _SSASM0)(__VA_ARGS__)
#define TEXT(text)                      U(surgescript_program_add_text(context.program, (text)))
#define LABEL(label)                    surgescript_program_add_label(context.program, (label))
#define NEWLABEL()                      surgescript_program_new_label(context.program)
#define F(x)                            surgescript_program_operand_f(x)
#define U(x)                            surgescript_program_operand_u(x)
#define B(x)                            surgescript_program_operand_b(x)
#define I(x)                            surgescript_program_operand_i(x)
#define T0                              U(0)
#define T1                              U(1)
#define T2                              U(2)


/* objects */
void emit_object_header(surgescript_nodecontext_t context, surgescript_program_label_t start, surgescript_program_label_t end)
{
    SSASM(SSOP_JMP, U(end));
    LABEL(start);
}

void emit_object_footer(surgescript_nodecontext_t context, surgescript_program_label_t start, surgescript_program_label_t end)
{
    surgescript_program_label_t aloc = NEWLABEL();

    SSASM(SSOP_RET);
    LABEL(end);
        SSASM(SSOP_MOVF, T2, F(surgescript_symtable_count(context.symtable)));
        LABEL(aloc);
            SSASM(SSOP_JE, U(start));
            SSASM(SSOP_ALOC);
            SSASM(SSOP_DEC, T2);
            SSASM(SSOP_JMP, U(aloc));
}

/* declarations */
void emit_vardecl(surgescript_nodecontext_t context, const char* identifier)
{
    surgescript_symtable_put_heap_symbol(context.symtable, identifier, (surgescript_heapptr_t)surgescript_symtable_count(context.symtable));
    surgescript_symtable_emit_write(context.symtable, identifier, context.program, 0);
    SSASM(SSOP_OUT, T0);
}

/* expressions */
void emit_assignexpr(surgescript_nodecontext_t context, const char* identifier, const char* assignop)
{
    /* put the identifier on the symbol table, and assign the symbol an address */
    if(!surgescript_symtable_has_parent(context.symtable))
        ssfatal("Invalid declaration (\"%s %s ...\") for object \"%s\" in %s: only a single attribution is allowed.", identifier, assignop, context.object_name, context.source_file);
    else
        surgescript_symtable_put_stack_symbol(context.symtable, identifier, (surgescript_stackptr_t)(-2 - surgescript_symtable_count(context.symtable)));

    /* perform the assignment operation */
    switch(*assignop) {
        case '=':
            surgescript_symtable_emit_write(context.symtable, identifier, context.program, 0);
            break;

        case '+': {
            surgescript_program_label_t dif = NEWLABEL();
            surgescript_program_label_t cat = NEWLABEL();
            surgescript_program_label_t add = NEWLABEL();
            surgescript_program_label_t end = NEWLABEL();

            surgescript_symtable_emit_read(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_TCHK, T1, T0);
            SSASM(SSOP_JNE, U(dif));
            SSASM(SSOP_TCHKN, T1);
            SSASM(SSOP_JE, U(add));
            LABEL(dif);
            SSASM(SSOP_TCHKS, T1);
            SSASM(SSOP_JE, U(cat));
            SSASM(SSOP_TCHKO, T1);
            SSASM(SSOP_JNE, U(add));
            LABEL(cat);
            SSASM(SSOP_CAT, T1, T0);
            SSASM(SSOP_JMP, U(end));
            LABEL(add);
            SSASM(SSOP_ADD, T1, T0);
            LABEL(end);
            surgescript_symtable_emit_write(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_XCHG, T0, T1);

            break;
        }

        case '-':
            surgescript_symtable_emit_read(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_SUB, T1, T0);
            surgescript_symtable_emit_write(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_XCHG, T0, T1);
            break;

        case '*':
            surgescript_symtable_emit_read(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_MUL, T1, T0);
            surgescript_symtable_emit_write(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_XCHG, T0, T1);
            break;

        case '/':
            surgescript_symtable_emit_read(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_DIV, T1, T0);
            surgescript_symtable_emit_write(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_XCHG, T0, T1);
            break;
    }
}

void emit_conditionalexpr1(surgescript_nodecontext_t context, surgescript_program_label_t nope, surgescript_program_label_t done)
{
    SSASM(SSOP_TEST, T0, T0);
    SSASM(SSOP_JE, U(nope));
}

void emit_conditionalexpr2(surgescript_nodecontext_t context, surgescript_program_label_t nope, surgescript_program_label_t done)
{
    SSASM(SSOP_JMP, U(done));
    LABEL(nope);
}

void emit_conditionalexpr3(surgescript_nodecontext_t context, surgescript_program_label_t nope, surgescript_program_label_t done)
{
    LABEL(done);
}

void emit_logicalorexpr1(surgescript_nodecontext_t context, surgescript_program_label_t done)
{
    /* short-circuit evaluation */
    SSASM(SSOP_TEST, T0, T0);
    SSASM(SSOP_JNE, U(done));
}

void emit_logicalorexpr2(surgescript_nodecontext_t context, surgescript_program_label_t done)
{
    LABEL(done);
}

void emit_logicalandexpr1(surgescript_nodecontext_t context, surgescript_program_label_t done)
{
    /* short-circuit evaluation */
    SSASM(SSOP_TEST, T0, T0);
    SSASM(SSOP_JE, U(done));
}

void emit_logicalandexpr2(surgescript_nodecontext_t context, surgescript_program_label_t done)
{
    LABEL(done);
}

void emit_equalityexpr1(surgescript_nodecontext_t context)
{
    SSASM(SSOP_PUSH, T0);
}

void emit_equalityexpr2(surgescript_nodecontext_t context, const char* equalityop)
{
    surgescript_program_label_t done = NEWLABEL();

    SSASM(SSOP_POP, T1);
    SSASM(SSOP_CMP, T1, T0);
    SSASM(SSOP_MOVB, T0, B(true));
    if(strcmp(equalityop, "==") == 0) {
        SSASM(SSOP_JE, U(done));
        SSASM(SSOP_MOVB, T0, B(false));
    }
    else if(strcmp(equalityop, "!=") == 0) {
        SSASM(SSOP_JNE, U(done));
        SSASM(SSOP_MOVB, T0, B(false));
    }
    LABEL(done);
}

void emit_relationalexpr1(surgescript_nodecontext_t context)
{
    SSASM(SSOP_PUSH, T0);
}

void emit_relationalexpr2(surgescript_nodecontext_t context, const char* relationalop)
{
    surgescript_program_label_t done = NEWLABEL();

    SSASM(SSOP_POP, T1);
    SSASM(SSOP_CMP, T1, T0);
    SSASM(SSOP_MOVB, T0, B(true));
    if(strcmp(relationalop, ">=") == 0) {
        SSASM(SSOP_JGE, U(done));
        SSASM(SSOP_MOVB, T0, B(false));
    }
    else if(strcmp(relationalop, ">") == 0) {
        SSASM(SSOP_JG, U(done));
        SSASM(SSOP_MOVB, T0, B(false));
    }
    else if(strcmp(relationalop, "<") == 0) {
        SSASM(SSOP_JL, U(done));
        SSASM(SSOP_MOVB, T0, B(false));
    }
    else if(strcmp(relationalop, "<=") == 0) {
        SSASM(SSOP_JLE, U(done));
        SSASM(SSOP_MOVB, T0, B(false));
    }
    LABEL(done);
}

void emit_additiveexpr1(surgescript_nodecontext_t context)
{
    SSASM(SSOP_PUSH, T0);
}

void emit_additiveexpr2(surgescript_nodecontext_t context, const char* additiveop)
{
    SSASM(SSOP_POP, T1);
    switch(*additiveop) {
        case '+': {
            surgescript_program_label_t dif = NEWLABEL();
            surgescript_program_label_t cat = NEWLABEL();
            surgescript_program_label_t add = NEWLABEL();
            surgescript_program_label_t end = NEWLABEL();

            SSASM(SSOP_TCHK, T1, T0);
            SSASM(SSOP_JNE, U(dif));
            SSASM(SSOP_TCHKN, T1);
            SSASM(SSOP_JE, U(add));
            LABEL(dif);
            SSASM(SSOP_TCHKS, T1);
            SSASM(SSOP_JE, U(cat));
            SSASM(SSOP_TCHKO, T1);
            SSASM(SSOP_JNE, U(add));
            LABEL(cat);
            SSASM(SSOP_CAT, T1, T0);
            SSASM(SSOP_XCHG, T0, T1);
            SSASM(SSOP_JMP, U(end));
            LABEL(add);
            SSASM(SSOP_ADD, T0, T1);
            LABEL(end);

            break;
        }

        case '-':
            SSASM(SSOP_SUB, T1, T0);
            SSASM(SSOP_XCHG, T1, T0);
            break;
    }
}

void emit_multiplicativeexpr1(surgescript_nodecontext_t context)
{
    SSASM(SSOP_PUSH, T0);
}

void emit_multiplicativeexpr2(surgescript_nodecontext_t context, const char* multiplicativeop)
{
    SSASM(SSOP_POP, T1);
    switch(*multiplicativeop) {
        case '*':
            SSASM(SSOP_MUL, T0, T1);
            break;

        case '/':
            SSASM(SSOP_DIV, T1, T0);
            SSASM(SSOP_XCHG, T1, T0);
            break;
    }
}

/* constants */
void emit_null(surgescript_nodecontext_t context)
{
    SSASM(SSOP_MOVN, T0);
}

void emit_bool(surgescript_nodecontext_t context, bool value)
{
    SSASM(SSOP_MOVB, T0, B(value));
}

void emit_number(surgescript_nodecontext_t context, float value)
{
    SSASM(SSOP_MOVF, T0, F(value));
}

void emit_string(surgescript_nodecontext_t context, const char* value)
{
    SSASM(SSOP_MOVS, T0, TEXT(value));
}