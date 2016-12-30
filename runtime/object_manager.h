/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
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
bool surgescript_objectmanager_update(surgescript_objectmanager_t* manager); /* updates all objects; returns false when there are no more objects present */

/* operations */
surgescript_objectmanager_handle_t surgescript_objectmanager_spawn(surgescript_objectmanager_t* manager, const char* object_name, void* user_data, bool (*on_init)(struct surgescript_object_t*), bool (*on_release)(struct surgescript_object_t*)); /* spawns a new object; user_data and callbacks may be NULL */
struct surgescript_object_t* surgescript_objectmanager_get(surgescript_objectmanager_t* manager, surgescript_objectmanager_handle_t handle); /* returns NULL if the object is not found */
bool surgescript_objectmanager_delete(surgescript_objectmanager_t* manager, surgescript_objectmanager_handle_t handle); /* deletes an existing object; returns true on success */

/* utilities */
surgescript_objectmanager_handle_t surgescript_objectmanager_root(surgescript_objectmanager_t* manager); /* handle to the root object (the first one to be added) */

#endif
