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

/* expressions */
void emit_assignexpr(surgescript_nodecontext_t context, const char* identifier, const char* assignop);
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

/* constants */
void emit_null(surgescript_nodecontext_t context);
void emit_bool(surgescript_nodecontext_t context, bool value);
void emit_number(surgescript_nodecontext_t context, float value);
void emit_string(surgescript_nodecontext_t context, const char* value);

#endif