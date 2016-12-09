/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/lambda.c
 * SurgeScript lambdas (program + runtime environment)
 */

#include "lambda.h"
#include "program.h"
#include "runtime_environment.h"
#include "../util/util.h"

/* lambda structure */
struct surgescript_program_lambda_t
{
    surgescript_program_t* program; /* reference to an existing program */
    surgescript_program_runtimeenv_t* runtime_environment; /* a newly-created runtime environment associated with this lambda */
};



/*
 * surgescript_program_lambda_create()
 * Creates a new lambda: a program + a runtime env
 */
surgescript_program_lambda_t* surgescript_program_lambda_create(surgescript_program_t* program, surgescript_program_runtimeenv_t* runtime_environment)
{
    surgescript_program_lambda_t* lambda = ssmalloc(sizeof *lambda);
    lambda->program = program;
    lambda->runtime_environment = runtime_environment;
    return lambda;
}

/*
 * surgescript_program_lambda_clone()
 * Clones a lambda
 */
surgescript_program_lambda_t* surgescript_program_lambda_clone(const surgescript_program_lambda_t* lambda)
{
    surgescript_program_lambda_t* clone = ssmalloc(sizeof *clone);
    clone->program = lambda->program;
    clone->runtime_environment = surgescript_program_runtimeenv_clone(lambda->runtime_environment);
    return clone;
}

/*
 * surgescript_program_lambda_destroy()
 * Destroys an existing lambda
 */
surgescript_program_lambda_t* surgescript_program_lambda_destroy(surgescript_program_lambda_t* lambda)
{
    surgescript_program_runtimeenv_destroy(lambda->runtime_environment);
    ssfree(lambda);
    return NULL;
}

/*
 * surgescript_program_lambda_run_update()
 * Update logic
 */
void surgescript_program_lambda_run_update(const surgescript_program_lambda_t* lambda)
{
    surgescript_program_run_update(lambda->program, lambda->runtime_environment);
}

/*
 * surgescript_program_lambda_run_render()
 * Render logic
 */
void surgescript_program_lambda_run_render(const surgescript_program_lambda_t* lambda)
{
    surgescript_program_run_render(lambda->program, lambda->runtime_environment);
}

/*
 * surgescript_program_lambda_program()
 * Returns a pointer to the program
 */
const surgescript_program_t* surgescript_program_lambda_program(const surgescript_program_lambda_t* lambda)
{
    return lambda->program;
}
