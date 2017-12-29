/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * util/temp.c
 * SurgeScript standard library: Temporary Storage Area (holds arrays, dicts, etc.)
 */

#include <time.h>
#include "../vm.h"
#include "../object.h"

/* private stuff */
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/*
 * surgescript_sslib_register_temp()
 * Register methods
 */
void surgescript_sslib_register_temp(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "__Temp", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "__Temp", "destroy", fun_destroy, 0);
}



/* my functions */

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}

/* destroy */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    return NULL;
}