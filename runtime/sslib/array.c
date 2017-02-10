/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/array.c
 * SurgeScript Arrays
 */

#include "../vm.h"
#include "../heap.h"
#include "../../util/util.h"


/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_length(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_push(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_pop(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_shift(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_unshift(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_sort(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_reverse(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* utilities */
#define ORDINAL(j)              (((j) == 1) ? "st" : (((j) == 2) ? "nd" : (((j) == 3) ? "rd" : "th")))
#define ARRAY_LENGTH(heap)      ((int)surgescript_var_get_number(surgescript_heap_at((heap), LENGTH_ADDR)))
#define SWAP(a, b, tmp)         do { surgescript_var_copy((tmp), (a)); surgescript_var_copy((a), (b)); surgescript_var_copy((b), (tmp)); } while(0)
static const surgescript_heapptr_t LENGTH_ADDR = 0; /* the length of the array is allocated on the first address */
static const surgescript_heapptr_t BASE_ADDR = 1;   /* array elements come later */



/*
 * surgescript_sslib_register_array()
 * Register the methods of the SurgeScript Arrays
 */
void surgescript_sslib_register_array()
{
    ;
}


/* my functions */

/* array constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* since we don't ever free() anything from the heap (except the last cell),
       memory cells are allocated contiguously */
    surgescript_heap_t* heap = surgescript_object_heap(object);

    surgescript_heapptr_t length_addr = surgescript_heap_malloc(heap);
    surgescript_var_set_number(surgescript_heap_at(heap, length_addr), 0);
    ssassert(length_addr == LENGTH_ADDR);

    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
}

/* destructor */
surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

/* main state: does nothing */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

/* returns the length of the array */
surgescript_var_t* fun_length(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    return surgescript_var_clone(surgescript_heap_at(heap, LENGTH_ADDR));
}

/* gets i-th element of the array (indexes are 0-based) */
surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    int index = surgescript_var_get_number(param[0]);

    if(index >= 0 && index < ARRAY_LENGTH(heap))
        return surgescript_var_clone(surgescript_heap_at(heap, BASE_ADDR + index));

    ssfatal("Can't get %d-%s element of the array: the index is out of bounds.", index, ORDINAL(index));
    return NULL;
}

/* sets the i-th element of the array */
surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    int index = surgescript_var_get_number(param[0]);
    int length = ARRAY_LENGTH(heap);
    const surgescript_var_t* value = param[1];

    /* sanity check & leak prevention */
    if(index < 0 || index >= length + 1024) {
        ssfatal("Can't set %d-%s element of the array: the index is out of bounds.", index, ORDINAL(index));
        return surgescript_var_clone(value);
    }

    /* create memory addresses as needed */
    while(index >= length) {
        surgescript_heapptr_t ptr = surgescript_heap_malloc(heap); /* fast */
        surgescript_var_set_number(surgescript_heap_at(heap, LENGTH_ADDR), ++length);
        ssassert(ptr == BASE_ADDR + (length - 1));
    }

    /* set the value to the correct address */
    surgescript_var_copy(surgescript_heap_at(heap, BASE_ADDR + index), value);

    /* done! */
    return surgescript_var_clone(value); /* the C expression (arr[i] = value) returns value */
}

/* pushes a new element into the last position of the array */
surgescript_var_t* fun_push(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    const surgescript_var_t* value = param[0];
    int length = ARRAY_LENGTH(heap);

    surgescript_heapptr_t ptr = surgescript_heap_malloc(heap);
    surgescript_var_copy(surgescript_heap_at(heap, ptr), value);
    surgescript_var_set_number(surgescript_heap_at(heap, LENGTH_ADDR), ++length);
    ssassert(ptr == BASE_ADDR + (length - 1));

    return NULL;
}

/* pops the last element from the array */
surgescript_var_t* fun_pop(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    int length = ARRAY_LENGTH(heap);

    if(length > 0) {
        surgescript_var_t* value = surgescript_var_clone(surgescript_heap_at(heap, BASE_ADDR + (length - 1)));
        surgescript_var_set_number(surgescript_heap_at(heap, LENGTH_ADDR), length - 1);
        surgescript_heap_free(heap, BASE_ADDR + (length - 1));
        return value;
    }

    return NULL;
}

/* removes (and returns) the first element and shifts all others to a lower index */
surgescript_var_t* fun_shift(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    int length = ARRAY_LENGTH(heap);

    if(length > 0) {
        surgescript_var_t* value = surgescript_var_clone(surgescript_heap_at(heap, BASE_ADDR + 0));

        for(int i = 0; i < length - 1; i++)
            surgescript_var_copy(surgescript_heap_at(heap, BASE_ADDR + i), surgescript_heap_at(heap, BASE_ADDR + (i + 1)));

        surgescript_var_set_number(surgescript_heap_at(heap, LENGTH_ADDR), length - 1);
        surgescript_heap_free(heap, BASE_ADDR + (length - 1));
        return value;
    }

    return NULL;
}

/* adds an element to the beginning of the array and shifts all others to a higher index */
surgescript_var_t* fun_unshift(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    const surgescript_var_t* value = param[0];
    int length = ARRAY_LENGTH(heap);

    surgescript_heapptr_t ptr = surgescript_heap_malloc(heap);
    surgescript_var_set_number(surgescript_heap_at(heap, LENGTH_ADDR), ++length);
    ssassert(ptr == BASE_ADDR + (length - 1));

    for(int i = length - 1; i > 0; i--)
        surgescript_var_copy(surgescript_heap_at(heap, BASE_ADDR + i), surgescript_heap_at(heap, BASE_ADDR + (i - 1)));
    surgescript_var_copy(surgescript_heap_at(heap, BASE_ADDR + 0), value);

    return NULL;
}

/* reverses the array */
surgescript_var_t* fun_reverse(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_var_t* tmp = surgescript_var_create();
    int length = ARRAY_LENGTH(heap);

    for(int i = 0; i < length / 2; i++) {
        surgescript_var_t* a = surgescript_heap_at(heap, BASE_ADDR + i);
        surgescript_var_t* b = surgescript_heap_at(heap, BASE_ADDR + (length - 1 - i));
        SWAP(a, b, tmp);
    }

    surgescript_var_destroy(tmp);
    return NULL;
}

surgescript_var_t* fun_sort(surgescript_object_t* object, const surgescript_var_t** param, int num_params);