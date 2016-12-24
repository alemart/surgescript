/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object.h
 * SurgeScript object
 */

#ifndef _SURGESCRIPT_RUNTIME_OBJECT_H
#define _SURGESCRIPT_RUNTIME_OBJECT_H

/* types */
typedef struct surgescript_object_t surgescript_object_t;

/* forward declarations */
struct surgescript_program_t;
struct surgescript_heap_t;

/* public methods */

/* these are handled by the object pool */
surgescript_object_t* surgescript_object_create();
surgescript_object_t* surgescript_object_clone(const surgescript_object_t* object);
surgescript_object_t* surgescript_object_destroy(surgescript_object_t* object);

/* properties */
const char* surgescript_object_name(const surgescript_object_t* object);
struct surgescript_heap_t* surgescript_object_heap(const surgescript_object_t* object);
unsigned surgescript_object_handle(const surgescript_object_t* object); /* "this" pointer (in the object pool) */
unsigned surgescript_object_parent(const surgescript_object_t* object); /* parent object (in the object pool) */
unsigned surgescript_object_child(const surgescript_object_t* object, int index); /* n-th child */
int surgescript_object_child_count(const surgescript_object_t* object);

void surgescript_object_set_name(surgescript_object_t* object, const char* name);
//void surgescript_object_register_program(surgescript_object_t* object, const char* fun_name, struct surgescript_program_t* program); // n preciso

#endif
