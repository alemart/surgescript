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
typedef struct surgescript_parsetree_object_t surgescript_parsetree_object_t;
typedef struct surgescript_parsetree_objectdecl_t surgescript_parsetree_objectdecl_t;

/* basic tree */
struct surgescript_parsetree_t
{
    surgescript_nodecontext_t context; /* the context of this node */
    void (*emit)(surgescript_parsetree_t*); /* method for emitting code */
    void (*release)(surgescript_parsetree_t*); /* destructor */
};
static inline surgescript_parsetree_t* init_tree(void* tree, surgescript_nodecontext_t context, void (*emit)(surgescript_parsetree_t*), void (*release)(surgescript_parsetree_t*));
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
    surgescript_parsetree_t* object;
    surgescript_parsetree_t* objectlist;
};
static void emit_objectlist(surgescript_parsetree_t* tree);
static void release_objectlist(surgescript_parsetree_t* tree);

struct surgescript_parsetree_object_t
{
    surgescript_parsetree_t base;
    surgescript_parsetree_t* objectdecl;
};
static void emit_object(surgescript_parsetree_t* tree);
static void release_object(surgescript_parsetree_t* tree);

struct surgescript_parsetree_objectdecl_t
{
    surgescript_parsetree_t base;
    surgescript_parsetree_t* notelist;
    surgescript_parsetree_t* vardecllist;
    surgescript_parsetree_t* statedecllist;
    surgescript_parsetree_t* fundecllist;
};
static void emit_objectdecl(surgescript_parsetree_t* tree);
static void release_objectdecl(surgescript_parsetree_t* tree);





/* public api */

/*
 * surgescript_parsetree_create()
 * create an empty tree
 */
surgescript_parsetree_t* surgescript_parsetree_create()
{
    surgescript_parsetree_t* node = ssmalloc(sizeof *node);
    return init_tree(node, nodecontext(NULL, NULL, NULL), NULL, NULL);
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



/* Constants */

/*
 * surgescript_parsetree_create_null()
 * Creates a null node
 */
surgescript_parsetree_t* surgescript_parsetree_create_null(surgescript_nodecontext_t context)
{
    surgescript_parsetree_null_t* node = ssmalloc(sizeof *node);
    return init_tree(node, context, emit_null, NULL);
}

void emit_null(surgescript_parsetree_t* tree)
{
    printf("\tnull\n");
    ;
}


/*
 * surgescript_parsetree_create_bool()
 * Creates a boolean node
 */
surgescript_parsetree_t* surgescript_parsetree_create_bool(surgescript_nodecontext_t context, bool value)
{
    surgescript_parsetree_number_t* node = ssmalloc(sizeof *node);
    node->value = value;
    return init_tree(node, context, emit_bool, NULL);
}

void emit_bool(surgescript_parsetree_t* tree)
{
    ;
}

/*
 * surgescript_parsetree_create_number()
 * Creates a numeric node
 */
surgescript_parsetree_t* surgescript_parsetree_create_number(surgescript_nodecontext_t context, float value)
{
    surgescript_parsetree_number_t* node = ssmalloc(sizeof *node);
    node->value = value;
    return init_tree(node, context, emit_number, NULL);
}

void emit_number(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_number_t* node = (surgescript_parsetree_number_t*)tree;
    printf("\t%f\n", node->value);
}




/*
 * surgescript_parsetree_create_string()
 * Creates a text node
 */
surgescript_parsetree_t* surgescript_parsetree_create_string(surgescript_nodecontext_t context, const char* value)
{
    surgescript_parsetree_string_t* node = ssmalloc(sizeof *node);
    node->value = ssstrdup(value);
    return init_tree(node, context, emit_string, release_string);
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





/* object list & decls */
/* (comment headers omitted) */

/* object list */
surgescript_parsetree_t* surgescript_parsetree_create_objectlist(surgescript_parsetree_t* object, surgescript_parsetree_t* objectlist)
{
    surgescript_parsetree_objectlist_t* node = ssmalloc(sizeof *node);
    node->object = object;
    node->objectlist = objectlist;
    return init_tree(node, nodecontext(NULL, NULL, NULL), emit_objectlist, release_objectlist);
}

void emit_objectlist(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_objectlist_t* node = (surgescript_parsetree_objectlist_t*)tree;
    printf("Object list:\n");
    node->object->emit(node->object);
    node->objectlist->emit(node->objectlist);
}

void release_objectlist(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_objectlist_t* node = (surgescript_parsetree_objectlist_t*)tree;
    node->object->release(node->object);
    node->objectlist->release(node->objectlist);
    release_tree(tree);
}



/* object */
surgescript_parsetree_t* surgescript_parsetree_create_object(surgescript_nodecontext_t context, surgescript_parsetree_t* objectdecl)
{
    surgescript_parsetree_object_t* node = ssmalloc(sizeof *node);
    node->objectdecl = objectdecl;
    return init_tree(node, context, emit_object, release_object);
}

void emit_object(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_object_t* node = (surgescript_parsetree_object_t*)tree;
    printf("\t%s:\n", tree->context.object_name);
    node->objectdecl->emit(node->objectdecl);
}

void release_object(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_object_t* node = (surgescript_parsetree_object_t*)tree;
    node->objectdecl->release(node->objectdecl);
    ssfree((char*)(tree->context.object_name));
    // TODO: free symbol table
    release_tree(tree);
}



/* object declarations */
surgescript_parsetree_t* surgescript_parsetree_create_objectdecl(surgescript_nodecontext_t context, surgescript_parsetree_t* notelist, surgescript_parsetree_t* vardecllist, surgescript_parsetree_t* statedecllist, surgescript_parsetree_t* fundecllist)
{
    surgescript_parsetree_objectdecl_t* node = ssmalloc(sizeof *node);
    node->notelist = notelist;
    node->vardecllist = vardecllist;
    node->statedecllist = statedecllist;
    node->fundecllist = fundecllist;
    return init_tree(node, context, emit_objectdecl, release_objectdecl);
}

void emit_objectdecl(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_objectdecl_t* node = (surgescript_parsetree_objectdecl_t*)tree;
    printf("\t\tdeclarations of %s => %p\n", tree->context.object_name, tree->context.program);
    node->notelist->emit(node->notelist);
    node->vardecllist->emit(node->vardecllist);
    node->statedecllist->emit(node->statedecllist);
    node->fundecllist->emit(node->fundecllist);
}

void release_objectdecl(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_objectdecl_t* node = (surgescript_parsetree_objectdecl_t*)tree;
    node->notelist->release(node->notelist);
    node->vardecllist->release(node->vardecllist);
    node->statedecllist->release(node->statedecllist);
    node->fundecllist->release(node->fundecllist);
    release_tree(tree);
}



/*


surgescript_parsetree_t* surgescript_parsetree_create_uuu(surgescript_nodecontext_t context)
{
    surgescript_parsetree_uuu_t* node = ssmalloc(sizeof *node);
    return init_tree(node, context, emit_uuu, release_uuu);
}

void emit_uuu(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_uuu_t* node = (surgescript_parsetree_uuu_t*)tree;
}

void release_uuu(surgescript_parsetree_t* tree)
{
    surgescript_parsetree_uuu_t* node = (surgescript_parsetree_uuu_t*)tree;
    release_tree(tree);
}


*/

/*
 * surgescript_()
 * 
 */




/* privates */

/* helper for initializing trees */
surgescript_parsetree_t* init_tree(void* tree, surgescript_nodecontext_t context, void (*emit)(surgescript_parsetree_t*), void (*release)(surgescript_parsetree_t*))
{
    surgescript_parsetree_t* node = (surgescript_parsetree_t*)tree;
    node->context = context;
    node->emit = emit ? emit : emit_tree;
    node->release = release ? release : release_tree;
    return node;
}

/* do nothing */
void emit_tree(surgescript_parsetree_t* tree)
{
    ;
}

/* just release itself (useful for releasing basic trees, i.e., leaf nodes) */
void release_tree(surgescript_parsetree_t* tree)
{
    ssfree(tree);
}