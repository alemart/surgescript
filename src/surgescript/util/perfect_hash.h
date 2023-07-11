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
 * util/perfect_hash.h
 * Perfect hashing utility
 */

#ifndef _SURGESCRIPT_PERFECTHASH_H
#define _SURGESCRIPT_PERFECTHASH_H

#include <stdint.h>
#include <stdlib.h>

/* We use 32-bit hashes */
typedef uint32_t surgescript_perfecthashkey_t;
typedef uint32_t surgescript_perfecthashseed_t;
typedef surgescript_perfecthashkey_t (*surgescript_perfecthashfunction_t)(const char*,surgescript_perfecthashseed_t);

/* given a family H = { h_s(x) | s } of hash functions and a set K of strings,
   find a seed value s such that h_s(x) is a perfect hash function for K (i.e., no coliisions).
   Note: you must ensure that there are no repeated strings in the key[] array. */
surgescript_perfecthashseed_t surgescript_perfecthash_find_seed(surgescript_perfecthashfunction_t hash_fn, const char** key, size_t key_count);

#endif