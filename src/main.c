/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016-2017  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/main.c
 * SurgeScript Runtime Engine entry point
 */

#include <stdio.h>
#include <float.h>
#include <math.h>
#include "util/util.h"
#include "runtime/vm.h"
#include "compiler/token.h"
#include "compiler/lexer.h"
#include "compiler/parser.h"

/*
 * main()
 * Entry point
 */
int main(int argc, char* argv[])
{
    const char* file = argc > 1 ? argv[1] : NULL;
    if(file) {
        /* spawn the VM and compile the input file */
        surgescript_vm_t* vm = surgescript_vm_create();
        surgescript_vm_compile(vm, file);

        /* run the VM */
        surgescript_vm_launch_ex(vm, argc, argv);
        while(surgescript_vm_update(vm)) {
            ;
        }
        surgescript_vm_destroy(vm);

        /* done! */
        return 0;
    }
    else {
        /* print usage */
        printf("%s\n", SSINFO);
        printf("Usage: %s input-script.ss\n", surgescript_util_basename(argv[0]));
        return 1;
    }
}