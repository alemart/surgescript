/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2020 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * SurgeScript Runtime Engine entry point
 */

#include <surgescript.h>
#include <string.h>
#include <stdio.h>

static surgescript_vm_t* make_vm(int argc, char** argv);
static void print_to_stdout(const char* message);
static void print_to_stderr(const char* message);
static void discard_message(const char* message);
static void show_help(const char* executable);

/*
 * main()
 * Entry point
 */
int main(int argc, char* argv[])
{
    if(argc > 1) {
        /* create the VM and compile the input file(s) */
        surgescript_vm_t* vm = make_vm(argc, argv);
        if(vm != NULL) {
            /* run the VM */
            while(surgescript_vm_update(vm)) {
                ;
            }

            /* destroy the VM */
            surgescript_vm_destroy(vm);
        }
    }
    else {
        /* print usage */
        show_help(surgescript_util_basename(argv[0]));
    }

    /* done! */
    return 0;
}

/*
 * make_vm()
 * Parses the command line arguments and creates a VM
 * with the compiled scripts
 */
surgescript_vm_t* make_vm(int argc, char** argv)
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
        else {
            printf("Unrecognized option: '%s'.\nType '%s --help' for more information.\n", arg, surgescript_util_basename(argv[0]));
            return NULL;
        }
    }

    /* create an empty VM */
    vm = surgescript_vm_create();

    /* compile the scripts */
    for(; i < argc && strcmp(argv[i], "--") != 0; i++) {
        const char* file = argv[i];
        surgescript_vm_compile(vm, file);
    }

    /* launch the VM */
    surgescript_vm_launch_ex(vm, argc, argv);

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
        "    -h, --help                            shows this message\n"
        "\n"
        "Examples:\n"
        "    %s script.ss                 compiles and executes script.ss\n"
        "    %s file1.ss file2.ss         compiles and executes file1.ss and file2.ss\n"
        "    %s --debug test.ss           compiles and runs test.ss with debugging information\n"
        "    %s file.ss -- -x -y          passes custom arguments -x and -y to file.ss\n"
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