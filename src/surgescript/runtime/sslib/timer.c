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
 * runtime/sslib/timer.c
 * SurgeScript standard library: Timer component
 */

#include "../vm.h"
#include "../heap.h"
#include "../object.h"
#include "../object_manager.h"
#include "../signal_system.h"
#include "../../util/util.h"

/* private stuff */
static surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_start(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_stop(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_getwaittime(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setwaittime(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getpaused(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setpaused(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getautostart(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setautostart(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getrepeat(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_setrepeat(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

static surgescript_var_t* fun_gettimeleft(surgescript_object_t* object, const surgescript_var_t** param, int num_params);
static surgescript_var_t* fun_getstopped(surgescript_object_t* object, const surgescript_var_t** param, int num_params);

/* the internal state of a timer */
typedef struct timerstate_t {
    double elapsed_time;
    double wait_time;
    bool is_stopped;
    bool is_paused;
    bool repeat;
    bool autostart;
    bool is_first_run;
} timerstate_t;

static const timerstate_t INITIAL_STATE = {
    .elapsed_time = 0.0, /* measured in seconds */
    .wait_time = 1.0, /* one second */
    .is_stopped = true,
    .is_paused = false,
    .repeat = true,
    .autostart = false,
    .is_first_run = true
};

/* signals */
enum { TIMEOUT, SIGNAL_COUNT };
static surgescript_signalcode_t signal_code[SIGNAL_COUNT] = { 0 };
static const char* SIGNAL_NAME[SIGNAL_COUNT] = {
    [TIMEOUT] = "timer timeout"
};

/* misc */
static surgescript_objecthandle_t time_handle = 0; /* reference to Time */
static bool is_class_initialized = false;
static void initialize_class(surgescript_object_t* object);

/*
 * surgescript_sslib_register_timer()
 * Register methods
 */
void surgescript_sslib_register_timer(surgescript_vm_t* vm)
{
    surgescript_vm_bind(vm, "Timer", "constructor", fun_constructor, 0);
    surgescript_vm_bind(vm, "Timer", "destructor", fun_destructor, 0);
    surgescript_vm_bind(vm, "Timer", "state:main", fun_main, 0);

    surgescript_vm_bind(vm, "Timer", "start", fun_start, 0);
    surgescript_vm_bind(vm, "Timer", "stop", fun_stop, 0);

    surgescript_vm_bind(vm, "Timer", "get_waitTime", fun_getwaittime, 0);
    surgescript_vm_bind(vm, "Timer", "set_waitTime", fun_setwaittime, 1);
    surgescript_vm_bind(vm, "Timer", "get_paused", fun_getpaused, 0);
    surgescript_vm_bind(vm, "Timer", "set_paused", fun_setpaused, 1);
    surgescript_vm_bind(vm, "Timer", "get_autoStart", fun_getautostart, 0);
    surgescript_vm_bind(vm, "Timer", "set_autoStart", fun_setautostart, 1);
    surgescript_vm_bind(vm, "Timer", "get_repeat", fun_getrepeat, 0);
    surgescript_vm_bind(vm, "Timer", "set_repeat", fun_setrepeat, 1);

    surgescript_vm_bind(vm, "Timer", "get_timeLeft", fun_gettimeleft, 0);
    surgescript_vm_bind(vm, "Timer", "get_stopped", fun_getstopped, 0);

    surgescript_vm_register_signal(vm, SIGNAL_NAME[TIMEOUT], SIGTYPE_LOCAL, NULL);
    surgescript_vm_register_signal_emissions(vm, "Timer", SIGNAL_NAME[TIMEOUT], NULL);

    is_class_initialized = false;
}

/* this initialization routine runs only once */
void initialize_class(surgescript_object_t* object)
{
    /* cache a reference to the Time object */
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    time_handle = surgescript_objectmanager_system_object(manager, "Time");

    /* cache the signal codes */
    const surgescript_signalsystem_t* signal_system = surgescript_objectmanager_signalsystem(manager);
    for(int s = 0; s < SIGNAL_COUNT; s++)
        signal_code[s] = surgescript_signalsystem_signal_code(signal_system, SIGNAL_NAME[s]);
}



/* my functions */

/* constructor */
surgescript_var_t* fun_constructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    /* initialize the class */
    if(!is_class_initialized) {
        initialize_class(object);
        is_class_initialized = true;
    }

    /* initialize the internal state of this instance */
    timerstate_t* state = ssmalloc(sizeof *state);
    *state = INITIAL_STATE;
    surgescript_object_set_userdata(object, state);

    /* done */
    return NULL;
}

/* destructor */
surgescript_var_t* fun_destructor(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    timerstate_t* state = surgescript_object_userdata(object);
    ssfree(state);
    return NULL;
}

/* main state */
surgescript_var_t* fun_main(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    timerstate_t* state = surgescript_object_userdata(object);

    /* Autostart the timer. The internal state is assumed to be INITIAL_STATE.
       We can't autostart it in the constructor because the autostart option
       may or may not be set after the constructor is executed, and before
       this main state runs for the first time */
    if(state->is_first_run) {
        state->is_first_run = false;
        if(state->autostart)
            surgescript_object_call_function(object, "start", NULL, 0, NULL);
    }

    /* do nothing if the timer is paused or stopped */
    if(state->is_stopped || state->is_paused)
        return NULL;

    /* increment the time: elapsed += Time.delta */
    const surgescript_objectmanager_t* manager = surgescript_object_manager(object);
    surgescript_object_t* time_object = surgescript_objectmanager_get(manager, time_handle);
    surgescript_var_t* delta = surgescript_var_create();
    surgescript_object_call_function(time_object, "get_delta", NULL, 0, delta);
    state->elapsed_time += surgescript_var_get_number(delta);
    surgescript_var_destroy(delta);

    /* do nothing else if the timer has not reached the end */
    if(state->elapsed_time < state->wait_time)
        return NULL;

    /* otherwise, emit a timeout signal */
    surgescript_objecthandle_t null_handle = surgescript_objectmanager_null(manager);
    surgescript_object_emit_signal(object, signal_code[TIMEOUT], null_handle);

    /* stop or restart the timer */
    if(state->repeat)
        state->elapsed_time -= state->wait_time;
    else
        state->is_stopped = true;

    /* done */
    return NULL;
}

/* start the timer, or reset it if it was already started. This operation does not resume a paused timer. */
surgescript_var_t* fun_start(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    timerstate_t* state = surgescript_object_userdata(object);
    state->elapsed_time = 0.0;
    state->is_stopped = false;
    return NULL;
}

/* stop the timer. A stopped timer can only be restarted, unlike a paused timer. */
surgescript_var_t* fun_stop(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    timerstate_t* state = surgescript_object_userdata(object);
    state->is_stopped = true;
    return NULL;
}

/* (getter) autostart: if true, the timer will start automatically just after being spawned, */
surgescript_var_t* fun_getautostart(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const timerstate_t* state = surgescript_object_userdata(object);
    return surgescript_var_set_bool(surgescript_var_create(), state->autostart);
}

/* (setter) autostart: if true, the timer will start automatically just after being spawned. */
surgescript_var_t* fun_setautostart(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    timerstate_t* state = surgescript_object_userdata(object);
    bool autostart = surgescript_var_get_bool(param[0]);
    state->autostart = autostart;
    return NULL;
}

/* (getter) paused: if true, the timer is paused. A paused timer does not emit timeout signals until it's unpaused. */
surgescript_var_t* fun_getpaused(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const timerstate_t* state = surgescript_object_userdata(object);
    return surgescript_var_set_bool(surgescript_var_create(), state->is_paused);
}

/* (setter) paused: if true, the timer is paused. A paused timer does not emit timeout signals until it's unpaused. */
surgescript_var_t* fun_setpaused(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    timerstate_t* state = surgescript_object_userdata(object);
    bool is_paused = surgescript_var_get_bool(param[0]);
    state->is_paused = is_paused;
    return NULL;
}

/* (getter) waitTime: the time, in seconds, required for the timer to end. */
surgescript_var_t* fun_getwaittime(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const timerstate_t* state = surgescript_object_userdata(object);
    return surgescript_var_set_number(surgescript_var_create(), state->wait_time);
}

/* (setter) waitTime: the time, in seconds, required for the timer to end. */
surgescript_var_t* fun_setwaittime(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    timerstate_t* state = surgescript_object_userdata(object);
    double wait_time = surgescript_var_get_number(param[0]);
    state->wait_time = wait_time > 0.0 ? wait_time : 0.0;
    return NULL;
}

/* (getter) repeat: if true, the timer will restart automatically after a timeout. Otherwise, it will stop. */
surgescript_var_t* fun_getrepeat(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const timerstate_t* state = surgescript_object_userdata(object);
    return surgescript_var_set_bool(surgescript_var_create(), state->repeat);
}

/* (setter) repeat: if true, the timer will restart automatically after a timeout. Otherwise, it will stop. */
surgescript_var_t* fun_setrepeat(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    timerstate_t* state = surgescript_object_userdata(object);
    bool repeat = surgescript_var_get_bool(param[0]);
    state->repeat = repeat;
    return NULL;
}

/* (getter, read-only) timeLeft: the remaining time, in seconds, of the timer. If the timer is stopped, this is always zero. */
surgescript_var_t* fun_gettimeleft(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const timerstate_t* state = surgescript_object_userdata(object);
    double time_left = state->elapsed_time < state->wait_time && !state->is_stopped ? state->wait_time - state->elapsed_time : 0.0;
    return surgescript_var_set_number(surgescript_var_create(), time_left);
}

/* (getter, read-only) stopped: true if the timer is stopped or has not started. A stopped timer does not emit timeout signals. */
surgescript_var_t* fun_getstopped(surgescript_object_t* object, const surgescript_var_t** param, int num_params)
{
    const timerstate_t* state = surgescript_object_userdata(object);
    return surgescript_var_set_bool(surgescript_var_create(), state->is_stopped);
}