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

struct surgescript_symtable_t;
struct surgescript_program_t;
struct surgescript_programpool_t;

/* node context */
typedef struct surgescript_nodecontext_t
{
    const char* object_name;
    struct surgescript_symtable_t* symtable;
    struct surgescript_program_t* program;
} surgescript_nodecontext_t;

/* node context constructor */
static inline surgescript_nodecontext_t nodecontext(const char* object_name, struct surgescript_symtable_t* symbol_table, struct surgescript_program_t* program)
{
    surgescript_nodecontext_t ctx = { object_name, symbol_table, program };
    return ctx;
}

#endif