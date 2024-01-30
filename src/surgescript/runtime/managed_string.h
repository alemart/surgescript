/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2024 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/managed_string.h
 * Fast immutable strings
 */

#ifndef _SURGESCRIPT_RUNTIME_MANAGED_STRING_H
#define _SURGESCRIPT_RUNTIME_MANAGED_STRING_H

typedef struct surgescript_managedstring_t surgescript_managedstring_t;

/* create & destroy */
surgescript_managedstring_t* surgescript_managedstring_create(const char* string);
surgescript_managedstring_t* surgescript_managedstring_destroy(surgescript_managedstring_t* managed_string);
surgescript_managedstring_t* surgescript_managedstring_clone(const surgescript_managedstring_t* managed_string);

/* quickly read the string */
#define surgescript_managedstring_data(managed_string) (*((const char**)managed_string))

/* string pool */
void surgescript_managedstring_init_pool();
void surgescript_managedstring_release_pool();

#endif