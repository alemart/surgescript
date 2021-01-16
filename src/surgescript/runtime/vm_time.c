/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2021  Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/vm_time.c
 * SurgeScript Virtual Machine Time - this is used to count time
 */

#include "vm_time.h"
#include "../util/util.h"

/* VM time */
struct surgescript_vmtime_t {
    uint64_t time; /* in ms */
    uint64_t ticks_at_last_update;
    bool is_paused;
};

/*
 * surgescript_vmtime_create()
 * Create a VM time object
 */
surgescript_vmtime_t* surgescript_vmtime_create()
{
    surgescript_vmtime_t* vmtime = ssmalloc(sizeof *vmtime);

    vmtime->time = 0;
    vmtime->ticks_at_last_update = surgescript_util_gettickcount();
    vmtime->is_paused = false;

    return vmtime;
}

/*
 * surgescript_vmtime_destroy()
 * Destroy a VM time object
 */
surgescript_vmtime_t* surgescript_vmtime_destroy(surgescript_vmtime_t* vmtime)
{
    ssfree(vmtime);
    return NULL;
}

/*
 * surgescript_vmtime_update()
 * Update the VM time object
 */
void surgescript_vmtime_update(surgescript_vmtime_t* vmtime)
{
    uint64_t now = surgescript_util_gettickcount();
    uint64_t delta_time = now > vmtime->ticks_at_last_update ? now - vmtime->ticks_at_last_update : 0;
    vmtime->time += vmtime->is_paused ? 0 : delta_time;
    vmtime->ticks_at_last_update = now;
}

/*
 * surgescript_vmtime_pause()
 * Pause the VM time
 */
void surgescript_vmtime_pause(surgescript_vmtime_t* vmtime)
{
    /* nothing to do */
    if(vmtime->is_paused)
        return;

    /* pause the time */
    vmtime->is_paused = true;
}

/*
 * surgescript_vmtime_resume()
 * Resume the VM time
 */
void surgescript_vmtime_resume(surgescript_vmtime_t* vmtime)
{
    /* nothing to do */
    if(!vmtime->is_paused)
        return;

    /* resume the time */
    vmtime->ticks_at_last_update = surgescript_util_gettickcount();
    vmtime->is_paused = false;
}

/*
 * surgescript_vmtime_time()
 * Get the time, in milliseconds, at the beginning of the current update cycle
 */
uint64_t surgescript_vmtime_time(const surgescript_vmtime_t* vmtime)
{
    return vmtime->time;
}

/*
 * surgescript_vmtime_is_paused()
 * Is the VM time paused?
 */
bool surgescript_vmtime_is_paused(const surgescript_vmtime_t* vmtime)
{
    return vmtime->is_paused;
}