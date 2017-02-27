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

#include "nodecontext.h"

typedef struct surgescript_parsetree_t surgescript_parsetree_t;
struct surgescript_program_t;

/* basic operations */
surgescript_parsetree_t* surgescript_parsetree_create(); /* create an empty tree */
void surgescript_parsetree_emit(surgescript_parsetree_t* tree); /* emit code */
surgescript_parsetree_t* surgescript_parsetree_destroy(surgescript_parsetree_t* tree); /* destroy a parse tree */

/* object list & declarations */
surgescript_parsetree_t* surgescript_parsetree_create_objectlist(surgescript_parsetree_t* object, surgescript_parsetree_t* objectlist);
surgescript_parsetree_t* surgescript_parsetree_create_object(surgescript_nodecontext_t context, surgescript_parsetree_t* objectdecl);
surgescript_parsetree_t* surgescript_parsetree_create_objectdecl(surgescript_nodecontext_t context, surgescript_parsetree_t* notelist, surgescript_parsetree_t* vardecllist, surgescript_parsetree_t* statedecllist, surgescript_parsetree_t* fundecllist);

/* constants */
surgescript_parsetree_t* surgescript_parsetree_create_bool(surgescript_nodecontext_t context, bool value);
surgescript_parsetree_t* surgescript_parsetree_create_number(surgescript_nodecontext_t context, float value);
surgescript_parsetree_t* surgescript_parsetree_create_string(surgescript_nodecontext_t context, const char* value);
surgescript_parsetree_t* surgescript_parsetree_create_null(surgescript_nodecontext_t context);

#endif