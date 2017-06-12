/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016-2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object_manager.h
 * SurgeScript object manager
 */

#ifndef _SURGESCRIPT_RUNTIME_OBJECTMANAGER_H
#define _SURGESCRIPT_RUNTIME_OBJECTMANAGER_H

#include <stdbool.h>

/* opaque types */
typedef struct surgescript_objectmanager_t surgescript_objectmanager_t;
typedef unsigned surgescript_objectmanager_handle_t;

/* forward declarations */
struct surgescript_object_t;
struct surgescript_programpool_t;
struct surgescript_stack_t;



/* public methods */

/* life-cycle */
surgescript_objectmanager_t* surgescript_objectmanager_create(struct surgescript_programpool_t* program_pool, struct surgescript_stack_t* stack);
surgescript_objectmanager_t* surgescript_objectmanager_destroy(surgescript_objectmanager_t* manager);

/* operations */
surgescript_objectmanager_handle_t surgescript_objectmanager_spawn_root(surgescript_objectmanager_t* manager); /* spawns the root object */
surgescript_objectmanager_handle_t surgescript_objectmanager_spawn(surgescript_objectmanager_t* manager, surgescript_objectmanager_handle_t parent, const char* object_name, void* user_data); /* spawns a new object; user_data may be NULL */
bool surgescript_objectmanager_exists(surgescript_objectmanager_t* manager, surgescript_objectmanager_handle_t handle); /* does the specified handle points to a valid object? */
struct surgescript_object_t* surgescript_objectmanager_get(surgescript_objectmanager_t* manager, surgescript_objectmanager_handle_t handle); /* returns NULL if the object is not found */
bool surgescript_objectmanager_delete(surgescript_objectmanager_t* manager, surgescript_objectmanager_handle_t handle); /* deletes an existing object; returns true on success */

/* utilities */
int surgescript_objectmanager_count(surgescript_objectmanager_t* manager); /* how many objects there are? */
void surgescript_objectmanager_collectgarbage(surgescript_objectmanager_t* manager); /* runs the garbage collector */
struct surgescript_programpool_t* surgescript_objectmanager_programpool(const surgescript_objectmanager_t* manager); /* pointer to the program pool */

/* tags */
void surgescript_objectmanager_add_tag(surgescript_objectmanager_t* manager, const char* object_name, const char* tag_name); /* add tag_name to a certain class of objects */
bool surgescript_objectmanager_has_tag(const surgescript_objectmanager_t* manager, const char* object_name, const char* tag_name); /* is object_name tagged tag_name? */
const char* surgescript_objectmanager_tagged_object(const surgescript_objectmanager_t* manager, const char* tag_name, int index, void **tmp); /* index >= 0; returns NULL if no object */

/* root & built-in objects */
surgescript_objectmanager_handle_t surgescript_objectmanager_null(surgescript_objectmanager_t* manager); /* handle to a null object */
surgescript_objectmanager_handle_t surgescript_objectmanager_root(surgescript_objectmanager_t* manager); /* handle to the root object (the first one to be added) */
surgescript_objectmanager_handle_t surgescript_objectmanager_application(surgescript_objectmanager_t* manager); /* handle to the user's application */
surgescript_objectmanager_handle_t surgescript_objectmanager_system_object(surgescript_objectmanager_t* manager, const char* object_name); /* handle to the specified system object */

#endif
