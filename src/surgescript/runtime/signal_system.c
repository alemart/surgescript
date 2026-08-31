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

static int length_of_list(const char** list);
static char** clone_list(const char** list);
static bool list_has_repetition(const char** list);

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
    #define release_list_of_lists(list) \
        for(int i = ssarray_length(list) - 1; i >= 0; i--) { \
            for(char** it = list[i]; *it; it++) { \
                ssfree(*it); \
            } \
            ssfree(list[i]); \
        } \
        ssarray_release(list)

    #define release_list(list) \
        for(int i = ssarray_length(list) - 1; i >= 0; i--) { \
            ssfree(list[i]); \
        } \
        ssarray_release(list)

    release_list_of_lists(builder->handlers.signal_names);
    release_list(builder->handlers.object_name);

    release_list_of_lists(builder->emissions.signal_names);
    release_list(builder->emissions.object_name);

    ssarray_release(builder->declarations.signal_type);
    release_list_of_lists(builder->declarations.field_names);
    release_list(builder->declarations.signal_name);

    return ssfree(builder);

    #undef release_list
    #undef release_list_of_lists
}

/*
 * surgescript_signalsystembuilder_is_signal_registered()
 * Check if a signal declaration has already been registered
 */
bool surgescript_signalsystembuilder_is_signal_registered(surgescript_signalsystembuilder_t* builder, const char* signal_name)
{
    for(int i = ssarray_length(builder->declarations.signal_name) - 1; i >= 0; i--) {
        if(0 == strcmp(builder->declarations.signal_name[i], signal_name))
            return true;
    }

    return false;
}

/*
 * surgescript_signalsystembuilder_are_signal_emissions_registered()
 * Check if signal emissions have already been registered for a given class of objects
 */
bool surgescript_signalsystembuilder_are_signal_emissions_registered(surgescript_signalsystembuilder_t* builder, const char* object_name)
{
    for(int i = ssarray_length(builder->emissions.object_name) - 1; i >= 0; i--) {
        if(0 == strcmp(builder->emissions.object_name[i], object_name))
            return true;
    }

    return false;
}

/*
 * surgescript_signalsystembuilder_are_signal_handlers_registered()
 * Check if signal handlers have already been registered for a given class of objects
 */
bool surgescript_signalsystembuilder_are_signal_handlers_registered(surgescript_signalsystembuilder_t* builder, const char* object_name)
{
    for(int i = ssarray_length(builder->handlers.object_name) - 1; i >= 0; i--) {
        if(0 == strcmp(builder->handlers.object_name[i], object_name))
            return true;
    }

    return false;
}

/*
 * surgescript_signalsystembuilder_register_signal()
 * Register a signal declaration. You must ensure uniqueness;
 * see surgescript_signalsystembuilder_is_signal_registered().
 * field_names is a NULL-terminated array of strings
 */
void surgescript_signalsystembuilder_register_signal(surgescript_signalsystembuilder_t* builder, const char* signal_name, surgescript_signaltype_t signal_type, const char** field_names)
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
void surgescript_signalsystembuilder_register_signal_emissions(surgescript_signalsystembuilder_t* builder, const char* object_name, const char** signal_names)
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
void surgescript_signalsystembuilder_register_signal_handlers(surgescript_signalsystembuilder_t* builder, const char* object_name, const char** signal_names)
{
    ssassert(!surgescript_signalsystembuilder_are_signal_handlers_registered(builder, object_name));
    ssassert(!list_has_repetition(signal_names));

    ssarray_push(builder->handlers.object_name, ssstrdup(object_name));
    ssarray_push(builder->handlers.signal_names, clone_list(signal_names));
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
int length_of_list(const char** list)
{
    int count = 0;

    if(NULL == list) /* accept NULL? */
        return 0;

    for(const char** it = list; *it; it++)
        count++;

    return count;
}

/* clone a NULL-terminated array of strings */
char** clone_list(const char** list)
{
    int length = length_of_list(list);
    char** clone = ssmalloc((1 + length) * sizeof(char*));

    clone[length] = NULL;
    for(int i = length - 1; i >= 0; i--)
        clone[i] = ssstrdup(list[i]);

    return clone;
}

/* check if a NULL-terminated array of strings has repeated elements */
bool list_has_repetition(const char** list)
{
    for(const char** it = list; *it; it++) {
        for(const char** it2 = list; it2 != it && *it2; it2++) {
            if(0 == strcmp(*it, *it2))
                return true;
        }
    }

    return false;
}