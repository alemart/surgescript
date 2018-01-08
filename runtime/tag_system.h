/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/tag_system.h
 * SurgeScript Tag System
 */

#ifndef _SURGESCRIPT_RUNTIME_TAG_SYSTEM_H
#define _SURGESCRIPT_RUNTIME_TAG_SYSTEM_H

typedef struct surgescript_tagsystem_t surgescript_tagsystem_t;

/* tag system */
surgescript_tagsystem_t* surgescript_tagsystem_create();
surgescript_tagsystem_t* surgescript_tagsystem_destroy(surgescript_tagsystem_t* tag_system);

/* add & check tags */
void surgescript_tagsystem_add_tag(surgescript_tagsystem_t* tag_system, const char* object_name, const char* tag_name); /* add tag_name to a certain class of objects */
bool surgescript_tagsystem_has_tag(const surgescript_tagsystem_t* tag_system, const char* object_name, const char* tag_name); /* is object_name tagged tag_name? */

/* iteration */
void surgescript_tagsystem_foreach_tag(const surgescript_tagsystem_t* tag_system, void* data, void (*callback)(const char*,void*)); /* for each registered tag, calls callback(tag_name, data) */
void surgescript_tagsystem_foreach_tagged_object(const surgescript_tagsystem_t* tag_system, const char* tag_name, void* data, void (*callback)(const char*,void*)); /* for each object tagged tag_name, calls callback(object_name, data) */

#endif