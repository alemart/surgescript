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

/* setup some programs */
static void setup(surgescript_program_t* program)
{
    surgescript_program_label_t loop = surgescript_program_new_label(program);
    surgescript_program_label_t loop2 = surgescript_program_new_label(program);

    surgescript_program_add_text(program, "Fibonacci sequence:");
    surgescript_program_add_text(program, "call_c");

    // count 1 .. 10
    surgescript_program_add_line(program, SSOP_MOVF, SSOPu(1), SSOPf(10)); // t[1] = 10 (last)
    surgescript_program_add_line(program, SSOP_MOVF, SSOPu(0), SSOPf(1)); // t[0] = 1 (counter)
    surgescript_program_add_label(program, loop);
        surgescript_program_add_line(program, SSOP_OUT, SSOPu(0), SSOP()); // print counter
        surgescript_program_add_line(program, SSOP_INC, SSOPu(0), SSOP()); // counter++
        surgescript_program_add_line(program, SSOP_CMP, SSOPu(0), SSOPu(1)); // cmp counter, last
        surgescript_program_add_line(program, SSOP_JLE, SSOPu(loop), SSOP()); // jmp loop if counter <= last

    // hello
    surgescript_program_add_line(program, SSOP_MOVS, SSOPu(0), SSOP()); // t[0] = text[0]
    surgescript_program_add_line(program, SSOP_OUT, SSOPu(0), SSOP());  // print t[0]

    // print fib(1) .. fib(n), where fib(x) = fib(x-1) + fib(x-2) and n = 20
    surgescript_program_add_line(program, SSOP_MOVF, SSOPu(0), SSOPf(0)); // t[0] = 0
    surgescript_program_add_line(program, SSOP_PUSH, SSOPu(0), SSOP()); // push t[0]
    surgescript_program_add_line(program, SSOP_INC, SSOPu(0), SSOP()); // t[0] = 1
    surgescript_program_add_line(program, SSOP_PUSH, SSOPu(0), SSOP()); // push t[0]
    surgescript_program_add_line(program, SSOP_MOVF, SSOPu(0), SSOPf(20)); // t[0] = 20 (counter)
    surgescript_program_add_line(program, SSOP_PUSH, SSOPu(0), SSOP()); // push t[0]
    surgescript_program_add_label(program, loop2);
        surgescript_program_add_line(program, SSOP_POP, SSOPu(2), SSOP()); // pop t[2] (counter)
        surgescript_program_add_line(program, SSOP_POP, SSOPu(1), SSOP()); // pop t[1] (fib(x-1))
        surgescript_program_add_line(program, SSOP_POP, SSOPu(0), SSOP()); // pop t[0] (t[0] <= t[1]) (fib(x-2))
        surgescript_program_add_line(program, SSOP_ADD, SSOPu(0), SSOPu(1)); // t[0] += t[1] (fib(x))
        surgescript_program_add_line(program, SSOP_OUT, SSOPu(0), SSOP());  // print fib(x)
        surgescript_program_add_line(program, SSOP_DEC, SSOPu(2), SSOP()); // t[2] -= 1
        surgescript_program_add_line(program, SSOP_PUSH, SSOPu(1), SSOP()); // push fib(x-1)
        surgescript_program_add_line(program, SSOP_PUSH, SSOPu(0), SSOP()); // push fib(x)
        surgescript_program_add_line(program, SSOP_PUSH, SSOPu(2), SSOP()); // push counter
        surgescript_program_add_line(program, SSOP_MOVF, SSOPu(0), SSOPf(0)); // t[0] = 0
        surgescript_program_add_line(program, SSOP_CMP, SSOPu(2), SSOPu(0)); // cmp t[2], t[0]
        surgescript_program_add_line(program, SSOP_JG, SSOPu(loop2), SSOP()); // jmp loop2 if t[2] > 0
    surgescript_program_add_line(program, SSOP_POP, SSOPu(0), SSOP()); // pop counter
    surgescript_program_add_line(program, SSOP_POP, SSOPu(0), SSOP()); // pop fib(x-1)
    surgescript_program_add_line(program, SSOP_POP, SSOPu(0), SSOP()); // pop fib(x)

    // call other program
    surgescript_program_add_line(program, SSOP_MOVS, SSOPu(0), SSOPu(1)); // t[0] = text[1]
    surgescript_program_add_line(program, SSOP_MOVH, SSOPu(1), SSOPu(1)); // t[1] = (object)1
    surgescript_program_add_line(program, SSOP_MOVF, SSOPu(2), SSOPf(0)); // t[2] = 0
    surgescript_program_add_line(program, SSOP_CALL, SSOPu(0), SSOPu(1)); // call it!
    surgescript_program_add_line(program, SSOP_OUT, SSOPu(2), SSOP()); // print the return value
}

static void setup2(surgescript_program_t* program)
{
    surgescript_program_add_text(program, "Olá, mundo!");
    surgescript_program_add_line(program, SSOP_MOVS, SSOPu(0), SSOPu(0)); // t[0] = text[0]
    surgescript_program_add_line(program, SSOP_OUT, SSOPu(0), SSOP());  // print t[0]
}

static surgescript_var_t* my_cfun(surgescript_object_t* caller, const surgescript_var_t** param, int num_params)
{
    surgescript_var_t* var = surgescript_var_create();

    printf("heeeeeey from C! " __FILE__ ":%d\n", __LINE__);
    surgescript_object_kill(caller);

    return surgescript_var_set_string(var, "Today is " __DATE__);
}



int main()
{
    surgescript_stack_t* stack = surgescript_stack_create();
    surgescript_programpool_t* program_pool = surgescript_programpool_create();
    surgescript_objectmanager_t* object_manager = surgescript_objectmanager_create(program_pool, stack);
    surgescript_program_t* program = surgescript_program_create(0, 0);
    surgescript_program_t* program2 = surgescript_program_create(0, 0);
    surgescript_program_t* cprogram = surgescript_cprogram_create(0, my_cfun);

    setup(program);
    setup2(program2);

    surgescript_programpool_put(program_pool, "Application", "state:main", program);
    surgescript_programpool_put(program_pool, "Application", "__constructor", program2);
    surgescript_programpool_put(program_pool, "Application", "call_c", cprogram);
    surgescript_objectmanager_spawn(object_manager, "Application", NULL, NULL, NULL);

    surgescript_objectmanager_handle_t root_handle = surgescript_objectmanager_root(object_manager);
    while(surgescript_objectmanager_exists(object_manager, root_handle)) { /* while the root object exists */
        surgescript_object_t* root = surgescript_objectmanager_get(object_manager, root_handle);
        surgescript_object_update_fulltree(root);
    }

    surgescript_stack_destroy(stack);
    surgescript_programpool_destroy(program_pool);
    surgescript_objectmanager_destroy(object_manager);
    return 0;
}