/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * runtime/lambda.h
 * SurgeScript lambdas (program + runtime environment)
 */

#ifndef _SURGESCRIPT_RUNTIME_LAMBDA_H
#define _SURGESCRIPT_RUNTIME_LAMBDA_H

/* lambdas: programs associated with a runtime env */
typedef struct surgescript_program_lambda_t surgescript_program_lambda_t;

/* forward declarations */
struct surgescript_program_t;
struct surgescript_program_runtimeenv_t;

/* api */
surgescript_program_lambda_t* surgescript_program_lambda_create(struct surgescript_program_t* program, struct surgescript_program_runtimeenv_t* runtime_environment);
surgescript_program_lambda_t* surgescript_program_lambda_clone(const surgescript_program_lambda_t* lambda);
surgescript_program_lambda_t* surgescript_program_lambda_destroy(surgescript_program_lambda_t* lambda);

const struct surgescript_program_t* surgescript_program_lambda_program(const surgescript_program_lambda_t* lambda);

void surgescript_program_lambda_run_update(const surgescript_program_lambda_t* lambda);
void surgescript_program_lambda_run_render(const surgescript_program_lambda_t* lambda);

#endif
