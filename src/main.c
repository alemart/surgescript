/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2021 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * runtime/main.c
 * SurgeScript CLI
 */

#include <surgescript.h>
#include <string.h>
#include <stdio.h>

/* multithread support */
#if ENABLE_THREADS
# if !__STDC_NO_THREADS__
#  include <threads.h>
# else
#  error "Can't compile the SurgeScript CLI: threads.h is not found on this environment. Please change the environment or disable multithreading."
# endif
#endif

static surgescript_vm_t* make_vm(int argc, char** argv, uint64_t* time_limit);
static void run_vm(surgescript_vm_t* vm, uint64_t time_limit);
static void destroy_vm(surgescript_vm_t* vm);
static void print_to_stdout(const char* message);
static void print_to_stderr(const char* message);
static void discard_message(const char* message);
static void show_help(const char* executable);
static char* read_from_stdin();
static int main_loop(void* arg);

/* default time limit, given in milliseconds */
#define DEFAULT_TIME_LIMIT 30000

/*
 * main()
 * Entry point
 */
int main(int argc, char* argv[])
{
    uint64_t time_limit = DEFAULT_TIME_LIMIT;

    /* Create the VM and compile the input file(s) */
    surgescript_vm_t* vm = make_vm(argc, argv, &time_limit);

    /* got a VM? */
    if(vm != NULL) {
        /* run the VM */
        run_vm(vm, time_limit);

        /* destroy the VM */
        destroy_vm(vm);
    }

    /* done! */
    return 0;
}

/**
 * run_vm()
 * Run the VM with a time limit
 */
void run_vm(surgescript_vm_t* vm, uint64_t time_limit)
{
    uint64_t start_time = surgescript_util_gettickcount();
    #define show_time_limit_error() \
        fprintf(stderr, "Time limit of %.1lf seconds exceeded.\n", (double)time_limit * 0.001)

#if !ENABLE_THREADS

    /* main loop */
    while(surgescript_vm_update(vm)) {

        /* time limit */
        if(time_limit > 0 && surgescript_util_gettickcount() > start_time + time_limit) {
            show_time_limit_error();
            break;
        }

    }

#else

    /* run the SurgeScript VM on a separate thread */
    thrd_t thread;
    thrd_create(&thread, main_loop, vm);

    /* handle the time limit, if it's been set */
    if(time_limit > 0) {
        while(surgescript_vm_is_active(vm)) {
            if(surgescript_util_gettickcount() > start_time + time_limit) {
                show_time_limit_error();
                exit(1); /* TODO we should kill the other thread instead */
            }

            thrd_yield();
        }
    }

    /* wait for the other thread to complete */
    thrd_join(thread, NULL);

#endif

}

/**
 * destroy_vm()
 * Destroy a SurgeScript VM
 */
void destroy_vm(surgescript_vm_t* vm)
{
    surgescript_vm_destroy(vm);
}

/**
 * main_loop()
 * Game loop for multithreaded execution
 */
int main_loop(void* arg)
{
#if !ENABLE_THREADS

    (void)arg;
    return 0;

#else

    surgescript_vm_t* vm = (surgescript_vm_t*)arg;

    while(surgescript_vm_update(vm)) {
        thrd_yield();
    }

    return 0;

#endif
}

/*
 * make_vm()
 * Parses the command line arguments and creates a VM
 * with the compiled scripts
 */
surgescript_vm_t* make_vm(int argc, char** argv, uint64_t* time_limit)
{
    surgescript_vm_t* vm = NULL;
    int i;

    /* disable debugging */
    surgescript_util_set_error_functions(discard_message, print_to_stderr);

    /* parse the command line options */
    for(i = 1; i < argc && *argv[i] == '-'; i++) {
        const char* arg = argv[i];
        if(strcmp(arg, "--debug") == 0 || strcmp(arg, "-D") == 0) {
            /* enable debugging */
            surgescript_util_set_error_functions(print_to_stdout, print_to_stderr);
        }
        else if(strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0) {
            /* display version */
            printf("%s\n", surgescript_util_version());
            return NULL;
        }
        else if(strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            /* show help */
            show_help(surgescript_util_basename(argv[0]));
            return NULL;
        }
        else if(strcmp(arg, "--timelimit") == 0 || strcmp(arg, "-t") == 0) {
            /* set time limit (maximum execution time) */
            if(++i < argc && time_limit != NULL) {
                double seconds = atof(argv[i]);
                *time_limit = (seconds > 0.0) ? (uint64_t)(seconds * 1000.0) : 0;
            }
        }
        else if(strcmp(arg, "--") == 0) {
            /* user-specific command line arguments */
            break;
        }
        else {
            /* unrecognized option */
            printf("Unrecognized option: '%s'.\nType '%s --help' for more information.\n", arg, surgescript_util_basename(argv[0]));
            return NULL;
        }
    }

    /* create an empty VM */
    vm = surgescript_vm_create();

    /* compile the scripts */
    if(i < argc && strcmp(argv[i], "--") != 0) {
        /* read files */
        for(; i < argc && strcmp(argv[i], "--") != 0; i++) {
            const char* file = argv[i];
            surgescript_vm_compile(vm, file);
        }
    }
    else {
        /* read from stdin */
        char* code = read_from_stdin();
        surgescript_vm_compile_code_in_memory(vm, code);
        ssfree(code);
    }

    /* launch the VM */
    if(i < argc && strcmp(argv[i], "--") == 0) {
        /* launch with user-specific command line arguments */
        ++i;
        surgescript_vm_launch_ex(vm, argc - i, (char**)(argv + i));
    }
    else {
        /* launch without user-specific command line arguments */
        surgescript_vm_launch(vm);
    }

    /* done! */
    return vm;
}

/*
 * show_help()
 * Shows a help message
 */
void show_help(const char* executable)
{
    printf(
        "SurgeScript version %s\n"
        "Copyright (C) %s %s\n"
        "%s\n"
        "\n"
        "Usage: %s [OPTIONS] <scripts>\n"
        "Compiles and executes the given script(s).\n"
        "\n"
        "Options:\n"
        "    -v, --version                         shows the version of SurgeScript\n"
        "    -D, --debug                           prints debugging information\n"
        "    -t, --timelimit                       sets a maximum execution time, in seconds (0 = no limit)\n"
        "    -h, --help                            shows this message\n"
        "\n"
        "Examples:\n"
        "    %s script.ss                 compiles and executes script.ss\n"
        "    %s file1.ss file2.ss         compiles and executes file1.ss and file2.ss\n"
        "    %s --debug test.ss           compiles and runs test.ss with debugging information\n"
        "    %s file.ss -- -x -y          passes custom arguments -x and -y to file.ss\n"
        "    %s -t 5                      runs a script read from stdin, with a time limit of 5 seconds\n"
        "\n"
        "Full documentation available at: <%s>\n",
        surgescript_util_version(),
        surgescript_util_year(),
        surgescript_util_authors(),
        surgescript_util_website(),
        executable,
        executable,
        executable,
        executable,
        executable,
        executable,
        surgescript_util_website()
    );
}

/*
 * print_to_stdout()
 * Prints a message to the standard output
 */
void print_to_stdout(const char* message)
{
    puts(message);
}

/*
 * print_to_stderr()
 * Writes a message to the standard error stream
 */
void print_to_stderr(const char* message)
{
    fprintf(stderr, "%s\n", message);
}

/*
 * discard_message()
 * Discards a message
 */
void discard_message(const char* message)
{
    ;
}

/**
 * read_from_stdin()
 * Read data from stdin and store it in a string
 */
char* read_from_stdin()
{
    const size_t BUFSIZE = 1024;
    char* data = NULL;
    size_t read_chars = 0, data_size = 0;

    /* read to data[] */
    do {
        data_size += BUFSIZE;
        data = ssrealloc(data, data_size + 1);
        read_chars += fread(data + read_chars, sizeof(char), BUFSIZE, stdin);
        data[read_chars] = '\0';
    } while(read_chars == data_size);

    /* done! */
    return data;
}