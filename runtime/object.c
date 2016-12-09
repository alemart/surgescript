/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object.c
 * SurgeScript object
 */

#include "object.h"
#include "program_pool.h"
#include "object_pool.h"
#include "program.h"
#include "heap.h"
#include "stack.h"
#include "../util/ssarray.h"
#include "../util/util.h"

/* write class.c ? class.h ... it's better, non-duplicate hashes */
/* object.create("classname") */

/* an object is this: */
struct surgescript_object_t
{
    char* name; /* my name */
    surgescript_heap_t* heap; /* each object has its own heap */
    int is_destroyed;

    /* object tree */
    unsigned handle; /* "this" pointer in the object pool */
    unsigned parent; /* handle to the parent in the object pool */
    SSARRAY(unsigned, child); /* handles to the children */
};

/* -------------------------------
 * public methods
 * ------------------------------- */

surgescript_object_t* surgescript_object_create()
{
    return NULL;
}

surgescript_object_t* surgescript_object_clone(const surgescript_object_t* object)
{
    return NULL;
}

surgescript_object_t* surgescript_object_destroy(surgescript_object_t* object)
{
    /* can't destroy root object => exit program? */
    /* assign my children to my parent */
    // WHEN RUNNING THE SCRIPT (is_destroyed = true)
    // this fun just frees things
    // the runtime env, which includes the obj pool, is passed to the exec method
    return NULL;
}

const char* surgescript_object_name(const surgescript_object_t* object)
{
    return "null";
    return object->name;
}

surgescript_heap_t* surgescript_object_heap(const surgescript_object_t* object)
{
    return NULL;
    return object->heap;
}

void surgescript_object_register_program(surgescript_object_t* object, const char* fun_name, struct surgescript_program_t* program)
{

}
