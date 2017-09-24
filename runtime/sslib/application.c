/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/application.c
 * SurgeScript standard library: routines for the Application object
 */

#include <stdio.h>
#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_exit(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_crash(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getsession(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_application()
 * Register methods
 */
void surgescript_sslib_register_application(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Application", "exit", fun_exit, 0);
    surgescript_vm_bind(vm, "Application", "crash", fun_crash, 1);
    surgescript_vm_bind(vm, "Application", "destroy", fun_destroy, 0); /* overloads Object's destroy() */
    surgescript_vm_bind(vm, "Application", "getSession", fun_getsession, 0);
}



/* my functions */

/* exits the app */
surgescript_var_t* fun_exit(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* this will destroy the root object and stop the VM */
    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_objecthandle_t root_handle = surgescript_objectmanager_root(manager);
    surgescript_object_t* root = surgescript_objectmanager_get(manager, root_handle);
    surgescript_object_call_function(root, "exit", NULL, 0, NULL);
    surgescript_object_kill(object);
    return NULL;
}

/* destroys the app */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* this is the same as exiting the app */
    return fun_exit(object, param, num_params);
}

/* crashes the app with a message */
surgescript_var_t* fun_crash(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* text = surgescript_var_get_string(param[0]);
    ssfatal("Script Error: %s", text); /* change ssfatal to something else? */
    ssfree(text);
    return fun_exit(object, NULL, 0);
}

/* session variables */
surgescript_var_t* fun_getsession(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    static const char* SESSION_OBJECT = "Dictionary";
    static const unsigned long SESSION_MAGIC = 0x5E55107;
    surgescript_heap_t* heap = surgescript_object_heap(object);
    surgescript_heapptr_t ptr = surgescript_heap_size(heap);
    surgescript_objecthandle_t session = 0;

    /* stored previously? */
    while(ptr--) {
        if(surgescript_heap_validaddress(heap, ptr)) {
            if(surgescript_var_get_rawbits(surgescript_heap_at(heap, ptr)) == SESSION_MAGIC) {
                if(surgescript_heap_validaddress(heap, 1 + ptr)) {
                    /* validate */
                    surgescript_objectmanager_t* manager = surgescript_object_manager(object);
                    surgescript_var_t* candidate = surgescript_heap_at(heap, 1 + ptr);
                    session = surgescript_var_get_objecthandle(candidate);
                    if(surgescript_objectmanager_exists(manager, session))
                        return surgescript_var_clone(candidate);
                }
            }
        }
    }

    /* lazy spawn */
    session = surgescript_objectmanager_spawn(surgescript_object_manager(object), surgescript_object_handle(object), SESSION_OBJECT, NULL);
    surgescript_var_set_rawbits(surgescript_heap_at(heap, surgescript_heap_malloc(heap)), SESSION_MAGIC);
    surgescript_var_set_objecthandle(surgescript_heap_at(heap, surgescript_heap_malloc(heap)), session);

    /* done! */
    return surgescript_var_set_objecthandle(surgescript_var_create(), session);
}