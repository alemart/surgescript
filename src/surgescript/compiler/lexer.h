/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2023 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * compiler/lexer.h
 * SurgeScript compiler: lexical analyzer
 */

#ifndef _SURGESCRIPT_COMPILER_LEXER_H
#define _SURGESCRIPT_COMPILER_LEXER_H

#include <stdbool.h>

typedef struct surgescript_lexer_t surgescript_lexer_t;
struct surgescript_token_t;

surgescript_lexer_t* surgescript_lexer_create();
surgescript_lexer_t* surgescript_lexer_destroy(surgescript_lexer_t* lexer);

void surgescript_lexer_set(surgescript_lexer_t* lexer, const char* code); /* sets the code to be read */
struct surgescript_token_t* surgescript_lexer_scan(surgescript_lexer_t* lexer); /* scans the next token */
bool surgescript_lexer_unscan(surgescript_lexer_t* lexer, struct surgescript_token_t* token); /* puts a token back into the lexer */

#endif