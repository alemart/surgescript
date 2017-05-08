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
    surgescript_vm_t* vm = surgescript_vm_create();
    surgescript_programpool_t* program_pool = surgescript_vm_programpool(vm);
    surgescript_parser_t* parser = surgescript_parser_create(program_pool);
    const char* file = argc > 1 ? argv[1] : "";
    bool success;

    /* do we have an input file? */
    if(!*file) {
        printf("SurgeScript\n");
        printf("Usage: %s input-script.ss\n", argv[0]);
        return 1;
    }

    /* parse & compile the input file */
    success = surgescript_parser_parsefile(parser, file);
    if(!success) {
        printf("Error when parsing \"%s\".", file);
        return 1;
    }
    surgescript_parser_destroy(parser);

    /* debug */
    /*surgescript_program_dump(surgescript_programpool_get(program_pool, "Application", "__ssconstructor"), stdout);
    surgescript_program_dump(surgescript_programpool_get(program_pool, "Application", "state:main"), stdout);*/

    /* run the script */
    surgescript_vm_launch(vm);
    while(surgescript_vm_update(vm)) {
        ;
    }
    surgescript_vm_destroy(vm);

    /* done! */
    return 0;
}