/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2022 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * compiler/asm.h
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
void emit_vargetter(surgescript_nodecontext_t context, const char* identifier);
void emit_varsetter(surgescript_nodecontext_t context, const char* identifier);

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
void emit_getter(surgescript_nodecontext_t context, const char* property_name);
void emit_setter1(surgescript_nodecontext_t context, const char* property_name);
void emit_setter2(surgescript_nodecontext_t context, const char* property_name, const char* assignop);
void emit_setterincdec(surgescript_nodecontext_t context, const char* property_name, const char* op);
void emit_arrayexpr1(surgescript_nodecontext_t context);
void emit_arrayexpr2(surgescript_nodecontext_t context);
void emit_arrayelement(surgescript_nodecontext_t context);
void emit_dictdecl1(surgescript_nodecontext_t context);
void emit_dictdecl2(surgescript_nodecontext_t context);
void emit_dictdeclkey(surgescript_nodecontext_t context);
void emit_dictdeclvalue(surgescript_nodecontext_t context);
void emit_timeout(surgescript_nodecontext_t context);
void emit_assert(surgescript_nodecontext_t context, int line);

/* statements */
void emit_if(surgescript_nodecontext_t context, surgescript_program_label_t nope);
void emit_else(surgescript_nodecontext_t context, surgescript_program_label_t nope, surgescript_program_label_t done);
void emit_endif(surgescript_nodecontext_t context, surgescript_program_label_t done);
void emit_while1(surgescript_nodecontext_t context, surgescript_program_label_t begin);
void emit_whilecheck(surgescript_nodecontext_t context, surgescript_program_label_t end);
void emit_while2(surgescript_nodecontext_t context, surgescript_program_label_t begin, surgescript_program_label_t end);
void emit_foreach1(surgescript_nodecontext_t context, const char* identifier, surgescript_program_label_t begin, surgescript_program_label_t end);
void emit_foreach2(surgescript_nodecontext_t context, const char* identifier, surgescript_program_label_t begin, surgescript_program_label_t end);
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
void emit_caller(surgescript_nodecontext_t context);
void emit_identifier(surgescript_nodecontext_t context, const char* identifier, int line);
void emit_null(surgescript_nodecontext_t context);
void emit_bool(surgescript_nodecontext_t context, bool value);
void emit_number(surgescript_nodecontext_t context, double value);
void emit_string(surgescript_nodecontext_t context, const char* value);
void emit_object(surgescript_nodecontext_t context, unsigned handle);
void emit_zero(surgescript_nodecontext_t context);

/* misc */
void emit_setstate(surgescript_nodecontext_t context);
void emit_nop(surgescript_nodecontext_t context);
void emit_breakpoint(surgescript_nodecontext_t context, const char* text);

#endif