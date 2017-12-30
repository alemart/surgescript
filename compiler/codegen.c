/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/codegen.c
 * SurgeScript Compiler: Code Generator
 */

#include <ctype.h>
#include <string.h>
#include "codegen.h"
#include "symtable.h"
#include "../runtime/program.h"
#include "../runtime/program_pool.h"
#include "../runtime/object_manager.h"
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
#define TYPE(name)                      I(surgescript_var_type2code(name))
#define LABEL(label)                    surgescript_program_add_label(context.program, (label))
#define NEWLABEL()                      surgescript_program_new_label(context.program)
#define F(x)                            surgescript_program_operand_f(x)
#define U(x)                            surgescript_program_operand_u(x)
#define B(x)                            surgescript_program_operand_b(x)
#define I(x)                            surgescript_program_operand_i(x)
#define T0                              U(0)
#define T1                              U(1)
#define T2                              U(2)
#define T3                              U(3)
#define BREAKPOINT(str)                 emit_breakpoint(context, (str))


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
        SSASM(SSOP_MOVF, T2, F(surgescript_symtable_local_count(context.symtable)));
        LABEL(aloc);
            SSASM(SSOP_JE, U(start));
            SSASM(SSOP_ALLOC);
            SSASM(SSOP_DEC, T2);
            SSASM(SSOP_JMP, U(aloc));
}

/* declarations */
void emit_vardecl(surgescript_nodecontext_t context, const char* identifier)
{
    if(!surgescript_symtable_has_symbol(context.symtable, identifier))
        surgescript_symtable_put_heap_symbol(context.symtable, identifier, (surgescript_heapptr_t)surgescript_symtable_local_count(context.symtable));
    surgescript_symtable_emit_write(context.symtable, identifier, context.program, 0);
}

void emit_exportvar(surgescript_nodecontext_t context, const char* identifier)
{
    SSASM(SSOP_SELF, T0);
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_MOVS, T0, TEXT(identifier));
    SSASM(SSOP_PUSH, T0);
    surgescript_symtable_push_addr(context.symtable, identifier, context.program);
    SSASM(SSOP_CALL, TEXT("__export"), U(2));
    SSASM(SSOP_POPN, U(3));
}

static void emit_accessor(const char* fun_name, void* ctx)
{
    /* run only if fun_name is an accessor (getSomething / setSomething) */
    if((strncmp(fun_name, "get", 3) == 0 || strncmp(fun_name, "set", 3) == 0) && fun_name[3] != '\0') {
        /* fun_name is an accessor */
        surgescript_nodecontext_t* context = (surgescript_nodecontext_t*)ctx;
        char* accessor = ssstrdup(fun_name + 3);
        accessor[0] = tolower(fun_name[3]);

        /* now that we have the accessor name, add it to the symbol table */
        if(!surgescript_symtable_has_symbol(context->symtable, accessor))
            surgescript_symtable_put_fun_symbol(context->symtable, accessor);

        /* done */
        ssfree(accessor);
    }
}

void emit_accessors(surgescript_nodecontext_t context, const char* object_name, surgescript_programpool_t* program_pool)
{
    /* look for all accessors in object_name
       and add them to the symbol table */
    surgescript_programpool_foreach_ex(program_pool, object_name, &context, emit_accessor);
}

/* expressions */
void emit_assignexpr(surgescript_nodecontext_t context, const char* assignop, const char* identifier, int line)
{
    /* put the identifier on the symbol table, and assign the symbol an address */
    if(!surgescript_symtable_has_parent(context.symtable))
        ssfatal("Compile Error: invalid attribution (\"%s %s ...\") in object \"%s\" (%s:%d) - only a single attribution is allowed.", identifier, assignop, context.object_name, context.source_file, line);
    else if(!surgescript_symtable_has_symbol(context.symtable, identifier))
        surgescript_symtable_put_stack_symbol(context.symtable, identifier, (surgescript_stackptr_t)(1 + surgescript_symtable_local_count(context.symtable) - surgescript_program_arity(context.program))); /* fact: local_count >= arity */

    /* perform the assignment operation */
    switch(*assignop) {
        case '=':
            surgescript_symtable_emit_write(context.symtable, identifier, context.program, 0);
            break;

        case '+': {
            surgescript_program_label_t cat = NEWLABEL();
            surgescript_program_label_t end = NEWLABEL();
            surgescript_symtable_emit_read(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_TC01, TYPE("string"));
            SSASM(SSOP_JE, U(cat));
            SSASM(SSOP_ADD, T0, T1);
            SSASM(SSOP_JMP, U(end));
            LABEL(cat);
            SSASM(SSOP_MOVO, T2, U(surgescript_objectmanager_system_object(NULL, "String")));
            SSASM(SSOP_PUSH, T2);
            SSASM(SSOP_PUSH, T1);
            SSASM(SSOP_PUSH, T0);
            SSASM(SSOP_CALL, TEXT("plus"), U(2));
            SSASM(SSOP_POPN, U(3));
            LABEL(end);
            surgescript_symtable_emit_write(context.symtable, identifier, context.program, 0);
            break;
        }

        case '-':
            surgescript_symtable_emit_read(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_SUB, T1, T0);
            SSASM(SSOP_XCHG, T0, T1);
            surgescript_symtable_emit_write(context.symtable, identifier, context.program, 0);
            break;

        case '*':
            surgescript_symtable_emit_read(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_MUL, T1, T0);
            SSASM(SSOP_XCHG, T0, T1);
            surgescript_symtable_emit_write(context.symtable, identifier, context.program, 0);
            break;

        case '/':
            surgescript_symtable_emit_read(context.symtable, identifier, context.program, 1);
            SSASM(SSOP_DIV, T1, T0);
            SSASM(SSOP_XCHG, T0, T1);
            surgescript_symtable_emit_write(context.symtable, identifier, context.program, 0);
            break;

        default:
            ssfatal("Compile Error: invalid assignment expression in \"%s\" (object \"%s\")", context.source_file, context.object_name);
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
    if(strcmp(equalityop, "==") == 0) {
        SSASM(SSOP_CMP, T1, T0);
        SSASM(SSOP_LNOT, T0, T2);
    }
    else if(strcmp(equalityop, "!=") == 0) {
        SSASM(SSOP_CMP, T1, T0);
        SSASM(SSOP_MOV, T0, T2);
    }
    else if(strcmp(equalityop, "===") == 0) {
        surgescript_program_label_t nope = NEWLABEL();
        SSASM(SSOP_TCMP, T1, T0);
        SSASM(SSOP_JNE, U(nope));
        SSASM(SSOP_CMP, T1, T0);
        SSASM(SSOP_LNOT, T0, T2);
        SSASM(SSOP_JMP, U(done));
        LABEL(nope);
        SSASM(SSOP_MOVB, T0, B(false));
    }
    else if(strcmp(equalityop, "!==") == 0) {
        surgescript_program_label_t yep = NEWLABEL();
        SSASM(SSOP_TCMP, T1, T0);
        SSASM(SSOP_JNE, U(yep));
        SSASM(SSOP_CMP, T1, T0);
        SSASM(SSOP_MOV, T0, T2);
        SSASM(SSOP_JMP, U(done));
        LABEL(yep);
        SSASM(SSOP_MOVB, T0, B(true));
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
            surgescript_program_label_t cat = NEWLABEL();
            surgescript_program_label_t end = NEWLABEL();
            SSASM(SSOP_TC01, TYPE("string")); /* either T0 or T1 is a string */
            SSASM(SSOP_JE, U(cat));
            SSASM(SSOP_ADD, T0, T1);
            SSASM(SSOP_JMP, U(end));
            LABEL(cat);
            SSASM(SSOP_MOVO, T2, U(surgescript_objectmanager_system_object(NULL, "String")));
            SSASM(SSOP_PUSH, T2);
            SSASM(SSOP_PUSH, T1);
            SSASM(SSOP_PUSH, T0);
            SSASM(SSOP_CALL, TEXT("plus"), U(2));
            SSASM(SSOP_POPN, U(3));
            LABEL(end);
            break;
        }

        case '-':
            SSASM(SSOP_SUB, T1, T0);
            SSASM(SSOP_XCHG, T1, T0);
            break;

        default:
            ssfatal("Compile Error: invalid additive expression in \"%s\" (object \"%s\")", context.source_file, context.object_name);
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

        default:
            ssfatal("Compile Error: invalid multiplicative expression in \"%s\" (object \"%s\")", context.source_file, context.object_name);
            break;
    }
}

void emit_unarysign(surgescript_nodecontext_t context, const char* op)
{
    if(*op == '-')
        SSASM(SSOP_NEG, T0, T0);
}

void emit_unaryincdec(surgescript_nodecontext_t context, const char* op, const char* identifier, int line)
{
    if(surgescript_symtable_has_symbol(context.symtable, identifier)) {
        surgescript_symtable_emit_read(context.symtable, identifier, context.program, 0);
        if(strcmp(op, "++") == 0)
            SSASM(SSOP_INC, T0);
        else if(strcmp(op, "--") == 0)
            SSASM(SSOP_DEC, T0);
        surgescript_symtable_emit_write(context.symtable, identifier, context.program, 0);
    }
    else
        ssfatal("Compile Error: undefined symbol \"%s\" in %s:%d.", identifier, context.source_file, line);
}

void emit_unarynot(surgescript_nodecontext_t context)
{
    SSASM(SSOP_LNOT, T0, T0);
}

void emit_unarytype(surgescript_nodecontext_t context)
{
    surgescript_program_label_t str = NEWLABEL();
    surgescript_program_label_t obj = NEWLABEL();
    surgescript_program_label_t bol = NEWLABEL();
    surgescript_program_label_t nul = NEWLABEL();
    surgescript_program_label_t end = NEWLABEL();

    SSASM(SSOP_TCHK, T0, TYPE("number"));
    SSASM(SSOP_JNE, U(str));
    SSASM(SSOP_MOVS, T0, TEXT("number"));
    SSASM(SSOP_JMP, U(end));

    LABEL(str);
    SSASM(SSOP_TCHK, T0, TYPE("string"));
    SSASM(SSOP_JNE, U(obj));
    SSASM(SSOP_MOVS, T0, TEXT("string"));
    SSASM(SSOP_JMP, U(end));

    LABEL(obj);
    SSASM(SSOP_TCHK, T0, TYPE("object"));
    SSASM(SSOP_JNE, U(bol));
    SSASM(SSOP_MOVS, T0, TEXT("object"));
    SSASM(SSOP_JMP, U(end));

    LABEL(bol);
    SSASM(SSOP_TCHK, T0, TYPE("boolean"));
    SSASM(SSOP_JNE, U(nul));
    SSASM(SSOP_MOVS, T0, TEXT("boolean"));
    SSASM(SSOP_JMP, U(end));

    LABEL(nul);
    SSASM(SSOP_MOVS, T0, TEXT("null"));

    LABEL(end);
}

void emit_postincdec(surgescript_nodecontext_t context, const char* op, const char* identifier, int line)
{
    if(surgescript_symtable_has_symbol(context.symtable, identifier)) {
        surgescript_symtable_emit_read(context.symtable, identifier, context.program, 0);
        SSASM(SSOP_MOV, T1, T0);
        if(strcmp(op, "++") == 0)
            SSASM(SSOP_INC, T1);
        else if(strcmp(op, "--") == 0)
            SSASM(SSOP_DEC, T1);
        surgescript_symtable_emit_write(context.symtable, identifier, context.program, 1);
    }
    else
        ssfatal("Compile Error: undefined symbol \"%s\" in %s:%d.", identifier, context.source_file, line);
}

void emit_pushparam(surgescript_nodecontext_t context)
{
    SSASM(SSOP_PUSH, T0);
}

void emit_popparams(surgescript_nodecontext_t context, int n)
{
    SSASM(SSOP_POPN, U(n));
}

void emit_funcall(surgescript_nodecontext_t context, const char* fun_name, int num_params)
{
    BREAKPOINT(fun_name);
    SSASM(SSOP_CALL, TEXT(fun_name), U(num_params));
}

void emit_dictptr(surgescript_nodecontext_t context)
{
    /* save the pointer */
    SSASM(SSOP_PUSH, T0);
}

void emit_dictkey(surgescript_nodecontext_t context)
{
    /* save the key <expr> */
    SSASM(SSOP_PUSH, T0);
}

void emit_dictget(surgescript_nodecontext_t context)
{
    SSASM(SSOP_CALL, TEXT("get"), U(1));
    SSASM(SSOP_POPN, U(2));
}

void emit_dictset(surgescript_nodecontext_t context, const char* assignop)
{
    switch(*assignop) {
        case '=':
            SSASM(SSOP_PUSH, T0); /* value <assignexpr> */
            SSASM(SSOP_CALL, TEXT("set"), U(2));
            SSASM(SSOP_POP, T0); /* return <assignexpr> */
            SSASM(SSOP_POPN, U(2));
            break;

        case '+':
        case '-':
        case '*':
        case '/':
            SSASM(SSOP_XCHG, T0, T3); /* t3 = value <assignexpr> */
            SSASM(SSOP_POP, T1); /* t1 = key <expr> */
            SSASM(SSOP_POP, T0); /* t0 = pointer */

            /* prepare the stack to call set() */
            SSASM(SSOP_NOP); /* we have a pop t0 above and a push t0 below */
            SSASM(SSOP_PUSH, T0);
            SSASM(SSOP_PUSH, T1);
            SSASM(SSOP_PUSH, T3); /* save the value <assignexpr> */

            /* call get() */
            SSASM(SSOP_PUSH, T0);
            SSASM(SSOP_PUSH, T1);
            SSASM(SSOP_CALL, TEXT("get"), U(1));
            SSASM(SSOP_POPN, U(2));

            /* compute terms */
            SSASM(SSOP_POP, T1); /* t1 = value <assignexpr> and t0 = dict.get(key <expr>) */
            if(*assignop == '+') {
                surgescript_program_label_t cat = NEWLABEL();
                surgescript_program_label_t end = NEWLABEL();
                SSASM(SSOP_TC01, TYPE("string"));
                SSASM(SSOP_JE, U(cat));
                SSASM(SSOP_ADD, T0, T1); /* t0 = dict.get(<expr>) + <assignexpr> */
                SSASM(SSOP_JMP, U(end));
                LABEL(cat);
                SSASM(SSOP_MOVO, T2, U(surgescript_objectmanager_system_object(NULL, "String")));
                SSASM(SSOP_PUSH, T2);
                SSASM(SSOP_PUSH, T0);
                SSASM(SSOP_PUSH, T1);
                SSASM(SSOP_CALL, TEXT("plus"), U(2)); /* t0 = dict.get(<expr>).plus(<assignexpr>) */
                SSASM(SSOP_POPN, U(3));
                LABEL(end);
            }
            else if(*assignop == '-')
                SSASM(SSOP_SUB, T0, T1); /* t0 = dict.get(<expr>) - <assignexpr> */
            else if(*assignop == '*')
                SSASM(SSOP_MUL, T0, T1); /* t0 = dict.get(<expr>) * <assignexpr> */
            else
                SSASM(SSOP_DIV, T0, T1); /* t0 = dict.get(<expr>) / <assignexpr> */

            /* call set() */
            SSASM(SSOP_PUSH, T0);
            SSASM(SSOP_CALL, TEXT("set"), U(2));
            SSASM(SSOP_POP, T0); /* return dict.get(<expr>) <assignop> <assignexpr> */
            SSASM(SSOP_POPN, U(2));
            break;

        default:
            ssfatal("Compile Error: invalid dictset expression in \"%s\" (object \"%s\")", context.source_file, context.object_name);
            break;
    }
}

void emit_dictincdec(surgescript_nodecontext_t context, const char* op)
{
    SSASM(SSOP_CALL, TEXT("get"), U(1));
    SSASM(*op == '+' ? SSOP_INC : SSOP_DEC, T0);
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_CALL, TEXT("set"), U(2));
    SSASM(SSOP_POP, T0);
    SSASM(*op != '+' ? SSOP_INC : SSOP_DEC, T0); /* return value before modification */
    SSASM(SSOP_POPN, U(2));
}

void emit_getter(surgescript_nodecontext_t context, const char* property_name)
{
    char* getter_name = surgescript_util_camelcaseprefix("get", property_name);

    if(isupper(property_name[0]))
        sslog("Warning: property \"%s\" should start with a lowercase character - in \"%s\" (object \"%s\")", property_name, context.source_file, context.object_name);

    SSASM(SSOP_PUSH, T0); /* object pointer */
    SSASM(SSOP_CALL, TEXT(getter_name), U(0));
    SSASM(SSOP_POPN, U(1));

    ssfree(getter_name);
}

void emit_setter1(surgescript_nodecontext_t context, const char* property_name)
{
    char* getter_name = surgescript_util_camelcaseprefix("get", property_name); /* get the value first */

    if(isupper(property_name[0]))
        sslog("Warning: property \"%s\" should start with a lowercase character - in \"%s\" (object \"%s\")", property_name, context.source_file, context.object_name);

    SSASM(SSOP_PUSH, T0); /* object pointer */
    SSASM(SSOP_CALL, TEXT(getter_name), U(0));
    SSASM(SSOP_PUSH, T0); /* push object.property_name */

    ssfree(getter_name);
}

void emit_setter2(surgescript_nodecontext_t context, const char* property_name, const char* assignop)
{
    char* setter_name = surgescript_util_camelcaseprefix("set", property_name);

    if(isupper(property_name[0]))
        sslog("Warning: property \"%s\" should start with a lowercase character - in \"%s\" (object \"%s\")", property_name, context.source_file, context.object_name);

    SSASM(SSOP_POP, T1);
    SSASM(SSOP_XCHG, T0, T1);

    /* now, t1 = <assignexpr> and t0 = object.property_name */
    switch(*assignop) {
        case '=': /* object.property_name = <assignexpr> */
            SSASM(SSOP_PUSH, T1); /* push <assignexpr> */
            SSASM(SSOP_CALL, TEXT(setter_name), U(1));
            SSASM(SSOP_POP, T0); /* return <assignexpr> */
            SSASM(SSOP_POPN, U(1)); /* pop object pointer */
            break;

        case '+': { /* object.property_name += <assignexpr> */
            surgescript_program_label_t cat = NEWLABEL();
            surgescript_program_label_t end = NEWLABEL();

            SSASM(SSOP_TC01, TYPE("string"));
            SSASM(SSOP_JE, U(cat));
            SSASM(SSOP_ADD, T0, T1); /* t0 = object.property_name + <assignexpr> */
            SSASM(SSOP_JMP, U(end));
            LABEL(cat);
            SSASM(SSOP_MOVO, T2, U(surgescript_objectmanager_system_object(NULL, "String")));
            SSASM(SSOP_PUSH, T2);
            SSASM(SSOP_PUSH, T0);
            SSASM(SSOP_PUSH, T1);
            SSASM(SSOP_CALL, TEXT("plus"), U(2)); /* t0 = object.property_name.plus(<assignexpr>) */
            SSASM(SSOP_POPN, U(3));
            LABEL(end);

            SSASM(SSOP_PUSH, T0);
            SSASM(SSOP_CALL, TEXT(setter_name), U(1));
            SSASM(SSOP_POP, T0);
            SSASM(SSOP_POPN, U(1));
            break;
        }

        case '-': /* object.property_name -= <assignexpr> */
            SSASM(SSOP_SUB, T0, T1); /* t0 now stores the result of the expression */
            SSASM(SSOP_PUSH, T0);
            SSASM(SSOP_CALL, TEXT(setter_name), U(1));
            SSASM(SSOP_POP, T0);
            SSASM(SSOP_POPN, U(1));
            break;

        case '*': /* object.property_name *= <assignexpr> */
            SSASM(SSOP_MUL, T0, T1);
            SSASM(SSOP_PUSH, T0);
            SSASM(SSOP_CALL, TEXT(setter_name), U(1));
            SSASM(SSOP_POP, T0);
            SSASM(SSOP_POPN, U(1));
            break;

        case '/': /* object.property_name /= <assignexpr> */
            SSASM(SSOP_DIV, T0, T1);
            SSASM(SSOP_PUSH, T0);
            SSASM(SSOP_CALL, TEXT(setter_name), U(1));
            SSASM(SSOP_POP, T0);
            SSASM(SSOP_POPN, U(1));
            break;

        default:
            ssfatal("Compile Error: invalid setter call in \"%s\" (object \"%s\")", context.source_file, context.object_name);
            break;
    }

    ssfree(setter_name);
}

void emit_setterincdec(surgescript_nodecontext_t context, const char* property_name, const char* op)
{
    char* getter_name = surgescript_util_camelcaseprefix("get", property_name);
    char* setter_name = surgescript_util_camelcaseprefix("set", property_name);

    if(isupper(property_name[0]))
        sslog("Warning: property \"%s\" should start with a lowercase character - in \"%s\" (object \"%s\")", property_name, context.source_file, context.object_name);

    SSASM(SSOP_PUSH, T0); /* object pointer */
    SSASM(SSOP_CALL, TEXT(getter_name), U(0)); /* t0 = old value */
    SSASM(*op == '+' ? SSOP_INC : SSOP_DEC, T0); /* update t0 */
    SSASM(SSOP_PUSH, T0); /* push new value */
    SSASM(SSOP_CALL, TEXT(setter_name), U(1)); /* call setter */
    SSASM(SSOP_POP, T0); /* restore pushed value */
    SSASM(*op != '+' ? SSOP_INC : SSOP_DEC, T0); /* return old value */
    SSASM(SSOP_POPN, U(1)); /* clear up the stack */

    ssfree(setter_name);
    ssfree(getter_name);
}

void emit_arrayexpr1(surgescript_nodecontext_t context)
{
    SSASM(SSOP_MOVO, T0, U(surgescript_objectmanager_system_object(NULL, "System")));
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_CALL, TEXT("get__Temp"), U(0)); /* t0 = System.__Temp */
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_MOVS, T0, TEXT("Array"));
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_CALL, TEXT("spawn"), U(1)); /* t0 = t0.spawn("Array") */
    SSASM(SSOP_POPN, U(3));
    SSASM(SSOP_PUSH, T0); /* save the handle to the dictionary */
}

void emit_arrayexpr2(surgescript_nodecontext_t context)
{
    SSASM(SSOP_POP, T0); /* (t0 = array) is the return value of the expression */
}

void emit_arrayelement(surgescript_nodecontext_t context)
{
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_CALL, TEXT("push"), U(1)); /* array.push(<assignexpr>) */
    SSASM(SSOP_POPN, U(1));
}

void emit_dictdecl1(surgescript_nodecontext_t context)
{
    SSASM(SSOP_MOVO, T0, U(surgescript_objectmanager_system_object(NULL, "System")));
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_CALL, TEXT("get__Temp"), U(0)); /* t0 = System.__Temp */
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_MOVS, T0, TEXT("Dictionary"));
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_CALL, TEXT("spawn"), U(1)); /* t0 = t0.spawn("Dictionary") */
    SSASM(SSOP_POPN, U(3));
    SSASM(SSOP_PUSH, T0); /* save the handle to the dictionary */
}

void emit_dictdecl2(surgescript_nodecontext_t context)
{
    SSASM(SSOP_POP, T0); /* retrieve the handle to the dictionary */
}

void emit_dictdeclkey(surgescript_nodecontext_t context)
{
    SSASM(SSOP_PUSH, T0);
}

void emit_dictdeclvalue(surgescript_nodecontext_t context)
{
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_CALL, TEXT("set"), U(2)); /* dict.set(key, value) */
    SSASM(SSOP_POPN, U(2));
}

/* statements */
void emit_if(surgescript_nodecontext_t context, surgescript_program_label_t nope)
{
    SSASM(SSOP_TEST, T0, T0);
    SSASM(SSOP_JE, U(nope));
}

void emit_else(surgescript_nodecontext_t context, surgescript_program_label_t nope, surgescript_program_label_t done)
{
    SSASM(SSOP_JMP, U(done));
    LABEL(nope);
}

void emit_endif(surgescript_nodecontext_t context, surgescript_program_label_t done)
{
    LABEL(done);
}

void emit_while1(surgescript_nodecontext_t context, surgescript_program_label_t begin)
{
    LABEL(begin);
}

void emit_whilecheck(surgescript_nodecontext_t context, surgescript_program_label_t end)
{
    SSASM(SSOP_TEST, T0, T0);
    SSASM(SSOP_JE, U(end));
}

void emit_while2(surgescript_nodecontext_t context, surgescript_program_label_t begin, surgescript_program_label_t end)
{
    SSASM(SSOP_JMP, U(begin));
    LABEL(end);
}

void emit_forin1(surgescript_nodecontext_t context, const char* it, surgescript_program_label_t begin, surgescript_program_label_t end)
{
    SSASM(SSOP_PUSH, T0); /* push the array <expr> */
    SSASM(SSOP_CALL, TEXT("getLength"), U(0));
    SSASM(SSOP_POPN, U(1));
    SSASM(SSOP_PUSH, T0); /* the length of the array is saved on the stack */

    if(!surgescript_symtable_has_symbol(context.symtable, it)) {
        /* reserve an address on the stack to the iterator */
        surgescript_symtable_put_stack_symbol(context.symtable, it, (surgescript_stackptr_t)(1 + surgescript_symtable_local_count(context.symtable) - surgescript_program_arity(context.program)));
    }
    SSASM(SSOP_XOR, T1, T1); /* initialize the loop counter */
    surgescript_symtable_emit_write(context.symtable, it, context.program, 1);

    LABEL(begin);

    SSASM(SSOP_POP, T0); /* t0 = array length */
    /*surgescript_symtable_emit_read(context.symtable, it, context.program, 1);*/ /* loop counter */
    SSASM(SSOP_PUSH, T0);
    SSASM(SSOP_CMP, T1, T0);
    SSASM(SSOP_JGE, U(end));
}

void emit_forin2(surgescript_nodecontext_t context, const char* it, surgescript_program_label_t begin, surgescript_program_label_t end)
{
    /* increment the loop counter */
    surgescript_symtable_emit_read(context.symtable, it, context.program, 1);
    SSASM(SSOP_INC, T1);
    surgescript_symtable_emit_write(context.symtable, it, context.program, 1);

    /* jump to the beginning */
    SSASM(SSOP_JMP, U(begin));
    LABEL(end);
    SSASM(SSOP_POP, T0); /* release the length */
}

void emit_for1(surgescript_nodecontext_t context, surgescript_program_label_t begin)
{
    LABEL(begin);
}

void emit_forcheck(surgescript_nodecontext_t context, surgescript_program_label_t begin, surgescript_program_label_t body, surgescript_program_label_t increment, surgescript_program_label_t end)
{
    SSASM(SSOP_TEST, T0, T0);
    SSASM(SSOP_JE, U(end));
    SSASM(SSOP_JMP, U(body));
    LABEL(increment);
}

void emit_for2(surgescript_nodecontext_t context, surgescript_program_label_t begin, surgescript_program_label_t body)
{
    SSASM(SSOP_JMP, U(begin));
    LABEL(body);
}

void emit_for3(surgescript_nodecontext_t context, surgescript_program_label_t increment, surgescript_program_label_t end)
{
    SSASM(SSOP_JMP, U(increment));
    LABEL(end);
}

void emit_break(surgescript_nodecontext_t context, int line)
{
    if(context.loop_end != SURGESCRIPT_PROGRAM_UNDEFINED_LABEL)
        SSASM(SSOP_JMP, U(context.loop_end));
    else
        ssfatal("Compile Error: invalid usage of the \"break\" command in %s:%d - break/continue may only be used inside loops.", context.source_file, line);
}

void emit_continue(surgescript_nodecontext_t context, int line)
{
    if(context.loop_begin != SURGESCRIPT_PROGRAM_UNDEFINED_LABEL)
        SSASM(SSOP_JMP, U(context.loop_begin));
    else
        ssfatal("Compile Error: invalid usage of the \"continue\" command in %s:%d - break/continue may only be used inside loops.", context.source_file, line);
}

/* functions */
int emit_function_header(surgescript_nodecontext_t context)
{
    return SSASM(SSOP_NOP); /* to be filled later */
}

void emit_function_footer(surgescript_nodecontext_t context, int num_locals, int fun_header)
{
    if(num_locals > 0)
        surgescript_program_chg_line(context.program, fun_header, SSOP_PUSHN, U(num_locals), U(0));
    SSASM(SSOP_MOVN, T0);
    /*SSASM(SSOP_POPN, U(num_locals));*/ /* not needed, since popenv() clears the stack frame for us */
    SSASM(SSOP_RET);
}

void emit_function_argument(surgescript_nodecontext_t context, const char* identifier, int line, int idx, int argc)
{
    /* idx is the index of the argument; it is such that 0 <= idx < argc (left-to-right) */
    if(!surgescript_symtable_has_local_symbol(context.symtable, identifier))
        surgescript_symtable_put_stack_symbol(context.symtable, identifier, (surgescript_stackptr_t)(idx-argc));
    else
        ssfatal("Duplicate function parameter \"%s\" in %s:%d.", identifier, context.source_file, line);
}

void emit_ret(surgescript_nodecontext_t context)
{
    SSASM(SSOP_RET);
}

/* constants & variables */
void emit_this(surgescript_nodecontext_t context)
{
    SSASM(SSOP_SELF, T0);
}

void emit_state(surgescript_nodecontext_t context)
{
    SSASM(SSOP_STATE, T0);
}

void emit_identifier(surgescript_nodecontext_t context, const char* identifier, int line)
{
    if(surgescript_symtable_has_symbol(context.symtable, identifier))
        surgescript_symtable_emit_read(context.symtable, identifier, context.program, 0);
    else
        ssfatal("Compile Error: undefined symbol \"%s\" in %s:%d.", identifier, context.source_file, line);
}

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

void emit_object(surgescript_nodecontext_t context, unsigned handle)
{
    SSASM(SSOP_MOVO, T0, U(handle));
}

void emit_zero(surgescript_nodecontext_t context)
{
    SSASM(SSOP_XOR, T0, T0);
}

/* misc */
void emit_setstate(surgescript_nodecontext_t context)
{
    SSASM(SSOP_STATE, T0, I(-1)); /* return value is in t[0] */
}

void emit_nop(surgescript_nodecontext_t context)
{
    SSASM(SSOP_NOP);
}

void emit_push(surgescript_nodecontext_t context)
{
    SSASM(SSOP_PUSH, T0);
}

void emit_pop(surgescript_nodecontext_t context)
{
    SSASM(SSOP_POP, T0);
}

void emit_breakpoint(surgescript_nodecontext_t context, const char* text)
{
    SSASM(SSOP_NOP, I(-1), TEXT(text));
}