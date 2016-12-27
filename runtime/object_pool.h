/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/object_pool.h
 * SurgeScript object pool
 */

#ifndef _SURGESCRIPT_RUNTIME_OBJECTPOOL_H
#define _SURGESCRIPT_RUNTIME_OBJECTPOOL_H

/* opaque types */
typedef struct surgescript_objectpool_t surgescript_objectpool_t;
typedef unsigned surgescript_objectpool_handle_t;

/* forward declarations */
struct surgescript_object_t;

/* public methods */
surgescript_objectpool_t* surgescript_objectpool_create(struct surgescript_object_t* root);
surgescript_objectpool_t* surgescript_objectpool_destroy(surgescript_objectpool_t* pool);

/* http verbs */
surgescript_objectpool_handle_t surgescript_objectpool_put(surgescript_objectpool_t* pool, struct surgescript_object_t* object);
struct surgescript_object_t* surgescript_objectpool_get(surgescript_objectpool_t* pool, surgescript_objectpool_handle_t handle); /* returns NULL if the object is not found */
surgescript_objectpool_handle_t surgescript_objectpool_delete(surgescript_objectpool_t* pool, surgescript_objectpool_handle_t handle);

/* utilities */
surgescript_objectpool_handle_t surgescript_objectpool_get_root(surgescript_objectpool_t* pool); /* handle to the root object */

#endif
