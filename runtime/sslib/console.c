/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/sslib/console.c
 * SurgeScript standard library: Console
 */

#include <stdio.h>
#include <string.h>
#include "../vm.h"
#include "../object.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_print(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_write(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_readline(surgescript_object_t* object, const surgescript_var_t** param, int num_params);


/*
 * surgescript_sslib_register_console()
 * Register methods
 */
void surgescript_sslib_register_console(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Console", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Console", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Console", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Console", "print", fun_print, 1);
    surgescript_vm_bind(vm, "Console", "write", fun_write, 1);
    surgescript_vm_bind(vm, "Console", "readline", fun_readline, 0);
}



/* my functions */

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_set_active(object, false); /* we don't need to spend time updating this object */
    return NULL;
}

/* destroy */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing, as system objects cannot be destroyed */
    return NULL;
}

/* spawn */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing; you can't spawn children on this object */
    return NULL;
}

/* print a line to stdout */
surgescript_var_t* fun_print(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* str = surgescript_var_get_string(param[0]);
    puts(str);
    ssfree(str);
    return NULL;
}

/* write a string to stdout */
surgescript_var_t* fun_write(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char* str = surgescript_var_get_string(param[0]);
    fputs(str, stdout);
    ssfree(str);
    return NULL;
}

/* read a line from stdin */
surgescript_var_t* fun_readline(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    char str[1024] = "";
    char* result = fgets(str, sizeof(str) / sizeof(char), stdin);

    if(result != NULL && !ferror(stdin)) {
        if(!feof(stdin)) {
            int len = strlen(result);
            if(len > 0)
                result[len - 1] = 0;
        }
        return surgescript_var_set_string(surgescript_var_create(), result);
    }

    return NULL;
}