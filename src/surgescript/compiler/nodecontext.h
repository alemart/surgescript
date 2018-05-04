/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2018 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * compiler/nodecontext.h
 * SurgeScript compiler: node context utility (for the parser)
 */

#ifndef _SURGESCRIPT_COMPILER_NODECONTEXT_H
#define _SURGESCRIPT_COMPILER_NODECONTEXT_H

#include "../runtime/program.h"

struct surgescript_symtable_t;

/* node context */
typedef struct surgescript_nodecontext_t
{
    const char* source_file;
    const char* object_name;
    struct surgescript_symtable_t* symtable;
    surgescript_program_t* program;
    surgescript_program_label_t loop_begin;
    surgescript_program_label_t loop_end;
} surgescript_nodecontext_t;

/* node context constructor */
static inline surgescript_nodecontext_t nodecontext(const char* source_file, const char* object_name, struct surgescript_symtable_t* symbol_table, surgescript_program_t* program)
{
    surgescript_nodecontext_t ctx = { source_file, object_name, symbol_table, program, SURGESCRIPT_PROGRAM_UNDEFINED_LABEL, SURGESCRIPT_PROGRAM_UNDEFINED_LABEL };
    return ctx;
}

#endif