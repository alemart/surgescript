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
bool surgescript_programpool_put(surgescript_programpool_t* pool, const char* object_name, const char* program_name, struct surgescript_program_t* program); /* adds a program to an object */
struct surgescript_program_t* surgescript_programpool_get(surgescript_programpool_t* pool, const char* object_name, const char* program_name); /* may return NULL */
bool surgescript_programpool_exists(surgescript_programpool_t* pool, const char* object_name, const char* program_name); /* program exists? */
bool surgescript_programpool_shallowcheck(surgescript_programpool_t* pool, const char* object_name, const char* program_name); /* program exists? (shallow check) */
void surgescript_programpool_foreach(surgescript_programpool_t* pool, const char* object_name, void (*callback)(const char*)); /* for each program of object_name... */
void surgescript_programpool_foreach_ex(surgescript_programpool_t* pool, const char* object_name, void* data, void (*callback)(const char*, void*)); /* same as above with an added data parameter */
void surgescript_programpool_foreach_object(surgescript_programpool_t* pool, void (*callback)(const char*)); /* for each object named object_name... */
void surgescript_programpool_foreach_object_ex(surgescript_programpool_t* pool, void* data, void (*callback)(const char*, void*)); /* same as above with an added data parameter */
bool surgescript_programpool_replace(surgescript_programpool_t* pool, const char* object_name, const char* program_name, struct surgescript_program_t* program); /* replaces a program */
void surgescript_programpool_delete(surgescript_programpool_t* pool, const char* object_name, const char* program_name); /* deletes a programs from the specified object */
void surgescript_programpool_purge(surgescript_programpool_t* pool, const char* object_name); /* deletes all programs from the specified object */
bool surgescript_programpool_is_compiled(surgescript_programpool_t* pool, const char* object_name); /* is there any code for object_name? */
void surgescript_programpool_lock(surgescript_programpool_t* pool); /* locks the program pool, so that no (programs of) new objects can be added to it */

#endif
