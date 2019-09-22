/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2019 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * util/fasthash.h
 * A fast hash table with integer keys and linear probing
 */
#ifndef _FASTHASH_H
#define _FASTHASH_H

#include <stdint.h>
#include <stdbool.h>

/* Inline usage:

#define FASTHASH_INLINE
#include "fasthash.h"

No need to compile fasthash.c separately */
#if defined(FASTHASH_INLINE)
#define FASTHASH_API static inline
#else
#define FASTHASH_API
#endif

typedef struct fasthash_t fasthash_t;
FASTHASH_API fasthash_t* fasthash_create(void (*element_destructor)(void*), size_t lg2_cap);
FASTHASH_API fasthash_t* fasthash_destroy(fasthash_t* hashtable);
FASTHASH_API void* fasthash_get(fasthash_t* hashtable, uint64_t key);
FASTHASH_API void fasthash_put(fasthash_t* hashtable, uint64_t key, void* value);
FASTHASH_API bool fasthash_delete(fasthash_t* hashtable, uint64_t key);
FASTHASH_API void* fasthash_find(fasthash_t* hashtable, bool (*predicate)(const void*,void*), void* data);

#if defined(FASTHASH_INLINE)
#include "fasthash.c"
#endif

#endif