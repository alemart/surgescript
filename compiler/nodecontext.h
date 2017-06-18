/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
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