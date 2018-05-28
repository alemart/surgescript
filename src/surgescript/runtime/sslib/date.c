/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2018 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/sslib/date.c
 * SurgeScript standard library: Date utilities
 */

#define __STDC_WANT_LIB_EXT1__ 1
#include <time.h>
#include "../vm.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_time(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* my utilities */
static inline struct tm* localtime_x(const time_t* time, struct tm* result);
static inline struct tm tm_now();

/*
 * surgescript_sslib_register_date()
 * Register methods
 */
void surgescript_sslib_register_date(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Date", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Date", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Date", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Date", "time", fun_time, 0);
}



/* my functions */

/* destroy function */
surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}

/* spawn function */
surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing; you can't spawn children on this object */
    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* do nothing */
    return NULL;
}
 
/* returns the number of seconds since Jan 1 1970 00:00 (unixtime) */
surgescript_var_t* fun_time(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    time_t now = time(NULL);
    return surgescript_var_set_number(surgescript_var_create(), now);
}



/* localtime_x(): my localtime variant */
struct tm* localtime_x(const time_t* time, struct tm* result)
{
#if defined(__STDC_LIB_EXT1__)
    return localtime_s(time, result);
#elif !defined(_WIN32)
    return localtime_r(time, result);
#else
    *result = *localtime(time);
    return result;
#endif
}

/* what time is now? */
struct tm tm_now()
{
    struct tm st;
    time_t now = time(NULL);
    return *localtime_x(&now, &st);
}