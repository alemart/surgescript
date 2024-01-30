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
 * util/ssarray.h
 * SurgeScript expandable arrays
 */

#ifndef _SURGESCRIPT_SSARRAY_H
#define _SURGESCRIPT_SSARRAY_H

#include "util.h"

/*
 * SSARRAY()
 * declares an array of a certain type
 */
#define SSARRAY(type, arr)                    type* arr; size_t arr##_len, arr##_cap

/*
 * ssarray_init()
 * initializes the array
 */
#define ssarray_init(arr)                     ssarray_init_ex(arr, 0)

/*
 * ssarray_init_ex()
 * initializes the array with a pre-defined initial capacity
 */
#define ssarray_init_ex(arr, cap)             (arr##_len = 0, arr##_cap = ((cap) > 0 ? (cap) : 4), arr = ssmalloc(arr##_cap * sizeof(*(arr))))

/*
 * ssarray_release()
 * releases the array
 */
#define ssarray_release(arr)                  (arr##_len = arr##_cap = 0, arr = (arr ? ssfree(arr) : NULL))

/*
 * ssarray_push()
 * pushes element 'x' into the array, returning the new length of the array
 */
#define ssarray_push(arr, x)                  \
    (*(((arr##_len >= arr##_cap) ? (arr = ssrealloc(arr, (arr##_cap *= 2) * sizeof(*(arr)))) : arr) + (arr##_len)) = (x), ++arr##_len)

/*
 * ssarray_pop()
 * pops the last element from the array, writing its contents to variable dst
 */
#define ssarray_pop(arr, dst)                 \
    do { if(arr##_len > 0) dst = arr[--arr##_len]; } while(0)
    
/*
 * ssarray_remove()
 * removes the index-th element from the array
 */
 #define ssarray_remove(arr, index)           \
    do { if((index) < arr##_len && (index) >= 0) { memmove((arr) + (index), (arr) + ((index) + 1), (arr##_len - ((index) + 1)) * sizeof(*(arr))); arr##_len--; } } while(0)

/*
 * ssarray_length()
 * returns the length of the array
 */
#define ssarray_length(arr)                   (arr##_len)

/*
 * ssarray_reset()
 * sets the length of the array to zero, without freeing anything
 */
#define ssarray_reset(arr)                    (arr##_len = 0)

#endif
