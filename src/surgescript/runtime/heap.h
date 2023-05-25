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
 * runtime/heap.h
 * SurgeScript heap
 */

#ifndef _SURGESCRIPT_RUNTIME_HEAP_H
#define _SURGESCRIPT_RUNTIME_HEAP_H

#include <stdlib.h>
#include <stdbool.h>

/* types */
typedef struct surgescript_heap_t surgescript_heap_t;
struct surgescript_heap_t;
typedef unsigned surgescript_heapptr_t;

/* forward declarations */
struct surgescript_var_t;

/* public methods */
surgescript_heap_t* surgescript_heap_create();
surgescript_heap_t* surgescript_heap_destroy(surgescript_heap_t* heap);
surgescript_heapptr_t surgescript_heap_malloc(surgescript_heap_t* heap);
surgescript_heapptr_t surgescript_heap_free(surgescript_heap_t* heap, surgescript_heapptr_t ptr);
struct surgescript_var_t* surgescript_heap_at(const surgescript_heap_t* heap, surgescript_heapptr_t ptr);
void surgescript_heap_scan_objects(surgescript_heap_t* heap, void* userdata, bool (*callback)(unsigned,void*));
bool surgescript_heap_scan_all(surgescript_heap_t* heap, void* userdata, bool (*callback)(struct surgescript_var_t*,surgescript_heapptr_t,void*));
size_t surgescript_heap_size(const surgescript_heap_t* heap);
bool surgescript_heap_validaddress(const surgescript_heap_t* heap, surgescript_heapptr_t ptr);
size_t surgescript_heap_memspent(const surgescript_heap_t* heap);

#endif
