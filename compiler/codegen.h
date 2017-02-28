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

/* constants */
void emit_null(surgescript_nodecontext_t context);
void emit_bool(surgescript_nodecontext_t context, bool value);
void emit_number(surgescript_nodecontext_t context, float value);
void emit_string(surgescript_nodecontext_t context, const char* value);

#endif