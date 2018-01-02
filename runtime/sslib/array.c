/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017-2018  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/sslib/array.c
 * SurgeScript Arrays
 */

#include <string.h>
#include "../vm.h"
#include "../object_manager.h"
#include "../heap.h"
#include "../object.h"
#include "../../util/ssarray.h"
#include "../../util/util.h"


/* private stuff */

/* Array */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getlength(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_get(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_set(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_push(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_pop(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_shift(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_unshift(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_sort(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_reverse(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_shuffle(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_indexof(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_iterator(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* ArrayIterator */
static surgescript_var_t* fun_it_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_next(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_hasnext(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_it_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* sorting functions */
typedef int (*surgescript_sortcmp_t)(surgescript_object_t* object, const surgescript_var_t*, const surgescript_var_t*);
static int default_sort_function(surgescript_object_t* object, const surgescript_var_t* a, const surgescript_var_t* b);
static int custom_sort_function(surgescript_object_t* object, const surgescript_var_t* a, const surgescript_var_t* b);

/* utilities */
#define ORDINAL(j)              (((j) == 1) ? "st" : (((j) == 2) ? "nd" : (((j) == 3) ? "rd" : "th")))
#define ARRAY_LENGTH(heap)      ((int)surgescript_var_get_number(surgescript_heap_at((heap), LENGTH_ADDR)))
static void quicksort(surgescript_heap_t* heap, surgescript_heapptr_t begin, surgescript_heapptr_t end, surgescript_sortcmp_t compare, surgescript_object_t* compare_object);
static inline surgescript_heapptr_t partition(surgescript_heap_t* heap, surgescript_heapptr_t begin, surgescript_heapptr_t end, surgescript_sortcmp_t compare, surgescript_object_t* compare_object);
static inline surgescript_var_t* med3(surgescript_var_t* a, surgescript_var_t* b, surgescript_var_t* c);
static const surgescript_heapptr_t LENGTH_ADDR = 0; /* the length of the array is allocated on the first address */
static const surgescript_heapptr_t BASE_ADDR = 1; /* array elements come later */
static const surgescript_heapptr_t IT_LENGTH_ADDR = 0;
static const surgescript_heapptr_t IT_COUNTER_ADDR = 1;


/*
 * surgescript_sslib_register_array()
 * Register the methods of the SurgeScript Arrays
 */
void surgescript_sslib_register_array(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Array", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Array", "destructor", fun_destructor, 0);
    surgescript_vm_bind(vm, "Array", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Array", "getLength", fun_getlength, 0);
    surgescript_vm_bind(vm, "Array", "get", fun_get, 1);
    surgescript_vm_bind(vm, "Array", "set", fun_set, 2);
    surgescript_vm_bind(vm, "Array", "push", fun_push, 1);
    surgescript_vm_bind(vm, "Array", "pop", fun_pop, 0);
    surgescript_vm_bind(vm, "Array", "shift", fun_shift, 0);
    surgescript_vm_bind(vm, "Array", "unshift", fun_unshift, 1);
    surgescript_vm_bind(vm, "Array", "sort", fun_sort, 1);
    surgescript_vm_bind(vm, "Array", "reverse", fun_reverse, 0);
    surgescript_vm_bind(vm, "Array", "shuffle", fun_shuffle, 0);
    surgescript_vm_bind(vm, "Array", "indexOf", fun_indexof, 1);
    surgescript_vm_bind(vm, "Array", "iterator", fun_iterator, 0);
    surgescript_vm_bind(vm, "Array", "toString", fun_tostring, 0);

    surgescript_vm_bind(vm, "ArrayIterator", "constructor", fun_it_constructor, 0);
    surgescript_vm_bind(vm, "ArrayIterator", "state:main", fun_it_main, 0);
    surgescript_vm_bind(vm, "ArrayIterator", "next", fun_it_next, 0);
    surgescript_vm_bind(vm, "ArrayIterator", "hasNext", fun_it_hasnext, 0);
    surgescript_vm_bind(vm, "ArrayIterator", "toString", fun_it_tostring, 0);
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
    /* the heap gets freed anyway, so why bother? */
    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* returns the length of the array */
surgescript_var_t* fun_getlength(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
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
        return NULL;
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
    return NULL; /*surgescript_var_clone(value);*/ /* the C expression (arr[i] = value) returns value */
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

/* reverses the array. Returns the reversed array. */
surgescript_var_t* fun_reverse(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    int length = ARRAY_LENGTH(heap);

    for(int i = 0; i < length / 2; i++) {
        surgescript_var_t* a = surgescript_heap_at(heap, BASE_ADDR + i);
        surgescript_var_t* b = surgescript_heap_at(heap, BASE_ADDR + (length - 1 - i));
        surgescript_var_swap(a, b);
    }

    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
}

/* sorts the array. Returns the sorted array */
surgescript_var_t* fun_sort(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_sortcmp_t compare = surgescript_var_is_null(param[0]) ? default_sort_function : custom_sort_function;
    surgescript_object_t* compare_object = (compare == custom_sort_function) ? surgescript_objectmanager_get(manager, surgescript_var_get_objecthandle(param[0])) : NULL;

    quicksort(heap, BASE_ADDR, BASE_ADDR + ARRAY_LENGTH(heap) - 1, compare, compare_object);

    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
}

/* shuffles the array. Returns the shuffled array. */
surgescript_var_t* fun_shuffle(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    int length = ARRAY_LENGTH(heap);

    for(int i = length; i > 0; i--) {
        surgescript_var_t* a = surgescript_heap_at(heap, BASE_ADDR + (i - 1));
        surgescript_var_t* b = surgescript_heap_at(heap, BASE_ADDR + (rand() % i));
        surgescript_var_swap(a, b);
    }

    return surgescript_var_set_objecthandle(surgescript_var_create(), surgescript_object_handle(object));
}

/* finds the first i such that array[i] == param[0], or -1 if there is no such a match */
surgescript_var_t* fun_indexof(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* haystack = surgescript_object_heap(object);
    const surgescript_var_t* needle = param[0];
    int length = ARRAY_LENGTH(haystack);

    for(int i = 0; i < length; i++) {
        surgescript_var_t* element = surgescript_heap_at(haystack, BASE_ADDR + i);
        if(surgescript_var_compare(element, needle) == 0)
            return surgescript_var_set_number(surgescript_var_create(), i);
    }

    return surgescript_var_set_number(surgescript_var_create(), -1);
}

/* returns an ArrayIterator of this array */
surgescript_var_t* fun_iterator(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t it_handle = surgescript_objectmanager_spawn(manager, surgescript_object_handle(object), "ArrayIterator", NULL);
    return surgescript_var_set_objecthandle(surgescript_var_create(), it_handle);
}

/* converts to string */
surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    SSARRAY(char, sb); /* string builder */
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_var_t* stringified_array = surgescript_var_create();
    surgescript_heap_t* heap = surgescript_object_heap(object);
    int length = ARRAY_LENGTH(heap);
    static int depth = 0;
    bool can_descend = (++depth < 16); /* handle circular links */

    /* helper macro */
    #define WRITE_ELEMENT(element, write_as_quoted_string) \
        do { \
            char* value = surgescript_var_get_string((element), can_descend ? manager : NULL); \
            if(write_as_quoted_string) { \
                ssarray_push(sb, '"'); \
                for(const char* p = value; *p; p++) { \
                    switch(*p) { \
                        case '\n': ssarray_push(sb, '\\'); ssarray_push(sb, 'n'); break; \
                        case '\r': ssarray_push(sb, '\\'); ssarray_push(sb, 'r'); break; \
                        case '\t': ssarray_push(sb, '\\'); ssarray_push(sb, 't'); break; \
                        case '\f': ssarray_push(sb, '\\'); ssarray_push(sb, 'f'); break; \
                        case '\v': ssarray_push(sb, '\\'); ssarray_push(sb, 'v'); break; \
                        case '\b': ssarray_push(sb, '\\'); ssarray_push(sb, 'b'); break; \
                        case '\"': ssarray_push(sb, '\\'); ssarray_push(sb, '"'); break; \
                        default: ssarray_push(sb, *p); break; \
                    } \
                } \
                ssarray_push(sb, '"'); \
            } \
            else { \
                for(const char* p = value; *p; p++) \
                    ssarray_push(sb, *p); \
            } \
            ssfree(value); \
        } while(0) \

    /* start sb */
    ssarray_init(sb);
    ssarray_push(sb, '[');

    /* for each element */
    for(int i = 0; i < length; i++) {
        surgescript_var_t* element = surgescript_heap_at(heap, BASE_ADDR + i);

        /* add whitespace */
        ssarray_push(sb, ' ');

        /* write element */
        if(!surgescript_var_typecheck(element, surgescript_var_type2code("object"))) {
            surgescript_objecthandle_t handle = surgescript_var_get_objecthandle(element);
            surgescript_object_t* object = surgescript_objectmanager_get(manager, handle);
            if(strcmp(surgescript_object_name(object), "Array") != 0 && strcmp(surgescript_object_name(object), "Dictionary") != 0 && depth < 16) {
                if(can_descend)
                    surgescript_object_call_function(object, "toString", NULL, 0, element);
                WRITE_ELEMENT(element, strcmp(surgescript_var_fast_get_string(element), "[object]"));
            }
            else
                WRITE_ELEMENT(element, false);
        }
        else
            WRITE_ELEMENT(element, !surgescript_var_typecheck(element, surgescript_var_type2code("string")));

        /* add separator */
        ssarray_push(sb, i < length - 1 ? ',' : ' ');
    }

    /* convert sb to string */
    ssarray_push(sb, ']');
    ssarray_push(sb, '\0');
    surgescript_var_set_string(stringified_array, sb);
    ssarray_release(sb);
    --depth;

    /* done! */
    return stringified_array;
}




/* ArrayIterator */

surgescript_var_t* fun_it_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t parent_handle = surgescript_object_parent(object);
    surgescript_object_t* parent = surgescript_objectmanager_get(manager, parent_handle);
    surgescript_heap_t* parent_heap = surgescript_object_heap(parent);
    const char* parent_name = surgescript_object_name(parent);

    ssassert(IT_LENGTH_ADDR == surgescript_heap_malloc(heap));
    ssassert(IT_COUNTER_ADDR == surgescript_heap_malloc(heap));

    surgescript_var_set_number(surgescript_heap_at(heap, IT_LENGTH_ADDR), 0.0f);
    surgescript_var_set_number(surgescript_heap_at(heap, IT_COUNTER_ADDR), 0.0f);
    if(strcmp(parent_name, "Array") == 0)
        surgescript_var_set_number(surgescript_heap_at(heap, IT_LENGTH_ADDR), ARRAY_LENGTH(parent_heap));

    return NULL;
}

surgescript_var_t* fun_it_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

surgescript_var_t* fun_it_next(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    int cnt = surgescript_var_get_number(surgescript_heap_at(heap, IT_COUNTER_ADDR));
    int len = surgescript_var_get_number(surgescript_heap_at(heap, IT_LENGTH_ADDR));
    
    if(cnt < len) {
        surgescript_objectmanager_t* manager = surgescript_object_manager(object);
        surgescript_objecthandle_t parent_handle = surgescript_object_parent(object);
        surgescript_object_t* parent = surgescript_objectmanager_get(manager, parent_handle);
        surgescript_heap_t* parent_heap = surgescript_object_heap(parent);
        surgescript_var_t* element = surgescript_var_clone(surgescript_heap_at(parent_heap, BASE_ADDR + cnt));
        surgescript_var_set_number(surgescript_heap_at(heap, IT_COUNTER_ADDR), cnt + 1);
        return element;
    }

    return NULL;
}

surgescript_var_t* fun_it_hasnext(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_heap_t* heap = surgescript_object_heap(object);
    int cnt = surgescript_var_get_number(surgescript_heap_at(heap, IT_COUNTER_ADDR));
    int len = surgescript_var_get_number(surgescript_heap_at(heap, IT_LENGTH_ADDR));
    return surgescript_var_set_bool(surgescript_var_create(), cnt < len);
}

surgescript_var_t* fun_it_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return surgescript_var_set_string(surgescript_var_create(), "[ArrayIterator]");
}


/* utilities */

/* quicksort algorithm: sorts heap[begin .. end] */
void quicksort(surgescript_heap_t* heap, surgescript_heapptr_t begin, surgescript_heapptr_t end, surgescript_sortcmp_t compare, surgescript_object_t* compare_object)
{
    if(begin < end) {
        surgescript_heapptr_t p = partition(heap, begin, end, compare, compare_object);
        quicksort(heap, begin, p-1, compare, compare_object);
        quicksort(heap, p+1, end, compare, compare_object);
    }
}

/* returns ptr such that heap[begin .. ptr-1] <= heap[ptr] < heap[ptr+1 .. end], where begin <= end */
surgescript_heapptr_t partition(surgescript_heap_t* heap, surgescript_heapptr_t begin, surgescript_heapptr_t end, surgescript_sortcmp_t compare, surgescript_object_t* compare_object)
{
    surgescript_var_t* pivot = surgescript_heap_at(heap, end);
    surgescript_heapptr_t p = begin;

    surgescript_var_swap(pivot, med3(surgescript_heap_at(heap, begin), surgescript_heap_at(heap, begin + (end-begin)/2), pivot));
    for(surgescript_heapptr_t i = begin; i <= end - 1; i++) {
        if(compare(compare_object, surgescript_heap_at(heap, i), pivot) <= 0) {
            surgescript_var_swap(surgescript_heap_at(heap, i), surgescript_heap_at(heap, p));
            p++;
        }
    }

    surgescript_var_swap(surgescript_heap_at(heap, p), pivot);
    return p;
}

/* returns the median of 3 variables */
surgescript_var_t* med3(surgescript_var_t* a, surgescript_var_t* b, surgescript_var_t* c)
{
    int ab = surgescript_var_compare(a, b);
    int bc = surgescript_var_compare(b, c);
    int ac = surgescript_var_compare(a, c);

    if(ab >= 0 && ac >= 0) /* a = max(a, b, c) */
        return bc >= 0 ? b : c;
    else if(ab <= 0 && bc >= 0) /* b = max(a, b, c) */
        return ac >= 0 ? a : c;
    else /* c = max(a, b, c) */
        return ab >= 0 ? a : b;
}

/* default sort function */
int default_sort_function(surgescript_object_t* object, const surgescript_var_t* a, const surgescript_var_t* b)
{
    return surgescript_var_compare(a, b);
}

/* custom sort function (calls an object) */
int custom_sort_function(surgescript_object_t* object, const surgescript_var_t* a, const surgescript_var_t* b)
{
    const surgescript_var_t* param[] = { a, b };
    float return_value = 0;

    surgescript_var_t* ret = surgescript_var_create();
    surgescript_object_call_function(object, "call", param, 2, ret);
    return_value = surgescript_var_get_number(ret);
    surgescript_var_destroy(ret);

    return ((return_value < 0) ? -1 : (return_value > 0 ? 1 : 0));
}