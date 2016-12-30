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

#include <stdbool.h>

/* types */
typedef struct surgescript_object_t surgescript_object_t;
typedef const char* surgescript_object_annotation_param_t;

/* forward declarations */
struct surgescript_programpool_t;
struct surgescript_objectmanager_t;
struct surgescript_program_t;
struct surgescript_stack_t;
struct surgescript_heap_t;

/* public methods */

/* programs */
bool surgescript_object_update(surgescript_object_t* object); /* runs my programs */
bool surgescript_object_update_fulltree(surgescript_object_t* object); /* runs my programs and those of my children */

/* properties */
const char* surgescript_object_name(const surgescript_object_t* object); /* what's my name? */
struct surgescript_heap_t* surgescript_object_heap(const surgescript_object_t* object); /* each object has its own heap */
void* surgescript_object_userdata(const surgescript_object_t* object); /* custom user-data (if any) */

/* object tree */
unsigned surgescript_object_handle(const surgescript_object_t* object); /* "this" pointer (in the object manager) */
unsigned surgescript_object_parent(const surgescript_object_t* object); /* parent object (in the object manager) */
unsigned surgescript_object_child(const surgescript_object_t* object, int index); /* n-th child */
int surgescript_object_child_count(const surgescript_object_t* object); /* how many children there are? */
unsigned surgescript_object_find_child(const surgescript_object_t* object, const char* name); /* find 1st child whose name equals name */
void surgescript_object_add_child(surgescript_object_t* object, unsigned child_handle); /* adds a child to this object */
bool surgescript_object_remove_child(surgescript_object_t* object, unsigned child_handle); /* removes a child having this handle from this object */

/* life operations */
bool surgescript_object_is_active(const surgescript_object_t* object); /* am i active? an object runs its programs iff it's active */
void surgescript_object_set_active(surgescript_object_t* object, bool active); /* sets whether i am active or not; default is true */
const char* surgescript_object_state(const surgescript_object_t *object); /* each object is a state machine. in which state am i in? */
void surgescript_object_set_state(surgescript_object_t* object, const char* state_name); /* sets a state; default is "main" */
bool surgescript_object_is_killed(const surgescript_object_t* object); /* has this object been killed? */
void surgescript_object_kill(surgescript_object_t* object); /* will destroy the object as soon as the opportunity arises */

/* annotations: an annotation is a string plus a set of zero or more annotation parameters (which are also strings) */
/*void surgescript_object_add_annotation(surgescript_object_t* object, const char* annotation);
void surgescript_object_add_anotation_parameter(surgescript_object_t* object, const char* annotation, const char* param);
int surgescript_object_annotation_exists(surgescript_object_t* object, const char* annotation);
const char** surgescript_object_annotation_parameters(surgescript_object_t* object, const char* annotation, int *num_params);*/

#endif
