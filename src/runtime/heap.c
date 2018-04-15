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
 * runtime/heap.c
 * SurgeScript heap
 */

#include "heap.h"
#include "variable.h"
#include "../util/util.h"

/* constants */
static const size_t SSHEAP_INITIAL_SIZE = 8;
static const size_t SSHEAP_MAX_SIZE = 10 * 1024 * 1024; /* 10M cells max */

/* heap structure */
struct surgescript_heap_t
{
    size_t size;                /* size of the heap */
    surgescript_heapptr_t ptr;  /* allocation pointer */
    surgescript_var_t** mem;    /* data memory */
};


/* -------------------------------
 * public methods
 * ------------------------------- */

/*
 * surgescript_heap_create()
 * Creates a new heap
 */
surgescript_heap_t* surgescript_heap_create()
{
    surgescript_heap_t* heap = ssmalloc(sizeof *heap);
    size_t size = SSHEAP_INITIAL_SIZE;

    heap->mem = ssmalloc(size * sizeof(*(heap->mem)));
    heap->size = size;
    heap->ptr = size;
    while(heap->ptr)
        heap->mem[--heap->ptr] = NULL;

    return heap;
}

/*
 * surgescript_heap_destroy()
 * Destroys an existing heap
 */
surgescript_heap_t* surgescript_heap_destroy(surgescript_heap_t* heap)
{
    for(heap->ptr = 0; heap->ptr < heap->size; heap->ptr++) {
        if(heap->mem[heap->ptr] != NULL)
            surgescript_var_destroy(heap->mem[heap->ptr]);
    }

    ssfree(heap->mem);
    return ssfree(heap);
}

/*
 * surgescript_heap_malloc()
 * Allocates a memory cell
 */
surgescript_heapptr_t surgescript_heap_malloc(surgescript_heap_t* heap)
{
    for(; heap->ptr < heap->size; heap->ptr++) {
        if(heap->mem[heap->ptr] == NULL) {
            heap->mem[heap->ptr] = surgescript_var_create();
            return heap->ptr;
        }
    }

    if(heap->size * 2 >= SSHEAP_MAX_SIZE) { /* just in case... */
        ssfatal("surgescript_heap_malloc(): max size exceeded.");
        return heap->size - 1;
    }


    if(heap->size * 2 >= 256)
        sslog("surgescript_heap_malloc(): resizing heap to %d cells.", heap->size * 2);
    heap->mem = ssrealloc(heap->mem, (heap->size * 2) * sizeof(*(heap->mem)));
    while(heap->ptr)
        heap->mem[heap->size + --(heap->ptr)] = NULL;
    heap->size *= 2;
    return surgescript_heap_malloc(heap);
}

/*
 * surgescript_heap_free()
 * Deallocates the memory cell pointed by ptr
 */
surgescript_heapptr_t surgescript_heap_free(surgescript_heap_t* heap, surgescript_heapptr_t ptr)
{
    if(ptr >= 0 && ptr < heap->size && heap->mem[ptr] != NULL) {
        heap->mem[ptr] = surgescript_var_destroy(heap->mem[ptr]);
        heap->ptr = ptr;
    }

    return 0;
}

/*
 * surgescript_heap_at()
 * Returns the memory cell pointed by ptr
 */
surgescript_var_t* surgescript_heap_at(const surgescript_heap_t* heap, surgescript_heapptr_t ptr)
{
    if(ptr >= 0 && ptr < heap->size && heap->mem[ptr] != NULL)
        return heap->mem[ptr];

    ssfatal("surgescript_heap_at(0x%X): null pointer exception.", ptr);
    return NULL;
}

/*
 * surgescript_heap_scan_objects()
 * Scans all the objects in the heap, calling callback for each one of them
 */
void surgescript_heap_scan_objects(surgescript_heap_t* heap, void* userdata, bool (*callback)(unsigned,void*))
{
    for(surgescript_heapptr_t ptr = 0; ptr < heap->size; ptr++) {
        if(heap->mem[ptr] != NULL) {
            unsigned handle = surgescript_var_get_objecthandle(heap->mem[ptr]);
            if(handle != 0) { /* if heap->mem[ptr] is an object and not null */
                if(!callback(handle, userdata)) /* if the handle is broken */
                    surgescript_var_set_null(heap->mem[ptr]); /* fix it */
            }
        }
    }
}

/*
 * surgescript_heap_size()
 * The size of the heap
 */
size_t surgescript_heap_size(const surgescript_heap_t* heap)
{
    return heap->size;
}

/*
 * surgescript_heap_validaddress()
 * Is the given address valid within the given heap?
 */
bool surgescript_heap_validaddress(const surgescript_heap_t* heap, surgescript_heapptr_t ptr)
{
    return (ptr >= 0 && ptr < heap->size && heap->mem[ptr] != NULL);
}

/*
 * surgescript_heap_memspent()
 * Memory spent by the heap, in user space (in bytes)
 */
size_t surgescript_heap_memspent(const surgescript_heap_t* heap)
{
    size_t size = 0;

    for(surgescript_heapptr_t ptr = 0; ptr < heap->size; ptr++) {
        if(heap->mem[ptr] != NULL)
            size += surgescript_var_size(heap->mem[ptr]);
    }

    return size;
}