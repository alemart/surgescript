/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * compiler/parsetree.c
 * SurgeScript compiler: parse trees, for emitting commands
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "parsetree.h"
#include "../util/util.h"
#include "../util/ssarray.h"
#include "../runtime/program.h"

/* tree structures (Composite Pattern) */
typedef struct surgescript_parsetree_number_t surgescript_parsetree_number_t;
typedef struct surgescript_parsetree_string_t surgescript_parsetree_string_t;
typedef struct surgescript_parsetree_bool_t surgescript_parsetree_bool_t;
typedef struct surgescript_parsetree_null_t surgescript_parsetree_null_t;
typedef struct surgescript_parsetree_objectlist_t surgescript_parsetree_objectlist_t;
typedef struct surgescript_parsetree_unary_t surgescript_parsetree_unary_t;

/* basic tree */
struct surgescript_parsetree_t
{
    surgescript_program_t* program; /* the program we belong to (may be NULL) */
    void (*emit)(surgescript_parsetree_t*); /* method for emitting code */
    void (*release)(surgescript_parsetree_t*); /* destructor */
};
static surgescript_parsetree_t* init_tree(void* tree, surgescript_program_t* program, void (*emit)(surgescript_parsetree_t*), void (*release)(surgescript_parsetree_t*));
static void emit_tree(surgescript_parsetree_t* tree);
static void release_tree(surgescript_parsetree_t* tree);

/* constants */
struct surgescript_parsetree_number_t
{
    surgescript_parsetree_t base;
    float value;
};
static void emit_number(surgescript_parsetree_t* tree);

struct surgescript_parsetree_string_t
{
    surgescript_parsetree_t base;
    char* value;
};
static void emit_string(surgescript_parsetree_t* tree);
static void release_string(surgescript_parsetree_t* tree);

struct surgescript_parsetree_bool_t
{
    surgescript_parsetree_t base;
    bool value;
};
static void emit_bool(surgescript_parsetree_t* tree);

struct surgescript_parsetree_null_t
{
    surgescript_parsetree_t base;
};
static void emit_null(surgescript_parsetree_t* tree);


/* object list & decl */
struct surgescript_parsetree_objectlist_t
{
    surgescript_parsetree_t base;
    char* name;
    surgescript_parsetree_t* objectdecl;
    surgescript_parsetree_t* objectlist;
};
static void emit_objectlist(surgescript_parsetree_t* tree);
static void release_objectlist(surgescript_parsetree_t* tree);



/* operators */
struct surgescript_parsetree_unary_t
{
    surgescript_parsetree_t base;
    char unaryop;
    surgescript_parsetree_t* value;
};
static void emit_unary(surgescript_parsetree_t* tree);
static void release_unary(surgescript_parsetree_t* tree);



/* public api */

/*
 * surgescript_parsetree_create()
 * create an empty tree
 */
surgescript_parsetree_t* surgescript_parsetree_create()
{
    surgescript_parsetree_t* node = ssmalloc(sizeof *node);
    return init_tree(node, NULL, NULL, NULL);
}

/*
 * surgescript_parsetree_destroy()
 * destroy a parse tree
 */
surgescript_parsetree_t* surgescript_parsetree_destroy(surgescript_parsetree_t* tree)
{
    tree->release(tree);
    return NULL;
}

/*
 * surgescript_parsetree_emit()
 * Emits the code corresponding to this tree
 */
void surgescript_parsetree_emit(surgescript_parsetree_t* tree)
{
    tree->emit(tree);
}









/*
 * surgescript_parsetree_create_null()
 * Creates a null node
 */
surgescript_parsetree_t* surgescript_parsetree_create_null(surgescript_program_t* program)
{
    surgescript_parsetree_null_t* tree = ssmalloc(sizeof *tree);
    return init_tree(tree, program, emit_null, NULL);
}

/*
 * surgescript_parsetree_create_bool()
 * Creates a boolean node
 */
surgescript_parsetree_t* surgescript_parsetree_create_bool(surgescript_program_t* program, bool value)
{
    surgescript_parsetree_number_t* tree = ssmalloc(sizeof *tree);
    tree->value = value;
    return init_tree(tree, program, emit_bool, NULL);
}

/*
 * surgescript_parsetree_create_number()
 * Creates a numeric node
 */
surgescript_parsetree_t* surgescript_parsetree_create_number(surgescript_program_t* program, float value)
{
    surgescript_parsetree_number_t* tree = ssmalloc(sizeof *tree);
    tree->value = value;
    return init_tree(tree, program, emit_number, NULL);
}

/*
 * surgescript_parsetree_create_string()
 * Creates a text node
 */
surgescript_parsetree_t* surgescript_parsetree_create_string(surgescript_program_t* program, const char* value)
{
    surgescript_parsetree_string_t* tree = ssmalloc(sizeof *tree);
    tree->value = surgescript_util_strdup(value);
    return init_tree(tree, program, emit_string, release_string);
}











/*
 * surgescript_parsetree_create_objectlist()
 * A list of objects
 */
surgescript_parsetree_t* surgescript_parsetree_create_objectlist(const char* name, surgescript_parsetree_t* objectdecl, surgescript_parsetree_t* objectlist)
{
    surgescript_parsetree_objectlist_t* node = ssmalloc(sizeof *node);
    node->name = surgescript_util_strdup(name);
    node->objectdecl = objectdecl;
    node->objectlist = objectlist;
    return init_tree(node, NULL, emit_objectlist, release_objectlist);
}








/*
 * surgescript_parser_create_unary()
 * Create a unary operator
 */
surgescript_parsetree_t* surgescript_parsetree_create_unary(struct surgescript_program_t* program, const char* unaryop, surgescript_parsetree_t* value)
{
    surgescript_parsetree_unary_t* node = ssmalloc(sizeof *node);
    node->unaryop = unaryop[0];
    node->value = value;
    return init_tree(node, program, emit_unary, release_unary);
}



/*
 * surgescript_()
 * 
 */




/* privates */

/* helper for initializing trees */
surgescript_parsetree_t* init_tree(void* tree, surgescript_program_t* program, void (*emit)(surgescript_parsetree_t*), void (*release)(surgescript_parsetree_t*))
{
    surgescript_parsetree_t* node = (surgescript_parsetree_t*)tree;
    node->program = program;
    node->emit = emit ? emit : emit_tree;
    node->release = release ? release : release_tree;
    return node;
}

/* do nothing (generate no-operation code) */
void emit_tree(surgescript_parsetree_t* tree)
{
    ;
}

/* just release itself (useful for releasing basic trees) */
void release_tree(surgescript_parsetree_t* tree)
{
    ssfree(tree);
}




/* code generation: constants */
void emit_number(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_number_t* node = (surgescript_parsetree_number_t*)tree;
    printf("\t%f\n", node->value);
}

void emit_bool(surgescript_parsetree_t* tree)
{
    ;
}
void emit_null(surgescript_parsetree_t* tree)
{
    printf("\tnull\n");
    ;
}

void emit_string(surgescript_parsetree_t* tree)
{
    ;
}

void release_string(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_string_t* node = (surgescript_parsetree_string_t*)tree;
    ssfree(node->value);
    release_tree(tree); /* base class destructor */
}


/* object list & decl */
void emit_objectlist(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_objectlist_t* node = (surgescript_parsetree_objectlist_t*)tree;
    printf("Object \"%s\":\n", node->name);
    node->objectdecl->emit(node->objectdecl);
    node->objectlist->emit(node->objectlist);
}

void release_objectlist(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_objectlist_t* node = (surgescript_parsetree_objectlist_t*)tree;
    node->objectlist->release(node->objectlist);
    node->objectdecl->release(node->objectdecl);
    ssfree(node->name);
    release_tree(tree);
}

/* operators */
void emit_unary(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_unary_t* node = (surgescript_parsetree_unary_t*)tree;
    putchar(node->unaryop);
    node->value->emit(node->value);
}

void release_unary(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_unary_t* node = (surgescript_parsetree_unary_t*)tree;
    node->value->release(node->value);
    release_tree(tree);
}