/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2024 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/vm_time.h
 * SurgeScript Virtual Machine Time - this is used to count time
 */

#ifndef _SURGESCRIPT_RUNTIME_VM_TIME_H
#define _SURGESCRIPT_RUNTIME_VM_TIME_H

#include <stdint.h>
#include <stdbool.h>

typedef struct surgescript_vmtime_t surgescript_vmtime_t;

surgescript_vmtime_t* surgescript_vmtime_create(); /* create a VM time object */
surgescript_vmtime_t* surgescript_vmtime_destroy(surgescript_vmtime_t* vmtime); /* destroy a VM time object */

void surgescript_vmtime_update(surgescript_vmtime_t* vmtime); /* update the VM time object */
void surgescript_vmtime_pause(surgescript_vmtime_t* vmtime); /* pause the VM time */
void surgescript_vmtime_resume(surgescript_vmtime_t* vmtime); /* resume the VM time */

uint64_t surgescript_vmtime_time(const surgescript_vmtime_t* vmtime); /* the time at the beginning of the current update cycle */
bool surgescript_vmtime_is_paused(const surgescript_vmtime_t* vmtime); /* is the VM time paused? */

#endif