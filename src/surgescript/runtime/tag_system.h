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
 * runtime/tag_system.h
 * SurgeScript Tag System
 */

#ifndef _SURGESCRIPT_RUNTIME_TAG_SYSTEM_H
#define _SURGESCRIPT_RUNTIME_TAG_SYSTEM_H

typedef struct surgescript_tagsystem_t surgescript_tagsystem_t;
typedef struct surgescript_boundtagsystem_t surgescript_boundtagsystem_t;

/* tag system */
surgescript_tagsystem_t* surgescript_tagsystem_create();
surgescript_tagsystem_t* surgescript_tagsystem_destroy(surgescript_tagsystem_t* tag_system);

/* add & check tags */
void surgescript_tagsystem_add_tag(surgescript_tagsystem_t* tag_system, const char* object_name, const char* tag_name); /* add tag_name to a certain class of objects */
bool surgescript_tagsystem_has_tag(const surgescript_tagsystem_t* tag_system, const char* object_name, const char* tag_name); /* is object_name tagged tag_name? */

/* iteration */
void surgescript_tagsystem_foreach_tag(const surgescript_tagsystem_t* tag_system, void* data, void (*callback)(const char*,void*)); /* for each registered tag, calls callback(tag_name, data) */
void surgescript_tagsystem_foreach_tagged_object(const surgescript_tagsystem_t* tag_system, const char* tag_name, void* data, void (*callback)(const char*,void*)); /* for each object tagged tag_name, calls callback(object_name, data) */
void surgescript_tagsystem_foreach_tag_of_object(const surgescript_tagsystem_t* tag_system, const char* object_name, void* data, void (*callback)(const char*,void*)); /* for each tag of object named object_name, calls callback(tag_name, data) */

/* bound tag system */
const surgescript_boundtagsystem_t* surgescript_tagsystem_bind(surgescript_tagsystem_t* tag_system, const char* object_name); /* get a bound tag system bound to object_name */
bool surgescript_boundtagsystem_has_tag(const surgescript_boundtagsystem_t* bound_tag_system, const char* tag_name); /* super quick tag test */

#endif
