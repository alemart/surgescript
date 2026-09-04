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
 * runtime/signal_system.c
 * SurgeScript Signal System
 */

#include <string.h>
#include "signal_system.h"
#include "object_manager.h"
#include "../util/ssarray.h"
#include "../util/util.h"

static int length_of_list(char* const* list);
static char** clone_list(char* const* list);
static bool list_has_repetition(char* const* list);
static int index_of_string(const char* key, char* const* array, size_t length);

/*
--------------------------------------------------------------------------------
Builder
--------------------------------------------------------------------------------
*/

struct surgescript_signalsystembuilder_t
{
    struct {
        /* the signal named signal_name[i] has type signal_type[i] and fields named field_names[i] */
        SSARRAY(char*, signal_name);
        SSARRAY(char**, field_names); /* NULL-terminated arrays */
        SSARRAY(surgescript_signaltype_t, signal_type);
    } declarations;

    struct {
        /* objects named object_name[i] can emit signals named signal_names[i] */
        SSARRAY(char*, object_name);
        SSARRAY(char**, signal_names); /* NULL-terminated arrays */
    } emissions;

    struct {
        /* objects named object_name[i] can catch signals named signal_names[i] */
        SSARRAY(char*, object_name);
        SSARRAY(char**, signal_names); /* NULL-terminated arrays */
    } handlers;
};

#define release_array_of_strings(list) \
    for(int i = ssarray_length(list) - 1; i >= 0; i--) { \
        ssfree(list[i]); \
    } \
    ssarray_release(list)

#define release_list_of_strings(list) \
    for(char** it = list; *it; it++) { \
        ssfree(*it); \
    } \
    ssfree(list)

#define release_array_of_lists_of_strings(list) \
    for(int i = ssarray_length(list) - 1; i >= 0; i--) { \
        release_list_of_strings(list[i]); \
    } \
    ssarray_release(list)

/*
 * surgescript_signalsystembuilder_create()
 * Create a Signal System Builder
 */
surgescript_signalsystembuilder_t* surgescript_signalsystembuilder_create()
{
    surgescript_signalsystembuilder_t* builder = ssmalloc(sizeof *builder);

    ssarray_init(builder->declarations.signal_name);
    ssarray_init(builder->declarations.field_names);
    ssarray_init(builder->declarations.signal_type);

    ssarray_init(builder->emissions.object_name);
    ssarray_init(builder->emissions.signal_names);

    ssarray_init(builder->handlers.object_name);
    ssarray_init(builder->handlers.signal_names);

    return builder;
}

/*
 * surgescript_signalsystembuilder_destroy()
 * Destroy a Signal System Builder
 */
surgescript_signalsystembuilder_t* surgescript_signalsystembuilder_destroy(surgescript_signalsystembuilder_t* builder)
{
    release_array_of_lists_of_strings(builder->handlers.signal_names);
    release_array_of_strings(builder->handlers.object_name);

    release_array_of_lists_of_strings(builder->emissions.signal_names);
    release_array_of_strings(builder->emissions.object_name);

    ssarray_release(builder->declarations.signal_type);
    release_array_of_lists_of_strings(builder->declarations.field_names);
    release_array_of_strings(builder->declarations.signal_name);

    return ssfree(builder);
}

/*
 * surgescript_signalsystembuilder_is_signal_registered()
 * Check if a signal declaration has already been registered
 */
bool surgescript_signalsystembuilder_is_signal_registered(surgescript_signalsystembuilder_t* builder, const char* signal_name)
{
    int i = index_of_string(signal_name, builder->declarations.signal_name, ssarray_length(builder->declarations.signal_name));
    return i >= 0;
}

/*
 * surgescript_signalsystembuilder_is_signal_emission_registered()
 * Check if a signal emission has already been registered for a given object
 */
bool surgescript_signalsystembuilder_is_signal_emission_registered(surgescript_signalsystembuilder_t* builder, const char* object_name, const char* signal_name)
{
    int i = index_of_string(object_name, builder->emissions.object_name, ssarray_length(builder->emissions.object_name));
    if(i >= 0) {
        for(char* const* it = builder->emissions.signal_names[i]; *it; it++) {
            if(0 == strcmp(*it, signal_name))
                return true;
        }
    }
    return false;
}

/*
 * surgescript_signalsystembuilder_is_signal_handler_registered()
 * Check if a signal handler has already been registered for a given object
 */
bool surgescript_signalsystembuilder_is_signal_handler_registered(surgescript_signalsystembuilder_t* builder, const char* object_name, const char* signal_name)
{
    int i = index_of_string(object_name, builder->handlers.object_name, ssarray_length(builder->handlers.object_name));
    if(i >= 0) {
        for(char* const* it = builder->handlers.signal_names[i]; *it; it++) {
            if(0 == strcmp(*it, signal_name))
                return true;
        }
    }
    return false;
}

/*
 * surgescript_signalsystembuilder_are_signal_emissions_registered()
 * Check if signal emissions have already been registered for a given class of objects
 */
bool surgescript_signalsystembuilder_are_signal_emissions_registered(surgescript_signalsystembuilder_t* builder, const char* object_name)
{
    int i = index_of_string(object_name, builder->emissions.object_name, ssarray_length(builder->emissions.object_name));
    return i >= 0;
}

/*
 * surgescript_signalsystembuilder_are_signal_handlers_registered()
 * Check if signal handlers have already been registered for a given class of objects
 */
bool surgescript_signalsystembuilder_are_signal_handlers_registered(surgescript_signalsystembuilder_t* builder, const char* object_name)
{
    int i = index_of_string(object_name, builder->handlers.object_name, ssarray_length(builder->handlers.object_name));
    return i >= 0;
}

/*
 * surgescript_signalsystembuilder_register_signal()
 * Register a signal declaration. You must ensure uniqueness;
 * see surgescript_signalsystembuilder_is_signal_registered().
 * field_names is a NULL-terminated array of strings
 */
void surgescript_signalsystembuilder_register_signal(surgescript_signalsystembuilder_t* builder, const char* signal_name, surgescript_signaltype_t signal_type, char* const* field_names)
{
    ssassert(!surgescript_signalsystembuilder_is_signal_registered(builder, signal_name));
    ssassert(!list_has_repetition(field_names));

    ssarray_push(builder->declarations.signal_name, ssstrdup(signal_name));
    ssarray_push(builder->declarations.signal_type, signal_type);
    ssarray_push(builder->declarations.field_names, clone_list(field_names));
}

/*
 * surgescript_signalsystembuilder_register_signal_emissions()
 * Register a signal emission. You must ensure uniqueness;
 * see surgescript_signalsystembuilder_are_signal_emissions_registered().
 * signal_names is a NULL-terminated array of strings
 */
void surgescript_signalsystembuilder_register_signal_emissions(surgescript_signalsystembuilder_t* builder, const char* object_name, char* const* signal_names)
{
    ssassert(!surgescript_signalsystembuilder_are_signal_emissions_registered(builder, object_name));
    ssassert(!list_has_repetition(signal_names));

    ssarray_push(builder->emissions.object_name, ssstrdup(object_name));
    ssarray_push(builder->emissions.signal_names, clone_list(signal_names));
}

/*
 * surgescript_signalsystembuilder_register_signal_handlers()
 * Register a signal handler. You must ensure uniqueness;
 * see surgescript_signalsystembuilder_are_signal_handlers_registered().
 * signal_names is a NULL-terminated array of strings
 */
void surgescript_signalsystembuilder_register_signal_handlers(surgescript_signalsystembuilder_t* builder, const char* object_name, char* const* signal_names)
{
    ssassert(!surgescript_signalsystembuilder_are_signal_handlers_registered(builder, object_name));
    ssassert(!list_has_repetition(signal_names));

    ssarray_push(builder->handlers.object_name, ssstrdup(object_name));
    ssarray_push(builder->handlers.signal_names, clone_list(signal_names));
}

/*
 * surgescript_signalsystembuilder_unregister_signal()
 * Unregister a signal declaration. Returns true on success.
 */
bool surgescript_signalsystembuilder_unregister_signal(surgescript_signalsystembuilder_t* builder, const char* signal_name)
{
    int i = index_of_string(signal_name, builder->declarations.signal_name, ssarray_length(builder->declarations.signal_name));
    if(i < 0)
        return false;

    release_list_of_strings(builder->declarations.field_names[i]);
    ssarray_remove(builder->declarations.field_names, i);

    ssarray_remove(builder->declarations.signal_type, i);

    ssarray_remove(builder->declarations.signal_name, i);
    ssfree(builder->declarations.signal_name[i]);

    return true;
}

/*
 * surgescript_signalsystembuilder_unregister_signal_emissions()
 * Unregister signal emissions of an object. Returns true on success.
 */
bool surgescript_signalsystembuilder_unregister_signal_emissions(surgescript_signalsystembuilder_t* builder, const char* object_name)
{
    int i = index_of_string(object_name, builder->emissions.object_name, ssarray_length(builder->emissions.object_name));
    if(i < 0)
        return false;

    release_list_of_strings(builder->emissions.signal_names[i]);
    ssarray_remove(builder->emissions.signal_names, i);

    ssfree(builder->emissions.object_name[i]);
    ssarray_remove(builder->emissions.object_name, i);

    return true;
}

/*
 * surgescript_signalsystembuilder_unregister_signal()
 * Unregister signal handlers of an object. Returns true on success.
 */
bool surgescript_signalsystembuilder_unregister_signal_handlers(surgescript_signalsystembuilder_t* builder, const char* object_name)
{
    int i = index_of_string(object_name, builder->handlers.object_name, ssarray_length(builder->handlers.object_name));
    if(i < 0)
        return false;

    release_list_of_strings(builder->handlers.signal_names[i]);
    ssarray_remove(builder->handlers.signal_names, i);

    ssfree(builder->handlers.object_name[i]);
    ssarray_remove(builder->handlers.object_name, i);

    return true;
}

/*
 * surgescript_signalsystembuilder_build()
 * Build a Signal System
 */
surgescript_signalsystem_t* surgescript_signalsystembuilder_build(const surgescript_signalsystembuilder_t* builder, const surgescript_objectmanager_t* object_manager)
{
    // TODO
    return NULL;
}

/*
--------------------------------------------------------------------------------
private stuff
--------------------------------------------------------------------------------
*/

/* count the number of elements of a NULL-terminated array of strings */
int length_of_list(char* const* list)
{
    int count = 0;

    if(NULL == list) /* accept NULL? */
        return 0;

    for(char* const* it = list; *it; it++)
        count++;

    return count;
}

/* clone a NULL-terminated array of strings */
char** clone_list(char* const* list)
{
    int length = length_of_list(list);
    char** clone = ssmalloc((1 + length) * sizeof(char*));

    clone[length] = NULL;
    for(int i = length - 1; i >= 0; i--)
        clone[i] = ssstrdup(list[i]);

    return clone;
}

/* check if a NULL-terminated array of strings has repeated elements */
bool list_has_repetition(char* const* list)
{
    for(char* const* it = list; *it; it++) {
        for(char* const* it2 = list; it2 != it && *it2; it2++) {
            if(0 == strcmp(*it, *it2))
                return true;
        }
    }

    return false;
}

/* the index of a key in an array of strings, or -1 if not found */
int index_of_string(const char* key, char* const* array, size_t length)
{
    /* start by the end (more efficient given the usage?) */
    while(length--) {
        if(0 == strcmp(key, array[length]))
            return length;
    }

    return -1;
}