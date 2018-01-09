/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016-2018  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/program_pool.h
 * SurgeScript program pool
 */

#ifndef _SURGESCRIPT_RUNTIME_PROGRAMPOOL_H
#define _SURGESCRIPT_RUNTIME_PROGRAMPOOL_H

#include <stdbool.h>

/* types */
typedef struct surgescript_programpool_t surgescript_programpool_t;

/* forward declarations */
struct surgescript_program_t;

/* public methods */
surgescript_programpool_t* surgescript_programpool_create();
surgescript_programpool_t* surgescript_programpool_destroy(surgescript_programpool_t* pool);
bool surgescript_programpool_put(surgescript_programpool_t* pool, const char* object_name, const char* program_name, struct surgescript_program_t* program);
struct surgescript_program_t* surgescript_programpool_get(surgescript_programpool_t* pool, const char* object_name, const char* program_name); /* may return NULL */
bool surgescript_programpool_exists(surgescript_programpool_t* pool, const char* object_name, const char* program_name); /* program exists? */
bool surgescript_programpool_shallowcheck(surgescript_programpool_t* pool, const char* object_name, const char* program_name); /* program exists? (shallow check) */
void surgescript_programpool_foreach(surgescript_programpool_t* pool, const char* object_name, void (*callback)(const char*)); /* for each program of object_name... */
void surgescript_programpool_foreach_ex(surgescript_programpool_t* pool, const char* object_name, void* data, void (*callback)(const char*, void*)); /* same as above with an added data parameter */
bool surgescript_programpool_replace(surgescript_programpool_t* pool, const char* object_name, const char* program_name, struct surgescript_program_t* program);

#endif
