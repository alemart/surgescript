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
#include "program_pool.h"
#include "../util/perfect_hash.h"
#include "../util/ssarray.h"
#include "../util/util.h"
#include "../util/xxh.h"

#define FASTHASH_INLINE
#include "../util/fasthash.h"

static int length_of_list_of_strings(char* const* list);
static char** clone_list_of_strings(char* const* list);
static bool list_of_strings_has_repetition(char* const* list);
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

#define is_valid_signal_type(signal_type) \
    ((signal_type) == SIGTYPE_GLOBAL || (signal_type) == SIGTYPE_BUBBLE)

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
    ssassert(!list_of_strings_has_repetition(field_names));
    ssassert(is_valid_signal_type(signal_type));

    ssarray_push(builder->declarations.signal_name, ssstrdup(signal_name));
    ssarray_push(builder->declarations.signal_type, signal_type);
    ssarray_push(builder->declarations.field_names, clone_list_of_strings(field_names));
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
    ssassert(!list_of_strings_has_repetition(signal_names));

    ssarray_push(builder->emissions.object_name, ssstrdup(object_name));
    ssarray_push(builder->emissions.signal_names, clone_list_of_strings(signal_names));
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
    ssassert(!list_of_strings_has_repetition(signal_names));

    ssarray_push(builder->handlers.object_name, ssstrdup(object_name));
    ssarray_push(builder->handlers.signal_names, clone_list_of_strings(signal_names));
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

    ssfree(builder->declarations.signal_name[i]);
    ssarray_remove(builder->declarations.signal_name, i);

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
--------------------------------------------------------------------------------
Signal System
--------------------------------------------------------------------------------
*/

typedef struct signaldata_t signaldata_t;
typedef struct signaledclass_t signaledclass_t;

struct signaldata_t
{
    surgescript_signalcode_t code; /* key */

    surgescript_signaltype_t type;
    char* name;
    char** field_names; /* NULL-terminated array */

    SSARRAY(surgescript_objectclassid_t, subscribers); /* sorted array of class of objects capable of catching this signal */
    SSARRAY(const struct signaledclass_t*, cached_subscribers); /* subscribers mapped to signaledclass_t */
};

struct signaledclass_t
{
    surgescript_objectclassid_t object_class; /* key */

    SSARRAY(const struct signaldata_t*, emitted_signals);
    SSARRAY(const struct signaldata_t*, caught_signals);

    SSARRAY(surgescript_objecthandle_t, instances);
    SSARRAY(surgescript_objecthandle_t, dirty_instances); /* cleanup queue */

    bool catches_global_signal; /* is any caught signal global? */
};

struct surgescript_signalsystem_t
{
    surgescript_perfecthashseed_t hash_seed; /* used to compute signal codes */
    const surgescript_objectmanager_t* object_manager; /* reference to the object manager */

    fasthash_t* signals; /* map: signal code -> signaldata_t */
    fasthash_t* signaled_classes; /* map: object class -> signaledclass_t */
    fasthash_t* context_validity; /* map: pair<signal code, signal context object class> -> bool */
};

static surgescript_perfecthashkey_t hash_of_signal_name(const char* signal_name, surgescript_perfecthashseed_t seed);
static int index_of_signal_code(surgescript_signalcode_t signal_code, const signaldata_t* const* array, size_t length);
static int index_of_object_handle(surgescript_objecthandle_t handle, const surgescript_objecthandle_t* array, size_t length);
static int binary_search_object_handle(surgescript_objecthandle_t target, const surgescript_objecthandle_t* array, size_t length);
static int binary_search_object_class_id(surgescript_objectclassid_t target, const surgescript_objectclassid_t* array, size_t length);
static signaldata_t* construct_signal_data(surgescript_signalcode_t signal_code, surgescript_signaltype_t signal_type, const char* signal_name, char* const* field_names);
static signaledclass_t* construct_signaled_class(surgescript_objectclassid_t object_class);
static void destruct_signal_data(void* ptr);
static void destruct_signaled_class(void* ptr);
static void foreach_own_program(const char* emitter_name, void* data);
static bool is_valid_signal_context(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code, surgescript_objecthandle_t signal_context);
static bool is_valid_signal_context_cached(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code, surgescript_objecthandle_t signal_context);
static const int LG2_MAX_OBJECT_CLASSES = 10; /* just a guess, not a strict limit */
static const int LG2_MAX_SIGNAL_CLASSES = 9;
#define LG2_INITIAL_CAPACITY(x) (2+(x)) /* make hash tables sparse */

/*
 * surgescript_signalsystembuilder_build()
 * Build a Signal System
 */
surgescript_signalsystem_t* surgescript_signalsystembuilder_build(const surgescript_signalsystembuilder_t* builder, const surgescript_objectmanager_t* object_manager)
{
    /* find a perfect hash function for signal names */
    surgescript_perfecthashseed_t hash_seed = surgescript_perfecthash_find_seed(hash_of_signal_name, builder->declarations.signal_name, ssarray_length(builder->declarations.signal_name));
    //printf("Signal System hash seed: %u\n", hash_seed);

    /* allocate an instance */
    surgescript_signalsystem_t* signal_system = ssmalloc(sizeof *signal_system);
    signal_system->hash_seed = hash_seed;
    signal_system->object_manager = object_manager;
    signal_system->signals = fasthash_create(destruct_signal_data, LG2_INITIAL_CAPACITY(LG2_MAX_SIGNAL_CLASSES));
    signal_system->signaled_classes = fasthash_create(destruct_signaled_class, LG2_INITIAL_CAPACITY(LG2_MAX_OBJECT_CLASSES));
    signal_system->context_validity = fasthash_create(NULL, LG2_INITIAL_CAPACITY(LG2_MAX_SIGNAL_CLASSES));

    /* store the declaration of each signal */
    for(int i = 0; i < ssarray_length(builder->declarations.signal_name); i++) {
        const char* signal_name = builder->declarations.signal_name[i];
        char* const* field_names = builder->declarations.field_names[i];
        surgescript_signaltype_t signal_type = builder->declarations.signal_type[i];
        surgescript_signalcode_t signal_code = surgescript_signalsystem_signal_code(signal_system, signal_name);

        ssassert(NULL == fasthash_get(signal_system->signals, signal_code)); /* no duplicates allowed */

        signaldata_t* signal = construct_signal_data(signal_code, signal_type, signal_name, field_names);
        fasthash_put(signal_system->signals, signal_code, signal);
    }

    /* for each object class, store the emitted signal codes */
    for(int i = 0; i < ssarray_length(builder->emissions.object_name); i++) {
        const char* object_name = builder->emissions.object_name[i];
        surgescript_objectclassid_t object_class = surgescript_objectmanager_class_id(object_manager, object_name);

        char* const* signal_names = builder->emissions.signal_names[i];
        for(char* const* it = signal_names; *it; it++) {
            const char* signal_name = *it;
            surgescript_signalcode_t signal_code = surgescript_signalsystem_signal_code(signal_system, signal_name);

            const signaldata_t* signal = fasthash_get(signal_system->signals, signal_code);
            if(signal == NULL) {
                sslog("Warning: object \"%s\" can't emit undeclared signal \"%s\"", object_name, signal_name);
                continue; /* ignore undeclared signals */
            }

            signaledclass_t* signaled_class = fasthash_get(signal_system->signaled_classes, object_class);
            if(signaled_class == NULL) {
                signaled_class = construct_signaled_class(object_class);
                fasthash_put(signal_system->signaled_classes, object_class, signaled_class);
            }

            ssarray_push(signaled_class->emitted_signals, signal);
        }
    }

    /* for each object class, store the caught signal codes */
    for(int i = 0; i < ssarray_length(builder->handlers.object_name); i++) {
        const char* object_name = builder->handlers.object_name[i];
        surgescript_objectclassid_t object_class = surgescript_objectmanager_class_id(object_manager, object_name);

        char* const* signal_names = builder->handlers.signal_names[i];
        for(char* const* it = signal_names; *it; it++) {
            const char* signal_name = *it;
            surgescript_signalcode_t signal_code = surgescript_signalsystem_signal_code(signal_system, signal_name);

            const signaldata_t* signal = fasthash_get(signal_system->signals, signal_code);
            if(signal == NULL) {
                sslog("Warning: object \"%s\" can't catch undeclared signal \"%s\"", object_name, signal_name);
                continue; /* ignore undeclared signals */
            }

            signaledclass_t* signaled_class = fasthash_get(signal_system->signaled_classes, object_class);
            if(signaled_class == NULL) {
                signaled_class = construct_signaled_class(object_class);
                fasthash_put(signal_system->signaled_classes, object_class, signaled_class);
            }

            ssarray_push(signaled_class->caught_signals, signal);
            if(signal->type == SIGTYPE_GLOBAL)
                signaled_class->catches_global_signal = true;
        }
    }

    /* for each signal, find its subscribers */
    for(int i = 0; i < ssarray_length(builder->declarations.signal_name); i++) {
        const char* signal_name = builder->declarations.signal_name[i];
        surgescript_signalcode_t signal_code = surgescript_signalsystem_signal_code(signal_system, signal_name);

        /* for each object that can catch a signal */
        for(int j = 0; j < ssarray_length(builder->handlers.object_name); j++) {
            const char* object_name = builder->handlers.object_name[j];
            surgescript_objectclassid_t object_class = surgescript_objectmanager_class_id(object_manager, object_name);

            /* check if that object catches the signal we're inspecting */
            const signaledclass_t* signaled_class = fasthash_get(signal_system->signaled_classes, object_class);
            if(signaled_class != NULL && index_of_signal_code(signal_code, signaled_class->caught_signals, ssarray_length(signaled_class->caught_signals)) >= 0) {

                /* add the object class to the list of subscribers of the signal */
                signaldata_t* signal = fasthash_get(signal_system->signals, signal_code);
                ssassert(signal != NULL); /* the signal must exist */
                ssarray_push(signal->subscribers, object_class);
                ssarray_push(signal->cached_subscribers, signaled_class);

                /* keep the subscribers list sorted */
                for(int k = ssarray_length(signal->subscribers) - 1; k >= 1 && signal->subscribers[k-1] > signal->subscribers[k]; k--) {
                    signal->subscribers[k] = signal->subscribers[k-1];
                    signal->subscribers[k-1] = object_class;

                    signal->cached_subscribers[k] = signal->cached_subscribers[k-1];
                    signal->cached_subscribers[k-1] = signaled_class;
                }

            }
        }
    }

    /* done! */
    return signal_system;
}

/*
 * surgescript_signalsystem_destroy()
 * Release a Signal System instance
 */
surgescript_signalsystem_t* surgescript_signalsystem_destroy(surgescript_signalsystem_t* signal_system)
{
    fasthash_destroy(signal_system->context_validity);
    fasthash_destroy(signal_system->signaled_classes);
    fasthash_destroy(signal_system->signals);
    return ssfree(signal_system);
}

/*
 * surgescript_signalsystem_signal_code()
 * Get the code of a signal name
 */
surgescript_signalcode_t surgescript_signalsystem_signal_code(const surgescript_signalsystem_t* signal_system, const char* signal_name)
{
    return hash_of_signal_name(signal_name, signal_system->hash_seed);
}

/*
 * surgescript_signalsystem_signal_type()
 * Get the type of a given class of signals
 */
surgescript_signaltype_t surgescript_signalsystem_signal_type(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code)
{
    const signaldata_t* signal = fasthash_get(signal_system->signals, signal_code);

    /* no such signal? */
    if(signal == NULL)
        return SIGTYPE_NULL;

    /* return the signal type */
    return signal->type;
}

/*
 * surgescript_signalsystem_signal_name()
 * Get the name of a given class of signals
 */
const char* surgescript_signalsystem_signal_name(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code)
{
     const signaldata_t* signal = fasthash_get(signal_system->signals, signal_code);

    /* no such signal? */
    if(signal == NULL)
        return "<unknown_signal>";

    /* return the signal name */
    return signal->name;
}

/*
 * surgescript_signalsystem_signal_exists()
 * Check if a signal exists (i.e., if it has been declared)
 */
bool surgescript_signalsystem_signal_exists(const surgescript_signalsystem_t* signal_system, const char* signal_name)
{
    surgescript_signalcode_t signal_code = surgescript_signalsystem_signal_code(signal_system, signal_name);
    const signaldata_t* signal = fasthash_get(signal_system->signals, signal_code);
    return signal != NULL;
}

/*
 * surgescript_signalsystem_object_can_emit()
 * Check if a class of objects can emit a given class of signals
 */
bool surgescript_signalsystem_object_can_emit(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code, surgescript_objectclassid_t object_class)
{
    const signaledclass_t* signaled_class = fasthash_get(signal_system->signaled_classes, object_class);
    return (signaled_class != NULL) && (index_of_signal_code(signal_code, signaled_class->emitted_signals, ssarray_length(signaled_class->emitted_signals)) >= 0);
}

/*
 * surgescript_signalsystem_object_can_catch()
 * Check if a class of objects has a handler to catch a given class of signals
 */
bool surgescript_signalsystem_object_can_catch(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code, surgescript_objectclassid_t object_class)
{
    const signaledclass_t* signaled_class = fasthash_get(signal_system->signaled_classes, object_class);
    return (signaled_class != NULL) && (index_of_signal_code(signal_code, signaled_class->caught_signals, ssarray_length(signaled_class->caught_signals)) >= 0);
}

/*
 * surgescript_signalsystem_object_has_handlers()
 * Check if a class of objects has one or more signal handlers (i.e., can catch signals)
 */
bool surgescript_signalsystem_object_has_handlers(const surgescript_signalsystem_t* signal_system, surgescript_objectclassid_t object_class)
{
    const signaledclass_t* signaled_class = fasthash_get(signal_system->signaled_classes, object_class);
    return (signaled_class != NULL) && (ssarray_length(signaled_class->caught_signals) > 0);
}

/*
 * surgescript_signalsystem_signal_has_subscribers()
 * Check if a class of signals has one or more subscribers (object class IDs, not object instances)
 */
bool surgescript_signalsystem_signal_has_subscribers(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code)
{
    const signaldata_t* signal = fasthash_get(signal_system->signals, signal_code);
    return (signal != NULL) && (ssarray_length(signal->subscribers) > 0);
}

/*
 * surgescript_signalsystem_subscribe()
 * Subscribe an object to all global signals that it can catch
 */
void surgescript_signalsystem_subscribe(surgescript_signalsystem_t* signal_system, const surgescript_object_t* object)
{
    /* ************************************************************************ */
    /* ***** This operation must be fast! It's used when spawning objects ***** */
    /* ************************************************************************ */
    surgescript_objectclassid_t object_class = surgescript_object_class_id(object);
    signaledclass_t* signaled_class = fasthash_get(signal_system->signaled_classes, object_class);

    /* skip if the object can't catch any global signal */
    if(signaled_class == NULL || !signaled_class->catches_global_signal)
        return;

    /* keep track of this instance */
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    ssarray_push(signaled_class->instances, handle);
}

/*
 * surgescript_signalsystem_unsubscribe()
 * Unsubscribe an object from all global signals that it can catch
 */
void surgescript_signalsystem_unsubscribe(surgescript_signalsystem_t* signal_system, const surgescript_object_t* object)
{
    /* ************************************************************************ */
    /* ***** This operation must be fast! It's used when deleting objects ***** */
    /* ************************************************************************ */
    surgescript_objectclassid_t object_class = surgescript_object_class_id(object);
    signaledclass_t* signaled_class = fasthash_get(signal_system->signaled_classes, object_class);

    /* skip if the object can't catch any global signal */
    if(signaled_class == NULL || !signaled_class->catches_global_signal)
        return;

    /* in order to make this fast, let's postpone the removal; for now, just mark this instance as "dirty" */
    surgescript_objecthandle_t handle = surgescript_object_handle(object);
    ssarray_push(signaled_class->dirty_instances, handle);

    /* FIXME however, if the list gets too big, maybe we're never recycling things */
    /* not too big: recycling object handles may happen */
}

/*
 * surgescript_signalsystem_emit_signal()
 * Emit a signal
 */

void surgescript_signalsystem_emit_signal(const surgescript_signalsystem_t* signal_system, surgescript_objecthandle_t emitter, surgescript_signalcode_t signal_code, surgescript_objecthandle_t signal_context)
{
    const surgescript_object_t* emitter_instance = surgescript_objectmanager_get(signal_system->object_manager, emitter);
    surgescript_objectclassid_t emitter_class = surgescript_object_class_id(emitter_instance);

    /* check whether or not the emitter is able to emit this signal */
    if(!surgescript_signalsystem_object_can_emit(signal_system, signal_code, emitter_class)) {
        const char* emitter_name = surgescript_object_name(emitter_instance);
        const char* signal_name = surgescript_signalsystem_signal_name(signal_system, signal_code);

        if(surgescript_signalsystem_signal_exists(signal_system, signal_name))
            ssfatal("Runtime Error: object \"%s\" can't emit signal \"%s\". Double check the list of emitted signals for this object.", emitter_name, signal_name);
        else
            ssfatal("Runtime Error: object \"%s\" can't emit signal \"%s\". There is no such signal.", emitter_name, signal_name); /* this shouldn't happen */

        return;
    }

    /* check whether or not the provided signal context is valid for this signal */
    if(!is_valid_signal_context_cached(signal_system, signal_code, signal_context)) {
        const char* emitter_name = surgescript_object_name(emitter_instance);
        const char* signal_name = surgescript_signalsystem_signal_name(signal_system, signal_code);

        ssfatal("Runtime Error: object \"%s\" can't emit signal \"%s\". The provided context does not match the required shape.", emitter_name, signal_name);
        return;
    }

    // TODO
    /*

    sketch for global signals:

    1. cleanup
    2. notify all

    during cleanup,

    _. skip cleanup if dirty list is empty
    a. sort dirty instances (those scheduled for cleanup)
    b. for each instance, use binary search to check if it's dirty
    c. if it's dirty, quickly remove with swap-and-pop (last element is no-op)
    d. clear dirty list

    while iterating, take care of deletions happening at the same time

    sketch for bubble signals:

    think if caching is really worth it. reparenting problems.

    */
}

/*
--------------------------------------------------------------------------------
private utilities
--------------------------------------------------------------------------------
*/

/* hash of signal name */
surgescript_perfecthashkey_t hash_of_signal_name(const char* signal_name, surgescript_perfecthashseed_t seed)
{
    return XXH(signal_name, strlen(signal_name), seed);
}

/* count the number of elements of a NULL-terminated array of strings */
int length_of_list_of_strings(char* const* list)
{
    int count = 0;

    if(NULL == list) /* accept NULL? */
        return 0;

    for(char* const* it = list; *it; it++)
        count++;

    return count;
}

/* clone a NULL-terminated array of strings */
char** clone_list_of_strings(char* const* list)
{
    int length = length_of_list_of_strings(list);
    char** clone = ssmalloc((1 + length) * sizeof(char*));

    clone[length] = NULL;
    for(int i = length - 1; i >= 0; i--)
        clone[i] = ssstrdup(list[i]);

    return clone;
}

/* check if a NULL-terminated array of strings has repeated elements */
bool list_of_strings_has_repetition(char* const* list)
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

/* find the index of an entry in a surgescript_signalcode_t[] */
int index_of_signal_code(surgescript_signalcode_t signal_code, const signaldata_t* const* array, size_t length)
{
    while(length--) {
        if(signal_code == array[length]->code)
            return length;
    }

    return -1;
}

/* find the index of an entry in a surgescript_objecthandle_t[] */
int index_of_object_handle(surgescript_objecthandle_t handle, const surgescript_objecthandle_t* array, size_t length)
{
    while(length--) {
        if(handle == array[length])
            return length;
    }

    return -1;
}

/* use binary search to find the index of an entry in a surgescript_objecthandle_t[] */
int binary_search_object_handle(surgescript_objecthandle_t target, const surgescript_objecthandle_t* array, size_t length)
{
    int l = 0, r = (int)length - 1;

    while(l <= r) {
        int m = (l + r) / 2; /* l + (r-l)/2 */

        if(target < array[m])
            r = m - 1;
        else if(target > array[m])
            l = m + 1;
        else
            return m;
    }

    return -1;
}

/* use binary search to find the index of an entry in a surgescript_classid_t[] */
int binary_search_object_class_id(surgescript_objectclassid_t target, const surgescript_objectclassid_t* array, size_t length)
{
    int l = 0, r = (int)length - 1;

    while(l <= r) {
        int m = (l + r) / 2; /* l + (r-l)/2 */

        if(target < array[m])
            r = m - 1;
        else if(target > array[m])
            l = m + 1;
        else
            return m;
    }

    return -1;
}

/* create a signaldata_t */
signaldata_t* construct_signal_data(surgescript_signalcode_t signal_code, surgescript_signaltype_t signal_type, const char* signal_name, char* const* field_names)
{
    signaldata_t* signal = ssmalloc(sizeof *signal);

    signal->code = signal_code;
    signal->type = signal_type;
    signal->name = ssstrdup(signal_name);
    signal->field_names = clone_list_of_strings(field_names);
    ssarray_init(signal->subscribers);
    ssarray_init(signal->cached_subscribers);

    return signal;
}

/* create a signaledclass_t */
signaledclass_t* construct_signaled_class(surgescript_objectclassid_t object_class)
{
    signaledclass_t* signaled_class = ssmalloc(sizeof *signaled_class);

    signaled_class->object_class = object_class;
    ssarray_init(signaled_class->emitted_signals);
    ssarray_init(signaled_class->caught_signals);
    ssarray_init(signaled_class->instances);
    ssarray_init(signaled_class->dirty_instances);
    signaled_class->catches_global_signal = false;

    return signaled_class;
}

/* a fasthash destructor for signaldata_t */
void destruct_signal_data(void* ptr)
{
    signaldata_t* signal = (signaldata_t*)ptr;

    ssarray_release(signal->cached_subscribers);
    ssarray_release(signal->subscribers);
    release_list_of_strings(signal->field_names);
    ssfree(signal->name);
    ssfree(signal);
}

/* a fasthash destructor for signaledclass_t */
void destruct_signaled_class(void* ptr)
{
    signaledclass_t* signaled_class = (signaledclass_t*)ptr;

    ssarray_release(signaled_class->dirty_instances);
    ssarray_release(signaled_class->instances);
    ssarray_release(signaled_class->caught_signals);
    ssarray_release(signaled_class->emitted_signals);
    ssfree(signaled_class);
}

/* helper function */
void foreach_own_program(const char* program_name, void* data)
{
    void** params = (void**)data;
    bool* has_wanted_shape = (bool*)(params[2]);

    /* skip if the tested failed previously */
    if(!(*has_wanted_shape))
        return;

    /* skip if it's not a getter */
    else if(strncmp(program_name, "get_", 4) != 0)
        return;

    /* skip if it's __file */
    else if(strcmp(program_name, "get___file") == 0)
        return;

    char* const* field_names = (char* const*)(params[0]);
    int list_size = *((const int*)(params[1]));

    /* the getter name must belong to the list of fields
       program_name + 4 is a valid pointer, since program_name[0..3] is "get_" */
    int j = index_of_string(program_name + 4, field_names, list_size);
    *has_wanted_shape = (j >= 0);
}

/* check if a signal context object matches the required shape for the given signal code */
bool is_valid_signal_context(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code, surgescript_objecthandle_t signal_context)
{
    /* we'll validate the shape of the signal context */
    const signaldata_t* signal = fasthash_get(signal_system->signals, signal_code);

    /* no such signal */
    if(signal == NULL)
        return false;

    /* if there are no declared fields, then only null is acceptable as a signal context */
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(signal_system->object_manager);
    if(signal->field_names == NULL)
        return signal_context == null_handle;
    else if(signal_context == null_handle) /* if there are declared fields, then null is not acceptable as a signal context */
        return false;

    /* for each declared field of the signal, check if the object has a corresponding getter */
    const surgescript_object_t* context_instance = surgescript_objectmanager_get(signal_system->object_manager, signal_context);
    char getter_name[4 + SS_NAMEMAX + 1] = "get_";

    for(char* const* it = signal->field_names; *it; it++) {
        const char* field_name = *it;
        surgescript_util_strncpy(getter_name + 4, field_name, sizeof(getter_name) - 4);
        if(!surgescript_object_has_own_function(context_instance, getter_name))
            return false;
    }

    /* for each (own) getter of the object, excluding __file, check if it's declared as a field of the signal */
    surgescript_programpool_t* pool = surgescript_objectmanager_programpool(signal_system->object_manager);
    const char* context_instance_name = surgescript_object_name(context_instance);
    bool has_wanted_shape = true;
    int list_size = length_of_list_of_strings(signal->field_names);
    void* params[3] = { signal->field_names, &list_size, &has_wanted_shape };
    surgescript_programpool_foreach_ex(pool, context_instance_name, params, foreach_own_program);

    if(!has_wanted_shape)
        return false;

    /* the shape is valid */
    return true;
}

/* cached version of is_valid_signal_context() */
bool is_valid_signal_context_cached(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code, surgescript_objecthandle_t signal_context)
{
    /* we'll validate the shape of the signal context */
    const signaldata_t* signal = fasthash_get(signal_system->signals, signal_code);

    /* no such signal */
    if(signal == NULL)
        return false;

    /* if there are no declared fields, then only null is acceptable as a signal context */
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(signal_system->object_manager);
    if(signal->field_names == NULL)
        return signal_context == null_handle;
    else if(signal_context == null_handle) /* if there are declared fields, then null is not acceptable as a signal context */
        return false;

    /* check if there is a cached result */
    const surgescript_object_t* context_instance = surgescript_objectmanager_get(signal_system->object_manager, signal_context);
    surgescript_objectclassid_t context_class = surgescript_object_class_id(context_instance);
    uint64_t key = ((uint64_t)signal_code << 32) | (uint64_t)context_class;
    SS_STATIC_ASSERT(sizeof(surgescript_signalcode_t) + sizeof(surgescript_objectclassid_t) == sizeof(uint64_t));

    const bool* cached_result = fasthash_get(signal_system->context_validity, key);
    if(cached_result != NULL)
        return *cached_result;

    /* if there is no cached result, then validate and cache the result */
    static const bool TRUE_RESULT = true, FALSE_RESULT = false;
    bool is_valid = is_valid_signal_context(signal_system, signal_code, signal_context);
    const bool* result = is_valid ? &TRUE_RESULT : &FALSE_RESULT;
    fasthash_put(signal_system->context_validity, key, (void*)result);
    return is_valid;
}