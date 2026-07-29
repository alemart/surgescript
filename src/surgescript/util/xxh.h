/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2026 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * util/xxh.h
 * XXH macro for xxhash
 */

#ifndef _XXH_H
#define _XXH_H

#define XXH_INLINE_ALL
#include "../third_party/xxhash.h"

#if defined(__arm__) || ((defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)) && !(defined(__x86_64__) || defined(_M_X64)))
#define XXH(input, len, seed) (XXH32_hash_t)(XXH3_64bits_withSeed((input), (len), (seed))) /* just discard the higher bits; XXH32_hash_t is unsigned */
typedef XXH32_hash_t xxhash_t;
#else
#define XXH(input, len, seed) (XXH3_64bits_withSeed((input), (len), (seed)) & UINT64_C(0xFFFFFFFF)) /* we set the higher 32 bits to zero before computing signatures */
typedef XXH64_hash_t xxhash_t;
#endif

#endif
