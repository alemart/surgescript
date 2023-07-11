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

#include <stdbool.h>
#include "perfect_hash.h"
#include "util.h"

/* A set of integers is implemented with a simple binary search tree */
typedef struct treenode_t treenode_t;
struct treenode_t
{
    surgescript_perfecthashkey_t value;
    const char* string; /* just for reporting */
    treenode_t* left;
    treenode_t* right;
};

static treenode_t* treenode_create(surgescript_perfecthashkey_t value, const char* string);
static treenode_t* treenode_destroy(treenode_t* node);
static treenode_t* treenode_insert(treenode_t* node, surgescript_perfecthashkey_t value, const char* string);

/*
 * surgescript_perfecthash_find_seed()
 * Find a seed such that hash_fn() is a perfect hash function for key[]
 */
surgescript_perfecthashseed_t surgescript_perfecthash_find_seed(surgescript_perfecthashfunction_t hash_fn, const char** key, size_t key_count)
{
    /*

    We generate a seed for a perfect hash function using brute force. This very
    simple implementation is expected to finish in a single iteration if the
    provided hash function has "good" distribution and if the input set of
    strings is "small" enough (how small?)

    If any of the possible 2^b hashes (b = 32 bits) can be generated with
    (approximately) equal probability, then the probability of collision is
    very small to begin with (key_count << 2^b). We're most certainly getting a
    perfect hash function in the first iteration.

    This method will fail if there are repeated strings in key[].

    */
    surgescript_perfecthashseed_t seed = 0;
    surgescript_perfecthashkey_t hash = 0;
    treenode_t* set = NULL; /* a set of hash keys */
    bool failed = false;
    int iterations = 1;
    const int MAX_ITERATIONS = 16; /* really?! */

    /* if there are no strings, any seed will work */
    if(key_count == 0)
        return seed;

    /* pick a seed at random and check if there is a collision */
    sslog("Finding a perfect hash function for a set of %d strings...", key_count);
    do {
        failed = false;
        seed = surgescript_util_random64() & UINT64_C(0xFFFFFFFF);
        sslog("Trying seed 0x%lx...", seed);

        hash = hash_fn(key[0], seed);
        set = treenode_create(hash, key[0]);

        for(size_t i = 1; i < key_count && !failed; i++) {
            hash = hash_fn(key[i], seed);
            failed = (NULL == treenode_insert(set, hash, key[i]));
        }

        treenode_destroy(set);
    } while(failed && ++iterations <= MAX_ITERATIONS);

    /* failed? */
    if(failed)
        ssfatal("Can't find a perfect hash function for a set of %d strings", key_count);

    /* done! */
    sslog("Perfect!", seed);
    return seed;
}



/*
 * binary search tree for a set of integers
 */

treenode_t* treenode_create(surgescript_perfecthashkey_t value, const char* string)
{
    treenode_t* node = ssmalloc(sizeof *node);

    node->value = value;
    node->string = string;
    node->left = NULL;
    node->right = NULL;

    return node;
}

treenode_t* treenode_destroy(treenode_t* node)
{
    if(node == NULL)
        return NULL;
        
    treenode_destroy(node->left);
    treenode_destroy(node->right);

    return ssfree(node);
}

treenode_t* treenode_insert(treenode_t* node, surgescript_perfecthashkey_t value, const char* string)
{
    treenode_t* parent = NULL;

    while(node != NULL) {
        parent = node;

        if(value < node->value) {
            node = node->left;
        }
        else if(value > node->value) {
            node = node->right;
        }
        else {
            sslog("Found a collision between \"%s\" and \"%s\"", node->string, string);
            return NULL; /* value is already in the set */
        }
    }

    treenode_t* new_node = treenode_create(value, string);
    if(parent == NULL)
        return new_node;
    else if(value < parent->value)
        return (parent->left = new_node);
    else
        return (parent->right = new_node);
}