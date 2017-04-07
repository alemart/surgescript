/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/system.c
 * SurgeScript standard library: System object
 */

#include <stdio.h>
#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_system()
 * Register methods
 */
void surgescript_sslib_register_system(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "System", "__constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "System", "state:main", fun_main, 0);
}



/* my functions */

/* register some built-in objects */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    static const char* builtin[] = { "String", "Number", "Boolean" };
    static size_t length = sizeof(builtin) / sizeof(const char*);
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objectmanager_handle_t me = surgescript_object_handle(object);
    surgescript_heap_t* heap = surgescript_object_heap(object);

    for(int i = 0; i < length; i++) {
        surgescript_var_t* mem = surgescript_heap_at(heap, surgescript_heap_malloc(heap));
        surgescript_var_set_objecthandle(mem, surgescript_objectmanager_spawn(manager, me, builtin[i], NULL));
    }

    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}