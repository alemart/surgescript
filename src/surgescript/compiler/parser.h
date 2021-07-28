/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2019 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * compiler/parser.h
 * SurgeScript compiler: syntax analyzer
 */

#ifndef _SURGESCRIPT_COMPILER_PARSER_H
#define _SURGESCRIPT_COMPILER_PARSER_H

/*
 * SurgeScript grammar:
 * (e is the empty symbol)
 *
 *
 *
 * <script> := <importlist> <objectlist> // start here
 *
 * <objectlist> := <object> <objectlist> | e
 * <object> := <annotations> object string <qualifiers> { <objectdecl> }
 * <objectdecl> := <vardecllist> <statedecllist> <fundecllist>
 * <qualifiers> := is <tags> | is <tags> emoticon | is emoticon | e
 * <tags> := string , <tags> | string
 * <annotations> := annotation <annotations> | e
 *
 * <vardecllist> := <vardecl> <vardecllist> | e
 * <vardecl> := identifier = <conditionalexpr> ;
 *            | public identifier = <conditionalexpr> ;
 *            | public readonly identifier = <conditionalexpr> ;
 *
 * <statedecllist> := <statedecl> <statedecllist> | e
 * <statedecl> := state string { <stmtlist> }
 *
 * <fundecllist> := <fundecl> <fundecllist> | e
 * <fundecl> := fun identifier ( ) { <stmtlist> }
 *           |  fun identifier ( <funargs> ) { <stmtlist> }
 * <funargs> := identifier <funargs1>
 * <funargs1> := , identifier <funargs1> | e
 *
 * <importlist> := using <plugin>; <importlist> | e
 * <plugin> := identifier | identifier . <plugin>
 *
 *
 *
 * <expr> := <assignexpr> , <expr> | <assignexpr>
 * <assignexpr> := <conditionalexpr>
 *              |  identifier assignop <assignexpr>
 *              |  state = <assignexpr>
 * <conditionalexpr> := <logicalorexpr> | <logicalorexpr> ? <expr> : <conditionalexpr>
 * <logicalorexpr> := <logicalandexpr> <logicalorexpr1>
 * <logicalorexpr1> := || <logicalandexpr> <logicalorexpr1> | e
 * <logicalandexpr> := <equalityexpr> <logicalandexpr1>
 * <logicalandexpr1> := && <equalityexpr> <logicalandexpr1> | e
 * <equalityexpr> := <relationalexpr> <equalityexpr1>
 * <equalityexpr1> := equalityop <relationalexpr> <equalityexpr1> | e
 * <relationalexpr> := <additiveexpr> <relationalexpr1>
 * <relationalexpr1> := relationalop <additiveexpr> <relationalexpr1> | e
 * <additiveexpr> := <multiplicativeexpr> <additiveexpr1>
 * <additiveexpr1> := additiveop <multiplicativeexpr> <additiveexpr1> | e
 * <multiplicativeexpr> := <unaryexpr> <multiplicativeexpr1>
 * <multiplicativeexpr1> := multiplicativeop <unaryexpr> <multiplicativeexpr1> | e
 * <unaryexpr> := + <unaryexpr> | - <unaryexpr>
 *             |  ! <unaryexpr>
 *             |  typeof <unaryexpr> | typeof ( <expr> )
 *             |  ++ identifier | -- identifier
 *             |  timeout ( <assignexpr> )
 *             |  <postfixexpr>
 * <postfixexpr> := identifier ++ | identifier --
 *               |  <funcallexpr> <postfixexpr1>
 *               |  <primaryexpr> <postfixexpr1>
 *               |  <primaryexpr> <postfixexpr1> <dictgetexpr> <postfixexpr1>
 *               |  <primaryexpr> <postfixexpr1> <dictgetexpr> <postfixexpr1> assignop <assignexpr>
 *               |  <primaryexpr> <postfixexpr1> <dictgetexpr> ++ | <primaryexpr> <postfixexpr1> <dictgetexpr> --
 * <postfixexpr1> := . <funcallexpr> <lambdacall> <dictgetexpr1> <postfixexpr1>
 *                |  . identifier <lambdacall> <dictgetexpr1> <postfixexpr1>
 *                |  . identifier ++ | identifier --
 *                |  . identifier assignop <assignexpr>
 *                |  <lambdacall> <dictgetexpr1>
 * <dictgetexpr> := [ <expr> ] <dictgetexpr1>
 * <dictgetexpr1> := [ <expr> ] <dictgetexpr1> | e
 * <funcallexpr> := identifier ( )
 *               |  identifier ( <funargsexprlist> )
 * <lambdacall> := ( ) <lambdacall> | ( <funargsexprlist> ) <lambdacall> | e
 * <primaryexpr> := identifier | this | state | caller | <constant> | <arrayexpr> | <dictexpr> | ( <expr> )
 * <constant> := number | string | true | false | null
 * <funargsexprlist> := <assignexpr> <funargsexprlist1>
 * <funargsexprlist1> := , <assignexpr> <funargsexprlist1> | e
 * <arrayexpr> := [ <arrayexpr1> ] | [ ]
 * <arrayexpr1> := <assignexpr> | <assignexpr> , <arrayexpr1>
 * <dictexpr> := { <dictexpr1> } | { }
 * <dictexpr1> := string : <assignexpr> | string : <assignexpr> , <dictexpr1>
 *             |  number : <assignexpr> | number : <assignexpr> , <dictexpr1>
 *
 *
 *
 * <stmtlist> := <stmt> <stmtlist> | e
 * <stmt> := <blockstmt>
 *        |  <exprstmt>
 *        |  <condstmt>
 *        |  <loopstmt>
 *        |  <jumpstmt>
 *        |  <retstmt>
 *        |  <miscstmt>
 * <blockstmt> := { <stmtlist> }
 * <exprstmt> := ;
 *            |  <expr> ;
 * <condstmt> := if ( <expr> ) <stmt>
 *            |  if ( <expr> ) <stmt> else <stmt>
 * <loopstmt> := while ( <expr> ) <stmt>
 *            |  for ( <expr> ; <expr> ; <expr> ) <stmt>
 *            |  foreach ( identifier in <expr> ) <stmt>
 * <retstmt> := return <expr> ;
 *           |  return ;
 * <jumpstmt> := break ;
 *            |  continue ;
 * <miscstmt> := assert ( <expr> ) ;
 *
 */

#include <stdbool.h>

typedef struct surgescript_parser_t surgescript_parser_t;
struct surgescript_programpool_t;
struct surgescript_tagsystem_t;

/* parser flags (bitwise OR) */
typedef enum surgescript_parser_flags_t {
    SSPARSER_DEFAULTS = 0, /* default configuration */
    SSPARSER_ALLOW_DUPLICATES = 1, /* allow duplicate objects */
    SSPARSER_SKIP_DUPLICATES = 2, /* skip duplicate objects */
} surgescript_parser_flags_t;

/* create & destroy */
surgescript_parser_t* surgescript_parser_create(struct surgescript_programpool_t* program_pool, struct surgescript_tagsystem_t* tag_system);
surgescript_parser_t* surgescript_parser_destroy(surgescript_parser_t* parser);

/* operations */
bool surgescript_parser_parsefile(surgescript_parser_t* parser, const char* absolute_path); /* parse a script file */
bool surgescript_parser_parsemem(surgescript_parser_t* parser, const char* code_in_memory); /* parse a script (in memory) */
void surgescript_parser_foreach_plugin(surgescript_parser_t* parser, void* data, void (*fun)(const char*,void*)); /* foreach plugin object found in any parsed script, run fun(object_name, data) */
void surgescript_parser_set_flags(surgescript_parser_t* parser, surgescript_parser_flags_t flags); /* set parser options (flags) */
surgescript_parser_flags_t surgescript_parser_get_flags(surgescript_parser_t* parser); /* get parser flags */

#endif
