/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/main.c
 * SurgeScript entry point
 */

/* test file */
#include <stdio.h>
#include "util/util.h"
#include "util/ssarray.h"
#include "runtime/variable.h"
#include "runtime/heap.h"
#include "runtime/stack.h"
#include "runtime/program.h"
#include "runtime/object.h"
#include "runtime/program_pool.h"
#include "runtime/object_manager.h"

void setup(surgescript_program_t* program, surgescript_program_t* called_program)
{
    surgescript_program_label_t loop = surgescript_program_new_label(program);
    surgescript_program_label_t loop2 = surgescript_program_new_label(program);

    surgescript_program_add_text(program, "Contador:");
    surgescript_program_add_text(program, "fun2");

    // hello
    surgescript_program_add_line(program, SSOP_ASSIGN_STRING, SSOP(0), SSOP(0), SSNOP); // t[0] = text[0]
    surgescript_program_add_line(program, SSOP_OUT, SSOP(0), SSNOP, SSNOP);  // print t[0]

/*
    // count 1 .. 10
    surgescript_program_add_line(program, SSOP_ZERO, SSOP(0), SSNOP, SSNOP); // t[0] = 0
    surgescript_program_add_line(program, SSOP_ASSIGN_NUMBER, SSOP(1), SSOPf(10), SSNOP); // t[1] = 10
    surgescript_program_add_label(program, loop);
        surgescript_program_add_line(program, SSOP_INC, SSOP(0), SSNOP, SSNOP); // t[0] += 1
        surgescript_program_add_line(program, SSOP_OUT, SSOP(0), SSNOP, SSNOP);  // print t[0]
        surgescript_program_add_line(program, SSOP_JMP_IF_LOWER, SSOP(loop), SSOP(0), SSOP(1)); // jmp loop if t[0] < t[1]

    // print strlen(text[1])
    surgescript_program_add_line(program, SSOP_ASSIGN_STRING, SSOP(0), SSOP(1), SSNOP); // t[0] = text[1]
    surgescript_program_add_line(program, SSOP_STRLEN, SSOP(0), SSNOP, SSNOP); // t[0] = strlen(t[0])
    surgescript_program_add_line(program, SSOP_OUT, SSOP(0), SSNOP, SSNOP);  // print t[0]
*/
    // print fib(1) .. fib(10), where fib(x) = fib(x-1) + fib(x-2)
    surgescript_program_add_line(program, SSOP_ZERO, SSOP(0), SSNOP, SSNOP); // t[0] = 0
    surgescript_program_add_line(program, SSOP_PUSH, SSOP(0), SSNOP, SSNOP); // push 0
    surgescript_program_add_line(program, SSOP_INC, SSOP(0), SSNOP, SSNOP); // t[0] = 1
    surgescript_program_add_line(program, SSOP_PUSH, SSOP(0), SSNOP, SSNOP); // push 1
    surgescript_program_add_line(program, SSOP_ASSIGN_NUMBER, SSOP(2), SSOPf(15), SSNOP); // t[2] = 15
    surgescript_program_add_label(program, loop2);
        surgescript_program_add_line(program, SSOP_POP, SSOP(1), SSNOP, SSNOP); // pop t[1] (fib(x-1))
        surgescript_program_add_line(program, SSOP_POP, SSOP(0), SSNOP, SSNOP); // pop t[0] (t[0] <= t[1]) (fib(x-2))
        surgescript_program_add_line(program, SSOP_ADD, SSOP(0), SSOP(1), SSNOP); // t[0] += t[1] (fib(x))
        surgescript_program_add_line(program, SSOP_OUT, SSOP(0), SSNOP, SSNOP);  // print fib(x)
        surgescript_program_add_line(program, SSOP_PUSH, SSOP(1), SSNOP, SSNOP); // push fib(x-1)
        surgescript_program_add_line(program, SSOP_PUSH, SSOP(0), SSNOP, SSNOP); // push fib(x)
        surgescript_program_add_line(program, SSOP_DEC, SSOP(2), SSNOP, SSNOP); // t[2] -= 1
        surgescript_program_add_line(program, SSOP_JMP_IF_NOTZERO, SSOP(loop2), SSOP(2), SSNOP); // jmp loop2 if t[2] <> 0

    // call other program
    surgescript_program_add_line(program, SSOP_ASSIGN_STRING, SSOP(0), SSOP(1), SSNOP); // t[0] = text[1]
    surgescript_program_add_line(program, SSOP_ASSIGN_OBJECTHANDLE, SSOP(1), SSOP(1), SSNOP); // t[1] = (object)1
    //surgescript_program_add_line(program, SSOP_CALL, SSOP(0), SSOP(1), SSOP(0));
}

void setup2(surgescript_program_t* program)
{
    surgescript_program_add_text(program, "Olá, mundo!");
    surgescript_program_add_line(program, SSOP_ASSIGN_STRING, SSOP(0), SSOP(0), SSNOP); // t[0] = text[0]
    surgescript_program_add_line(program, SSOP_OUT, SSOP(0), SSNOP, SSNOP);  // print t[0]
}

int main()
{
    surgescript_stack_t* stack = surgescript_stack_create();
    surgescript_programpool_t* program_pool = surgescript_programpool_create();
    surgescript_objectmanager_t* object_manager = surgescript_objectmanager_create(program_pool, stack);
    surgescript_program_t* program = surgescript_program_create(0, 0);
    surgescript_program_t* called_program = surgescript_program_create(0, 0);

    setup2(called_program);
    setup(program, called_program);

    surgescript_programpool_put(program_pool, "Application", "state:main", program);
    surgescript_programpool_put(program_pool, "Application", "__constructor", called_program);
    surgescript_objectmanager_spawn(object_manager, "Application", NULL, NULL, NULL);

    int it = 0;
    surgescript_objectmanager_handle_t root_handle = surgescript_objectmanager_root(object_manager);
    while(surgescript_objectmanager_exists(object_manager, root_handle)) { // while the root object exists
        printf("Iteracao #%d\n", ++it);
        surgescript_object_t* root = surgescript_objectmanager_get(object_manager, root_handle);
        surgescript_object_update_fulltree(root);
        if(it == 10)
            surgescript_objectmanager_delete(object_manager, root_handle);
    }

    surgescript_stack_destroy(stack);
    surgescript_programpool_destroy(program_pool);
    surgescript_objectmanager_destroy(object_manager);
    return 0;
}
