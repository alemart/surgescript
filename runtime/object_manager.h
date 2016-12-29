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

/* opaque types */
typedef struct surgescript_objectmanager_t surgescript_objectmanager_t;
typedef unsigned surgescript_objectmanager_handle_t;

/* forward declarations */
struct surgescript_object_t;

/* public methods */
surgescript_objectmanager_t* surgescript_objectmanager_create(struct surgescript_object_t* root);
surgescript_objectmanager_t* surgescript_objectmanager_destroy(surgescript_objectmanager_t* pool);

/* http verbs */
surgescript_objectmanager_handle_t surgescript_objectmanager_put(surgescript_objectmanager_t* pool, struct surgescript_object_t* object);
struct surgescript_object_t* surgescript_objectmanager_get(surgescript_objectmanager_t* pool, surgescript_objectmanager_handle_t handle); /* returns NULL if the object is not found */
surgescript_objectmanager_handle_t surgescript_objectmanager_delete(surgescript_objectmanager_t* pool, surgescript_objectmanager_handle_t handle);

/* utilities */
surgescript_objectmanager_handle_t surgescript_objectmanager_get_root(surgescript_objectmanager_t* pool); /* handle to the root object */

#endif
