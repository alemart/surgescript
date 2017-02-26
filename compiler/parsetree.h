/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/parsetree.h
 * SurgeScript compiler: parse trees, for emitting commands
 */

#ifndef _SURGESCRIPT_COMPILER_PARSETREE_H
#define _SURGESCRIPT_COMPILER_PARSETREE_H

typedef struct surgescript_parsetree_t surgescript_parsetree_t;
struct surgescript_program_t;

/* basic operations */
surgescript_parsetree_t* surgescript_parsetree_create(); /* create an empty tree */
void surgescript_parsetree_emit(surgescript_parsetree_t* tree); /* emit code */
surgescript_parsetree_t* surgescript_parsetree_destroy(surgescript_parsetree_t* tree); /* destroy a parse tree */

/* constant expressions */
surgescript_parsetree_t* surgescript_parsetree_create_bool(struct surgescript_program_t* program, bool value);
surgescript_parsetree_t* surgescript_parsetree_create_number(struct surgescript_program_t* program, float value);
surgescript_parsetree_t* surgescript_parsetree_create_string(struct surgescript_program_t* program, const char* value);
surgescript_parsetree_t* surgescript_parsetree_create_null(struct surgescript_program_t* program);

/* object list & declarations */
surgescript_parsetree_t* surgescript_parsetree_create_objectlist(const char* name, surgescript_parsetree_t* objectdecl, surgescript_parsetree_t* objectlist);

/* operators */
surgescript_parsetree_t* surgescript_parsetree_create_unary(struct surgescript_program_t* program, const char* unaryop, surgescript_parsetree_t* value);

#endif