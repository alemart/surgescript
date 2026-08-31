/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2026 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/signal_system.h
 * SurgeScript Signal System
 */

#ifndef _SURGESCRIPT_RUNTIME_SIGNAL_SYSTEM_H
#define _SURGESCRIPT_RUNTIME_SIGNAL_SYSTEM_H

#include <stdbool.h>

typedef struct surgescript_signalsystem_t surgescript_signalsystem_t;
typedef struct surgescript_signalsystembuilder_t surgescript_signalsystembuilder_t;
typedef struct surgescript_objectmanager_t surgescript_objectmanager_t;

typedef enum surgescript_signaltype_t surgescript_signaltype_t;
enum surgescript_signaltype_t
{
    /* a global signal is broadcasted to all existing objects able to catch it
      (i.e., objects having a registered signal handler matching that signal) */
    SIGTYPE_GLOBAL,

    /* a bubble signal can only be caught by the closest ancestor of the emitter
       that implements a matching signal handler (it may or may not be caught) */
    SIGTYPE_BUBBLE
};

/* builder */
surgescript_signalsystembuilder_t* surgescript_signalsystembuilder_create();
surgescript_signalsystembuilder_t* surgescript_signalsystembuilder_destroy(surgescript_signalsystembuilder_t* builder);
bool surgescript_signalsystembuilder_is_signal_registered(surgescript_signalsystembuilder_t* builder, const char* signal_name);
bool surgescript_signalsystembuilder_are_signal_emissions_registered(surgescript_signalsystembuilder_t* builder, const char* object_name);
bool surgescript_signalsystembuilder_are_signal_handlers_registered(surgescript_signalsystembuilder_t* builder, const char* object_name);
void surgescript_signalsystembuilder_register_signal(surgescript_signalsystembuilder_t* builder, const char* signal_name, surgescript_signaltype_t signal_type, const char** field_names);
void surgescript_signalsystembuilder_register_signal_emissions(surgescript_signalsystembuilder_t* builder, const char* object_name, const char** signal_names);
void surgescript_signalsystembuilder_register_signal_handlers(surgescript_signalsystembuilder_t* builder, const char* object_name, const char** signal_names);
surgescript_signalsystem_t* surgescript_signalsystembuilder_build(const surgescript_signalsystembuilder_t* builder, const surgescript_objectmanager_t* object_manager);

#endif
