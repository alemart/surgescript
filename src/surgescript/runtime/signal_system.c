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
#include "../util/perfect_hash.h"
#include "../util/ssarray.h"
#include "../util/util.h"
#include "../util/xxh.h"

#define FASTHASH_INLINE
#include "../util/fasthash.h"

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

typedef struct signalcode_list_t signalcode_list_t;
typedef struct objecthandle_list_t objecthandle_list_t;
typedef struct objectclassid_list_t objectclassid_list_t;
typedef struct signalspec_t signalspec_t;

struct signalcode_list_t
{
    SSARRAY(surgescript_signalcode_t, signal_code);
};
struct objectclassid_list_t
{
    SSARRAY(surgescript_objectclassid_t, class_id);
};

struct objecthandle_list_t
{
    SSARRAY(surgescript_objecthandle_t, handle);
};

struct signalspec_t
{
    surgescript_signalcode_t signal_code;
    surgescript_signaltype_t signal_type;
    char* signal_name;
    char** field_names; /* NULL-terminated array */
};

struct surgescript_signalsystem_t
{
    surgescript_perfecthashseed_t hash_seed; /* used to compute signal codes */
    const surgescript_objectmanager_t* object_manager; /* reference to the object manager */

    fasthash_t* specifications; /* specifications indexed by signal codes */
    fasthash_t* emitted_signal_codes_per_object_class; /* for each object class, we store a list of the emitted signals */
    fasthash_t* caught_signal_codes_per_object_class; /* for each object class, we store a list of the caught signals */

    fasthash_t* subscribers_per_signal_code; /* for each global signal, we store a list of subscribers as class IDs */
    fasthash_t* instances_per_object_class; /* for each object class that catches global signals, we store a list of object instances */
    fasthash_t* instances_scheduled_for_cleanup; /* for each object class that catches global signals, we store a list of object instances scheduled for cleanup */
    SSARRAY(int, dirty_indices); /* auxiliary array for cleaning up */
};

static surgescript_perfecthashkey_t hash_of_signal_name(const char* signal_name, surgescript_perfecthashseed_t seed);
static int index_of_signal_code(surgescript_signalcode_t signal_code, const surgescript_signalcode_t* array, size_t length);
static int index_of_object_handle(surgescript_objecthandle_t handle, const surgescript_objecthandle_t* array, size_t length);
static void release_specification(void* ptr);
static void release_signalcode_list(void* ptr);
static void release_objectclassid_list(void* ptr);
static void release_objecthandle_list(void* ptr);
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
    signal_system->specifications = fasthash_create(release_specification, LG2_INITIAL_CAPACITY(LG2_MAX_SIGNAL_CLASSES));
    signal_system->emitted_signal_codes_per_object_class = fasthash_create(release_signalcode_list, LG2_INITIAL_CAPACITY(LG2_MAX_OBJECT_CLASSES));
    signal_system->caught_signal_codes_per_object_class = fasthash_create(release_signalcode_list, LG2_INITIAL_CAPACITY(LG2_MAX_OBJECT_CLASSES));
    signal_system->subscribers_per_signal_code = fasthash_create(release_objectclassid_list, LG2_INITIAL_CAPACITY(LG2_MAX_SIGNAL_CLASSES));
    signal_system->instances_per_object_class = fasthash_create(release_objecthandle_list, LG2_INITIAL_CAPACITY(LG2_MAX_OBJECT_CLASSES));
    signal_system->instances_scheduled_for_cleanup = fasthash_create(release_objecthandle_list, LG2_INITIAL_CAPACITY(LG2_MAX_OBJECT_CLASSES));
    ssarray_init(signal_system->dirty_indices);

    /* ----- part one ----- */

    /* store the specification of each signal */
    for(int i = 0; i < ssarray_length(builder->declarations.signal_name); i++) {
        char* signal_name = ssstrdup(builder->declarations.signal_name[i]);
        char** field_names = clone_list(builder->declarations.field_names[i]);
        surgescript_signaltype_t signal_type = builder->declarations.signal_type[i];
        surgescript_signalcode_t signal_code = surgescript_signalsystem_signal_code(signal_system, signal_name);

        ssassert(NULL == fasthash_get(signal_system->specifications, signal_code)); /* no duplicates allowed */

        signalspec_t* spec = ssmalloc(sizeof *spec);
        spec->signal_name = signal_name;
        spec->field_names = field_names;
        spec->signal_type = signal_type;
        spec->signal_code = signal_code;

        fasthash_put(signal_system->specifications, signal_code, spec);
    }

    /* for each object class, store the emitted signal codes */
    for(int i = 0; i < ssarray_length(builder->emissions.object_name); i++) {
        const char* object_name = builder->emissions.object_name[i];
        surgescript_objectclassid_t class_id = surgescript_objectmanager_class_id(object_manager, object_name);

        char* const* signal_names = builder->emissions.signal_names[i];
        for(char* const* it = signal_names; *it; it++) {
            const char* signal_name = *it;
            surgescript_signalcode_t signal_code = surgescript_signalsystem_signal_code(signal_system, signal_name);

            void* ptr = fasthash_get(signal_system->emitted_signal_codes_per_object_class, signal_code);
            if(ptr == NULL) {
                signalcode_list_t* list = ssmalloc(sizeof *list);
                ssarray_init(list->signal_code);
                ssarray_push(list->signal_code, signal_code);
                fasthash_put(signal_system->emitted_signal_codes_per_object_class, signal_code, list);
            }
            else {
                signalcode_list_t* list = (signalcode_list_t*)ptr;
                ssarray_push(list->signal_code, signal_code);
            }
        }
    }

    /* for each object class, store the caught signal codes */
    for(int i = 0; i < ssarray_length(builder->handlers.object_name); i++) {
        const char* object_name = builder->handlers.object_name[i];
        surgescript_objectclassid_t class_id = surgescript_objectmanager_class_id(object_manager, object_name);

        char* const* signal_names = builder->handlers.signal_names[i];
        for(char* const* it = signal_names; *it; it++) {
            const char* signal_name = *it;
            surgescript_signalcode_t signal_code = surgescript_signalsystem_signal_code(signal_system, signal_name);

            void* ptr = fasthash_get(signal_system->caught_signal_codes_per_object_class, signal_code);
            if(ptr == NULL) {
                signalcode_list_t* list = ssmalloc(sizeof *list);
                ssarray_init(list->signal_code);
                ssarray_push(list->signal_code, signal_code);
                fasthash_put(signal_system->caught_signal_codes_per_object_class, signal_code, list);
            }
            else {
                signalcode_list_t* list = (signalcode_list_t*)ptr;
                ssarray_push(list->signal_code, signal_code);
            }
        }
    }

    /* ----- part two ----- */

    /* for each global signal code, store the list of subscribers: class IDs that can catch it */
    for(int i = 0; i < ssarray_length(builder->declarations.signal_name); i++) {
        surgescript_signaltype_t signal_type = builder->declarations.signal_type[i];

        /* for each signal code, check if it's global */
        if(signal_type == SIGTYPE_GLOBAL) {
            const char* signal_name = builder->declarations.signal_name[i];
            surgescript_signalcode_t signal_code = surgescript_signalsystem_signal_code(signal_system, signal_name);

            ssassert(NULL == fasthash_get(signal_system->subscribers_per_signal_code, signal_code)); /* no duplicates allowed */

            /* first, let's create an empty list for that global signal */
            objectclassid_list_t* list = ssmalloc(sizeof *list);
            ssarray_init(list->class_id);
            fasthash_put(signal_system->subscribers_per_signal_code, signal_code, list);

            /* second, let's register all class IDs that can catch it */
            for(int j = 0; j < ssarray_length(builder->handlers.object_name); j++) {
                const char* object_name = builder->handlers.object_name[j];
                surgescript_objectclassid_t class_id = surgescript_objectmanager_class_id(object_manager, object_name);

                /* we assume that this function can be used at this point in time;
                   hence this block of code belongs to "part two" */
                if(surgescript_signalsystem_object_can_catch(signal_system, signal_code, class_id)) {

                    /* we found a match */
                    ssarray_push(list->class_id, class_id);

                }
            }
        }
    }

    /* for each object class that catches global signals, store empty lists of object instances */
    for(int i = 0; i < ssarray_length(builder->handlers.object_name); i++) {
        bool catches_global_signal = false;

        /* for all signals that objects named object_name[i] catch, check if any of them is global */
        char* const* signal_names = builder->handlers.signal_names[i];
        for(char* const* it = signal_names; *it; it++) {
            const char* signal_name = *it;
            surgescript_signalcode_t signal_code = surgescript_signalsystem_signal_code(signal_system, signal_name);

            /* we assume that this function can be used at this point in time;
               hence this block of code belongs to "part two" */
            if(surgescript_signalsystem_signal_type(signal_system, signal_code) == SIGTYPE_GLOBAL) {
                catches_global_signal = true;
                break;
            }
        }

        /* if any of them is global, then objects named object_name[i] catch at least one global signal */
        if(catches_global_signal) {
            const char* object_name = builder->handlers.object_name[i];
            surgescript_objectclassid_t class_id = surgescript_objectmanager_class_id(object_manager, object_name);

            /* for each applicable hash table, store an empty list of object instances that catch global signals (per object class) */
            fasthash_t* hashtables[2] = { signal_system->instances_per_object_class, signal_system->instances_scheduled_for_cleanup };
            for(int j = 0; j < 2; j++) {
                fasthash_t* hashtable = hashtables[j];
                ssassert(NULL == fasthash_get(hashtable, class_id)); /* no duplicates allowed */

                objecthandle_list_t* list = ssmalloc(sizeof *list);
                ssarray_init(list->handle);
                fasthash_put(hashtable, class_id, list);
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
    ssarray_release(signal_system->dirty_indices);
    fasthash_destroy(signal_system->instances_scheduled_for_cleanup);
    fasthash_destroy(signal_system->instances_per_object_class);
    fasthash_destroy(signal_system->subscribers_per_signal_code);
    fasthash_destroy(signal_system->caught_signal_codes_per_object_class);
    fasthash_destroy(signal_system->emitted_signal_codes_per_object_class);
    fasthash_destroy(signal_system->specifications);
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
    const signalspec_t* spec = (const signalspec_t*)fasthash_get(signal_system->specifications, signal_code);

    /* no such signal? */
    if(spec == NULL)
        return SIGTYPE_NULL;

    /* return the signal type */
    return spec->signal_type;
}

/*
 * surgescript_signalsystem_object_can_emit()
 * Check if a class of objects can emit a given class of signals
 */
bool surgescript_signalsystem_object_can_emit(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code, surgescript_objectclassid_t class_id)
{
    const signalcode_list_t* list = (const signalcode_list_t*)fasthash_get(signal_system->emitted_signal_codes_per_object_class, class_id);

    /* no emitted signals for that class id, or no such class id? */
    if(list == NULL)
        return false;

    /* check if signal_code is in the list */
    return index_of_signal_code(signal_code, list->signal_code, ssarray_length(list->signal_code)) >= 0;
}

/*
 * surgescript_signalsystem_object_can_catch()
 * Check if a class of objects has a handler to catch a given class of signals
 */
bool surgescript_signalsystem_object_can_catch(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code, surgescript_objectclassid_t class_id)
{
    const signalcode_list_t* list = (const signalcode_list_t*)fasthash_get(signal_system->caught_signal_codes_per_object_class, class_id);

    /* no caught signals for that class id, or no such class id? */
    if(list == NULL)
        return false;

    /* check if signal_code is in the list */
    return index_of_signal_code(signal_code, list->signal_code, ssarray_length(list->signal_code)) >= 0;
}

/*
 * surgescript_signalsystem_object_has_handlers()
 * Check if a class of objects has one or more signal handlers (i.e., can catch signals)
 */
bool surgescript_signalsystem_object_has_handlers(const surgescript_signalsystem_t* signal_system, surgescript_objectclassid_t class_id)
{
    /* This operation must be fast! It's used in subscribe & unsubscribe */
    const signalcode_list_t* list = (const signalcode_list_t*)fasthash_get(signal_system->caught_signal_codes_per_object_class, class_id);
    return (list != NULL) && (ssarray_length(list->signal_code) > 0);
}

/*
 * surgescript_signalsystem_signal_has_subscribers()
 * Check if a class of signals has one or more subscribers
 */
bool surgescript_signalsystem_signal_has_subscribers(const surgescript_signalsystem_t* signal_system, surgescript_signalcode_t signal_code)
{
    const objecthandle_list_t* list = (const objecthandle_list_t*)fasthash_get(signal_system->subscribers_per_signal_code, signal_code);
    return (list != NULL) && (ssarray_length(list->handle) > 0);
}

/*
 * surgescript_signalsystem_subscribe()
 * Subscribe an object to all global signals that it can catch
 */
void surgescript_signalsystem_subscribe(surgescript_signalsystem_t* signal_system, const surgescript_object_t* object)
{
    /* This operation must be fast! It's used when spawning objects */
    surgescript_objectclassid_t class_id = surgescript_object_class_id(object);

    /* if this object has no signal handlers, we don't need to subscribe it, as there is nothing to do */
    if(!surgescript_signalsystem_object_has_handlers(signal_system, class_id))
        return;

    // TODO
    //printf("Sub %u\n", handle);
}

/*
 * surgescript_signalsystem_unsubscribe()
 * Unsubscribe an object from all global signals that it can catch
 */
void surgescript_signalsystem_unsubscribe(surgescript_signalsystem_t* signal_system, const surgescript_object_t* object)
{
    /* This operation must be fast! It's used when deleting objects */
    surgescript_objectclassid_t class_id = surgescript_object_class_id(object);

    /* nothing to do */
    if(!surgescript_signalsystem_object_has_handlers(signal_system, class_id))
        return;

    // TODO
    //printf("Unsub %u\n", handle);
}

/*
 * surgescript_signalsystem_emit_signal()
 * Emit a signal
 */
void surgescript_signalsystem_emit_signal(const surgescript_signalsystem_t* signal_system, surgescript_objecthandle_t emitter, surgescript_signalcode_t signal_code, surgescript_objecthandle_t signal_context)
{
    // TODO
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

/* hash of signal name */
surgescript_perfecthashkey_t hash_of_signal_name(const char* signal_name, surgescript_perfecthashseed_t seed)
{
    return XXH(signal_name, strlen(signal_name), seed);
}

/* find the index of an entry in a surgescript_signalcode_t[] */
int index_of_signal_code(surgescript_signalcode_t signal_code, const surgescript_signalcode_t* array, size_t length)
{
    while(length--) {
        if(signal_code == array[length])
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

/* a fasthash destructor for signalspec_t */
void release_specification(void* ptr)
{
    signalspec_t* spec = (signalspec_t*)ptr;
    ssfree(spec->signal_name);
    release_list_of_strings(spec->field_names);
    ssfree(spec);
}

/* a fasthash destructor for signalcode_list_t */
void release_signalcode_list(void* ptr)
{
    signalcode_list_t* list = (signalcode_list_t*)ptr;
    ssarray_release(list->signal_code);
    ssfree(list);
}

/* a fasthash destructor for objectclassid_list_t */
void release_objectclassid_list(void* ptr)
{
    objectclassid_list_t* list = (objectclassid_list_t*)ptr;
    ssarray_release(list->class_id);
    ssfree(list);
}

/* a fasthash destructor for objecthandle_list_t */
void release_objecthandle_list(void* ptr)
{
    objecthandle_list_t* list = (objecthandle_list_t*)ptr;
    ssarray_release(list->handle);
    ssfree(list);
}

