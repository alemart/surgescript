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
 * runtime/sslib/date.c
 * SurgeScript standard library: Date utilities
 */

#define __STDC_WANT_LIB_EXT1__ 1
#include <time.h>
#include <stdlib.h>
#include "../vm.h"
#include "../object.h"
#include "../object_manager.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_spawn(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destroy(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_timezoneoffset(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getunixtime(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getyear(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getmonth(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getday(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_gethour(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getminute(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getsecond(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getweekday(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* my utilities */
static inline struct tm* localtime_x(time_t t, struct tm* result);
static inline int tz_offset(time_t t);

/*
 * surgescript_sslib_register_date()
 * Register methods
 */
void surgescript_sslib_register_date(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Date", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Date", "destructor", fun_destructor, 0);
    surgescript_vm_bind(vm, "Date", "state:main", fun_main, 0);
    surgescript_vm_bind(vm, "Date", "spawn", fun_spawn, 1);
    surgescript_vm_bind(vm, "Date", "destroy", fun_destroy, 0);
    surgescript_vm_bind(vm, "Date", "toString", fun_tostring, 0);
    surgescript_vm_bind(vm, "Date", "timezoneOffset", fun_timezoneoffset, 0);
    surgescript_vm_bind(vm, "Date", "get_unixtime", fun_getunixtime, 0);
    surgescript_vm_bind(vm, "Date", "get_year", fun_getyear, 0);
    surgescript_vm_bind(vm, "Date", "get_month", fun_getmonth, 0);
    surgescript_vm_bind(vm, "Date", "get_day", fun_getday, 0);
    surgescript_vm_bind(vm, "Date", "get_hour", fun_gethour, 0);
    surgescript_vm_bind(vm, "Date", "get_minute", fun_getminute, 0);
    surgescript_vm_bind(vm, "Date", "get_second", fun_getsecond, 0);
    surgescript_vm_bind(vm, "Date", "get_weekday", fun_getweekday, 0);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    struct tm* time_structure = ssmalloc(sizeof *time_structure);
    surgescript_object_set_userdata(object, localtime_x(time(NULL), time_structure));
    return NULL;
}

/* destructor */
surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    struct tm* time_structure = (struct tm*)surgescript_object_userdata(object);
    surgescript_object_set_userdata(object, ssfree(time_structure));
    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    surgescript_object_set_active(object, false); /* we don't need to spend time updating this object */
    return NULL;
}
 
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

/* convert Date to string, according to the ISO 8601 standard */
surgescript_var_t* fun_tostring(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    struct tm* time_structure = (struct tm*)surgescript_object_userdata(object);
    time_t now = time(NULL);
    int len, offset = tz_offset(now);
    char buf[32];

    localtime_x(now, time_structure);
    len = strftime(buf, sizeof(buf) - 1, "%Y-%m-%dT%H:%M:%S", time_structure);
    snprintf(buf + len, sizeof(buf) - len, "%c%02d:%02d", (offset >= 0 ? '+' : '-'), abs(offset) / 60, abs(offset) % 60);

    return surgescript_var_set_string(surgescript_var_create(), buf);
}

/* timezone offset, in minutes */
surgescript_var_t* fun_timezoneoffset(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    time_t now = time(NULL);
    return surgescript_var_set_number(surgescript_var_create(), tz_offset(now));
}

/* returns the number of seconds since Jan 1 1970 00:00 UTC (unixtime) */
surgescript_var_t* fun_getunixtime(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    time_t now = time(NULL);
    return surgescript_var_set_number(surgescript_var_create(), now);
}

/* the current year */
surgescript_var_t* fun_getyear(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    struct tm* time_structure = (struct tm*)surgescript_object_userdata(object);
    localtime_x(time(NULL), time_structure);
    return surgescript_var_set_number(surgescript_var_create(), time_structure->tm_year + 1900);
}

/* current month of the year (1-12) */
surgescript_var_t* fun_getmonth(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    struct tm* time_structure = (struct tm*)surgescript_object_userdata(object);
    localtime_x(time(NULL), time_structure);
    return surgescript_var_set_number(surgescript_var_create(), time_structure->tm_mon + 1);
}

/* day of the month (1-31) */
surgescript_var_t* fun_getday(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    struct tm* time_structure = (struct tm*)surgescript_object_userdata(object);
    localtime_x(time(NULL), time_structure);
    return surgescript_var_set_number(surgescript_var_create(), time_structure->tm_mday);
}

/* hours since midnight (0-23) */
surgescript_var_t* fun_gethour(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    struct tm* time_structure = (struct tm*)surgescript_object_userdata(object);
    localtime_x(time(NULL), time_structure);
    return surgescript_var_set_number(surgescript_var_create(), time_structure->tm_hour);
}

/* minutes after the hour (0-59) */
surgescript_var_t* fun_getminute(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    struct tm* time_structure = (struct tm*)surgescript_object_userdata(object);
    localtime_x(time(NULL), time_structure);
    return surgescript_var_set_number(surgescript_var_create(), time_structure->tm_min);
}

/* seconds after the minute (0-59) */
surgescript_var_t* fun_getsecond(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    struct tm* time_structure = (struct tm*)surgescript_object_userdata(object);
    localtime_x(time(NULL), time_structure);
    return surgescript_var_set_number(surgescript_var_create(), time_structure->tm_sec);
}

/* days since Sunday (0-6) */
surgescript_var_t* fun_getweekday(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    struct tm* time_structure = (struct tm*)surgescript_object_userdata(object);
    localtime_x(time(NULL), time_structure);
    return surgescript_var_set_number(surgescript_var_create(), time_structure->tm_wday);
}



/* localtime_x(): my localtime variant */
struct tm* localtime_x(time_t t, struct tm* result)
{
#if defined(__STDC_LIB_EXT1__)
    return localtime_s(&t, result);
#elif !defined(_WIN32)
    return localtime_r(&t, result);
#else
    *result = *localtime(&t);
    return result;
#endif
}

/* timezone offset (in minutes) from UTC to local timezone.
   E.g., if your timezone is GMT-03:00, it returns -180 */
int tz_offset(time_t t)
{
    /* compute offset */
    struct tm local = *localtime(&t), utc = *gmtime(&t);
    int offset = 60 * (local.tm_hour - utc.tm_hour) + (local.tm_min - utc.tm_min);

    /* found the next day or the end of month? */
    int delta_day = local.tm_mday - utc.tm_mday;
    if(delta_day == 1 || delta_day < -1)
        offset += 1440;
    else if(delta_day == -1 || delta_day > 1)
        offset -= 1440;

    /* done! */
    return offset;
}