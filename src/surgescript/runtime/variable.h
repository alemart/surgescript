/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2018 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/variable.h
 * SurgeScript variables
 */

#ifndef _SURGESCRIPT_RUNTIME_VARIABLE_H
#define _SURGESCRIPT_RUNTIME_VARIABLE_H

#include <stdlib.h>
#include <stdbool.h>

/* the variable type */
typedef struct surgescript_var_t surgescript_var_t;

/* misc */
struct surgescript_objectmanager_t;



/* public methods */

/* create & destroy variables */
surgescript_var_t* surgescript_var_create();
surgescript_var_t* surgescript_var_destroy(surgescript_var_t* var);

/* retrieve the value stored in a variable */
bool surgescript_var_is_null(const surgescript_var_t* var);
bool surgescript_var_get_bool(const surgescript_var_t* var);
float surgescript_var_get_number(const surgescript_var_t* var);
char* surgescript_var_get_string(const surgescript_var_t* var, const struct surgescript_objectmanager_t* manager); /* warning: allocates a new buffer; you have to ssfree() this. See also: surgescript_var_to_string() */
unsigned surgescript_var_get_objecthandle(const surgescript_var_t* var);

/* sets the value of a variable */
surgescript_var_t* surgescript_var_set_null(surgescript_var_t* var);
surgescript_var_t* surgescript_var_set_bool(surgescript_var_t* var, bool boolean);
surgescript_var_t* surgescript_var_set_number(surgescript_var_t* var, float number);
surgescript_var_t* surgescript_var_set_string(surgescript_var_t* var, const char* string);
surgescript_var_t* surgescript_var_set_objecthandle(surgescript_var_t* var, unsigned handle);

/* misc */
int surgescript_var_typecode(const surgescript_var_t* var); /* the typecode */
int surgescript_var_type2code(const char* type_name); /* typename -> typecode converter */
int surgescript_var_typecheck(const surgescript_var_t* var, int code); /* returns zero iff var has the given type code */
surgescript_var_t* surgescript_var_copy(surgescript_var_t* dst, const surgescript_var_t* src); /* similar to strcpy */
surgescript_var_t* surgescript_var_clone(const surgescript_var_t* var); /* similar to strdup */
char* surgescript_var_to_string(const surgescript_var_t* var, char* buf, size_t bufsize); /* copies var to buf and returns buf, converting var to string if necessary (similar to itoa / strncpy) */
const char* surgescript_var_fast_get_string(const surgescript_var_t* var); /* gets the string contents of var without performing any type conversion */
int surgescript_var_compare(const surgescript_var_t* a, const surgescript_var_t* b); /* similar to strcmp */
void surgescript_var_swap(surgescript_var_t* a, surgescript_var_t* b); /* swaps a <-> b */
int surgescript_var_get_rawbits(const surgescript_var_t* var); /* the binary value stored in var */
surgescript_var_t* surgescript_var_set_rawbits(surgescript_var_t* var, int raw); /* sets its binary value */
size_t surgescript_var_size(const surgescript_var_t* var); /* used memory in user space, in bytes */

/* var pooling */
void surgescript_var_init_pool();
void surgescript_var_release_pool();

#endif
