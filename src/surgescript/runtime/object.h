/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2023 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * util/object.h
 * SurgeScript object
 */

#ifndef _SURGESCRIPT_RUNTIME_OBJECT_H
#define _SURGESCRIPT_RUNTIME_OBJECT_H

#include <stdbool.h>
#include "heap.h"

/* types */
typedef struct surgescript_object_t surgescript_object_t;

/* forward declarations */
struct surgescript_programpool_t;
struct surgescript_objectmanager_t;
struct surgescript_program_t;
struct surgescript_stack_t;
struct surgescript_heap_t;
struct surgescript_var_t;
struct surgescript_transform_t;




/* public methods */

/* programs */
bool surgescript_object_update(surgescript_object_t* object); /* runs my programs */

/* properties */
const char* surgescript_object_name(const surgescript_object_t* object); /* what's my name? */
struct surgescript_heap_t* surgescript_object_heap(const surgescript_object_t* object); /* each object has its own heap */
struct surgescript_objectmanager_t* surgescript_object_manager(const surgescript_object_t* object); /* pointer to the object manager */
void* surgescript_object_userdata(const surgescript_object_t* object); /* custom user data (if any) */
void surgescript_object_set_userdata(surgescript_object_t* object, void* data); /* set custom user data */
bool surgescript_object_has_tag(const surgescript_object_t* object, const char* tag_name); /* is this object tagged tag_name? */
bool surgescript_object_has_function(const surgescript_object_t* object, const char* fun_name); /* does the object have the specified function? */
double surgescript_object_elapsed_time(const surgescript_object_t* object); /* elapsed time (in seconds) since last state change */
double surgescript_object_timespent(const surgescript_object_t* object); /* average time consumption (in seconds) */
size_t surgescript_object_memspent(const surgescript_object_t* object); /* memory consumption (in bytes) */

/* object tree */
unsigned surgescript_object_handle(const surgescript_object_t* object); /* "this" pointer handle (in the object manager) */
unsigned surgescript_object_parent(const surgescript_object_t* object); /* parent object handle (in the object manager) */
unsigned surgescript_object_nth_child(const surgescript_object_t* object, int index); /* n-th child */
int surgescript_object_child_count(const surgescript_object_t* object); /* how many children there are? */
unsigned surgescript_object_child(const surgescript_object_t* object, const char* name); /* gets the handle to a child named name */
int surgescript_object_children(const surgescript_object_t* object, const char* name, void* data, void (*callback)(unsigned,void*)); /* gets all direct children named name */
unsigned surgescript_object_tagged_child(const surgescript_object_t* object, const char* tag_name); /* gets the handle to a child tagged tag_name */
int surgescript_object_tagged_children(const surgescript_object_t* object, const char* tag_name, void* data, void (*callback)(unsigned,void*)); /* gets all direct children tagged tag_name */
unsigned surgescript_object_find_descendant(const surgescript_object_t* object, const char* name); /* finds a descendant named name */
int surgescript_object_find_descendants(const surgescript_object_t* object, const char* name, void* data, void (*callback)(unsigned,void*)); /* finds all descendants named name */
unsigned surgescript_object_find_tagged_descendant(const surgescript_object_t* object, const char* tag_name); /* find 1st child (or grand-child...) tagged tag_name */
int surgescript_object_find_tagged_descendants(const surgescript_object_t* object, const char* tag_name, void* data, void (*callback)(unsigned,void*)); /* finds all descendants tagged tag_name */
unsigned surgescript_object_find_ascendant(const surgescript_object_t* object, const char* name); /* finds an ascendant named name */
bool surgescript_object_traverse_tree(surgescript_object_t* object, bool (*callback)(surgescript_object_t*)); /* traverses the object tree, calling the callback function for each object */
bool surgescript_object_traverse_tree_ex(surgescript_object_t* object, void* data, bool (*callback)(surgescript_object_t*,void*)); /* tree traversal with an additional data parameter */
int surgescript_object_depth(const surgescript_object_t* object); /* depth in the object tree (root has depth zero) */
bool surgescript_object_is_ascendant(const surgescript_object_t* object, unsigned ascendant_handle); /* is an object an ascendant of another? */
bool surgescript_object_add_child(surgescript_object_t* object, unsigned child_handle); /* adds a child to this object */
bool surgescript_object_remove_child(surgescript_object_t* object, unsigned child_handle); /* removes a child having this handle from this object */
bool surgescript_object_reparent(surgescript_object_t* object, unsigned new_parent_handle, int flags); /* changes the parent of this object */

/* life operations */
const char* surgescript_object_state(const surgescript_object_t *object); /* each object is a state machine. in which state am i in? */
void surgescript_object_set_state(surgescript_object_t* object, const char* state_name); /* sets a state; default is "main" */
bool surgescript_object_is_active(const surgescript_object_t* object); /* am i active? an object runs its programs iff it's active */
void surgescript_object_set_active(surgescript_object_t* object, bool active); /* sets whether i am active or not; default is true */
bool surgescript_object_is_killed(const surgescript_object_t* object); /* has this object been killed? */
void surgescript_object_kill(surgescript_object_t* object); /* will destroy the object as soon as the opportunity arises */

/* transform */
void surgescript_object_peek_transform(const surgescript_object_t* object, struct surgescript_transform_t* transform); /* reads the local transform */
void surgescript_object_poke_transform(surgescript_object_t* object, const struct surgescript_transform_t* transform); /* sets the local transform */
bool surgescript_object_transform_changed(const surgescript_object_t* object); /* has the local transform ever been changed? */
struct surgescript_transform_t* surgescript_object_transform(surgescript_object_t* object); /* inner pointer to the local transform */

/* call SurgeScript functions from C (you may pass NULL to return_value; you may also pass NULL to param iff num_params is 0) */
void surgescript_object_call_function(surgescript_object_t* object, const char* fun_name, const struct surgescript_var_t* param[], int num_params, struct surgescript_var_t* return_value); /* call a SurgeScript function */
void surgescript_object_call_super_function(surgescript_object_t* object, const char* fun_name, const struct surgescript_var_t* param[], int num_params, struct surgescript_var_t* return_value); /* call a function of the super class */
void surgescript_object_call_state(surgescript_object_t* object, const char* state_name); /* run the code of a state */
void surgescript_object_call_current_state(surgescript_object_t* object); /* run the code of the current state */

#endif
