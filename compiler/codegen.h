/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/codegen.h
 * SurgeScript Compiler: Code Generator
 */

#ifndef _SURGESCRIPT_COMPILER_CODEGEN_H
#define _SURGESCRIPT_COMPILER_CODEGEN_H

#include <stdbool.h>
#include "nodecontext.h"
#include "../runtime/program.h"

/* objects */
void emit_object_header(surgescript_nodecontext_t context, surgescript_program_label_t start, surgescript_program_label_t end);
void emit_object_footer(surgescript_nodecontext_t context, surgescript_program_label_t start, surgescript_program_label_t end);

/* declarations */
void emit_vardecl(surgescript_nodecontext_t context, const char* identifier);
void emit_exportvar(surgescript_nodecontext_t context, const char* identifier);

/* expressions */
void emit_assignexpr(surgescript_nodecontext_t context, const char* assignop, const char* identifier, int line);
void emit_conditionalexpr1(surgescript_nodecontext_t context, surgescript_program_label_t nope, surgescript_program_label_t done);
void emit_conditionalexpr2(surgescript_nodecontext_t context, surgescript_program_label_t nope, surgescript_program_label_t done);
void emit_conditionalexpr3(surgescript_nodecontext_t context, surgescript_program_label_t nope, surgescript_program_label_t done);
void emit_logicalorexpr1(surgescript_nodecontext_t context, surgescript_program_label_t done);
void emit_logicalorexpr2(surgescript_nodecontext_t context, surgescript_program_label_t done);
void emit_logicalandexpr1(surgescript_nodecontext_t context, surgescript_program_label_t done);
void emit_logicalandexpr2(surgescript_nodecontext_t context, surgescript_program_label_t done);
void emit_equalityexpr1(surgescript_nodecontext_t context);
void emit_equalityexpr2(surgescript_nodecontext_t context, const char* equalityop);
void emit_relationalexpr1(surgescript_nodecontext_t context);
void emit_relationalexpr2(surgescript_nodecontext_t context, const char* relationalop);
void emit_additiveexpr1(surgescript_nodecontext_t context);
void emit_additiveexpr2(surgescript_nodecontext_t context, const char* additiveop);
void emit_multiplicativeexpr1(surgescript_nodecontext_t context);
void emit_multiplicativeexpr2(surgescript_nodecontext_t context, const char* multiplicativeop);
void emit_unarysign(surgescript_nodecontext_t context, const char* op);
void emit_unaryincdec(surgescript_nodecontext_t context, const char* op, const char* identifier, int line);
void emit_unarynot(surgescript_nodecontext_t context);
void emit_unarytype(surgescript_nodecontext_t context);
void emit_postincdec(surgescript_nodecontext_t context, const char* op, const char* identifier, int line);
void emit_pushparam(surgescript_nodecontext_t context);
void emit_popparams(surgescript_nodecontext_t context, int n);
void emit_funcall(surgescript_nodecontext_t context, const char* fun_name, int num_params);
void emit_dictptr(surgescript_nodecontext_t context);
void emit_dictkey(surgescript_nodecontext_t context);
void emit_dictget(surgescript_nodecontext_t context);
void emit_dictset(surgescript_nodecontext_t context, const char* assignop);
void emit_dictincdec(surgescript_nodecontext_t context, const char* op);
void emit_exportedvar(surgescript_nodecontext_t context, const char* identifier);
void emit_arrayexpr1(surgescript_nodecontext_t context);
void emit_arrayexpr2(surgescript_nodecontext_t context);
void emit_arrayelement(surgescript_nodecontext_t context);

/* statements */
void emit_if(surgescript_nodecontext_t context, surgescript_program_label_t nope);
void emit_else(surgescript_nodecontext_t context, surgescript_program_label_t nope, surgescript_program_label_t done);
void emit_endif(surgescript_nodecontext_t context, surgescript_program_label_t done);
void emit_while1(surgescript_nodecontext_t context, surgescript_program_label_t begin);
void emit_whilecheck(surgescript_nodecontext_t context, surgescript_program_label_t end);
void emit_while2(surgescript_nodecontext_t context, surgescript_program_label_t begin, surgescript_program_label_t end);
void emit_forin1(surgescript_nodecontext_t context, const char* it, surgescript_program_label_t begin, surgescript_program_label_t end);
void emit_forin2(surgescript_nodecontext_t context, const char* it, surgescript_program_label_t begin, surgescript_program_label_t end);
void emit_for1(surgescript_nodecontext_t context, surgescript_program_label_t begin);
void emit_forcheck(surgescript_nodecontext_t context, surgescript_program_label_t begin, surgescript_program_label_t body, surgescript_program_label_t increment, surgescript_program_label_t end);
void emit_for2(surgescript_nodecontext_t context, surgescript_program_label_t begin, surgescript_program_label_t body);
void emit_for3(surgescript_nodecontext_t context, surgescript_program_label_t increment, surgescript_program_label_t end);
void emit_break(surgescript_nodecontext_t context, int line);
void emit_continue(surgescript_nodecontext_t context, int line);

/* functions */
int emit_function_header(surgescript_nodecontext_t context);
void emit_function_footer(surgescript_nodecontext_t context, int num_locals, int fun_header);
void emit_function_argument(surgescript_nodecontext_t context, const char* identifier, int line, int idx, int argc);
void emit_ret(surgescript_nodecontext_t context);

/* constants & variables */
void emit_this(surgescript_nodecontext_t context);
void emit_state(surgescript_nodecontext_t context);
void emit_identifier(surgescript_nodecontext_t context, const char* identifier, int line);
void emit_null(surgescript_nodecontext_t context);
void emit_bool(surgescript_nodecontext_t context, bool value);
void emit_number(surgescript_nodecontext_t context, float value);
void emit_string(surgescript_nodecontext_t context, const char* value);
void emit_object(surgescript_nodecontext_t context, unsigned handle);
void emit_zero(surgescript_nodecontext_t context);

/* misc */
void emit_setstate(surgescript_nodecontext_t context);
void emit_nop(surgescript_nodecontext_t context);
void emit_push(surgescript_nodecontext_t context);
void emit_pop(surgescript_nodecontext_t context);
void emit_breakpoint(surgescript_nodecontext_t context, const char* text);

#endif