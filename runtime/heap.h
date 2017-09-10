/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016-2017  Alexandre Martins <alemartf(at)gmail(dot)com>
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
struct surgescript_var_t* surgescript_heap_at(surgescript_heap_t* heap, surgescript_heapptr_t ptr);
void surgescript_heap_scan_objects(surgescript_heap_t* heap, void* userdata, bool (*callback)(unsigned,void*));
size_t surgescript_heap_size(const surgescript_heap_t* heap);

#endif
