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
#include <stdint.h>
#include "object.h"

typedef uint32_t surgescript_signalcode_t;
typedef struct surgescript_signalsystem_t surgescript_signalsystem_t;
typedef struct surgescript_signalsystembuilder_t surgescript_signalsystembuilder_t;

typedef enum surgescript_signaltype_t surgescript_signaltype_t;
enum surgescript_signaltype_t
{
    /* a global signal is broadcasted to all existing objects able to catch it
      (i.e., objects having a registered signal handler matching that signal) */
    SIGTYPE_GLOBAL,

    /* a bubble signal can only be caught by the closest ancestor of the emitter
       that implements a matching signal handler (it may or may not be caught) */
    SIGTYPE_BUBBLE,

    /* a manual signal can only be caught by objects manually and explicitly
       connected to the emitter */
    /*SIGTYPE_MANUAL,*/

    /* a null signal does nothing; this is an error type */
    SIGTYPE_NULL
};

struct surgescript_objectmanager_t;

/* builder */
surgescript_signalsystembuilder_t* surgescript_signalsystembuilder_create();
surgescript_signalsystembuilder_t* surgescript_signalsystembuilder_destroy(surgescript_signalsystembuilder_t* builder);
bool surgescript_signalsystembuilder_is_signal_registered(surgescript_signalsystembuilder_t* builder, const char* signal_name);
bool surgescript_signalsystembuilder_is_signal_emission_registered(surgescript_signalsystembuilder_t* builder, const char* object_name, const char* signal_name);
bool surgescript_signalsystembuilder_is_signal_handler_registered(surgescript_signalsystembuilder_t* builder, const char* object_name, const char* signal_name);
bool surgescript_signalsystembuilder_are_signal_emissions_registered(surgescript_signalsystembuilder_t* builder, const char* object_name);
bool surgescript_signalsystembuilder_are_signal_handlers_registered(surgescript_signalsystembuilder_t* builder, const char* object_name);
void surgescript_signalsystembuilder_register_signal(surgescript_signalsystembuilder_t* builder, const char* signal_name, surgescript_signaltype_t signal_type, char* const* field_names);
void surgescript_signalsystembuilder_register_signal_emissions(surgescript_signalsystembuilder_t* builder, const char* object_name, char* const* signal_names);
void surgescript_signalsystembuilder_register_signal_handlers(surgescript_signalsystembuilder_t* builder, const char* object_name, char* const* signal_names);
bool surgescript_signalsystembuilder_unregister_signal(surgescript_signalsystembuilder_t* builder, const char* signal_name);
bool surgescript_signalsystembuilder_unregister_signal_emissions(surgescript_signalsystembuilder_t* builder, const char* object_name);
bool surgescript_signalsystembuilder_unregister_signal_handlers(surgescript_signalsystembuilder_t* builder, const char* object_name);
surgescript_signalsystem_t* surgescript_signalsystembuilder_build(const surgescript_signalsystembuilder_t* builder, const struct surgescript_objectmanager_t* object_manager);

/* signal system */
surgescript_signalsystem_t* surgescript_signalsystem_destroy(surgescript_signalsystem_t* signal_system);
bool surgescript_signalsystem_signal_exists(const surgescript_signalsystem_t* signal_system, const char* signal_name);
surgescript_signalcode_t surgescript_signalsystem_signal_code(const surgescript_signalsystem_t* signal_system, const char* signal_name);
surgescript_signaltype_t surgescript_signalsystem_signal_type(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code);
const char* surgescript_signalsystem_signal_name(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code);
bool surgescript_signalsystem_object_can_emit(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code, surgescript_objectclassid_t class_id);
bool surgescript_signalsystem_object_can_catch(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code, surgescript_objectclassid_t class_id);
bool surgescript_signalsystem_object_has_handlers(const surgescript_signalsystem_t* signal_system, surgescript_objectclassid_t class_id);
bool surgescript_signalsystem_signal_has_subscribers(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code);
void surgescript_signalsystem_subscribe(surgescript_signalsystem_t* signal_system, const surgescript_object_t* object);
void surgescript_signalsystem_unsubscribe(surgescript_signalsystem_t* signal_system, const surgescript_object_t* object);
void surgescript_signalsystem_emit_signal(const surgescript_signalsystem_t* signal_system, surgescript_objecthandle_t emitter, surgescript_signalcode_t signal_code, surgescript_objecthandle_t signal_context);

#endif
