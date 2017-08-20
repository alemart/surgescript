/*
 * SurgeScript
 * A lightweight programming language for computer games and interactive apps
 * Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * <script> := <objectlist> // start here
 *
 * <objectlist> := <object> <objectlist> | e
 * <object> := object string { <objectdecl> }
 * <objectdecl> := <tags> <vardecllist> <statedecllist> <fundecllist>
 *
 * <tags> := tag string ; <tags> | e
 *
 * <vardecllist> := <vardecl> <vardecllist> | e
 * <vardecl> := identifier = <conditionalexpr> ;
 *            | export identifier = <conditionalexpr> ;
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
 *             |  <postfixexpr>
 * <postfixexpr> := identifier ++ | identifier --
 *               |  <funcallexpr> <postfixexpr1>
 *               |  <primaryexpr> <postfixexpr1>
 *               |  <primaryexpr> <postfixexpr1> <dictexpr> <postfixexpr1>
 *               |  <primaryexpr> <postfixexpr1> <dictexpr> <postfixexpr1> assignop <assignexpr>
 *               |  <primaryexpr> <postfixexpr1> <dictexpr> ++ | <primaryexpr> <postfixexpr1> <dictexpr> --
 * <postfixexpr1> := . <funcallexpr> <lambdacall> <dictexpr1> <postfixexpr1>
 *                |  . identifier <lambdacall> <dictexpr1> <postfixexpr1>
 *                |  . identifier ++ | identifier --
 *                |  . identifier assignop <assignexpr>
 *                |  <lambdacall> <dictexpr1>
 * <dictexpr> := [ <expr> ] <dictexpr1>
 * <dictexpr1> := [ <expr> ] <dictexpr1> | e
 * <funcallexpr> := identifier ( )
 *               |  identifier ( <funargsexprlist> )
 * <lambdacall> := ( ) <lambdacall> | ( <funargsexprlist> ) <lambdacall> | e
 * <primaryexpr> := identifier | this | state | <constant> | <arrayexpr> | ( <expr> )
 * <constant> := number | string | true | false | null
 * <funargsexprlist> := <assignexpr> <funargsexprlist1>
 * <funargsexprlist1> := , <assignexpr> <funargsexprlist1> | e
 * <arrayexpr> := [ <arrayexpr1> ]
 * <arrayexpr1> := <assignexpr> | <assignexpr> , <arrayexpr1>
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
 * <blockstmt> := { <stmtlist> }
 * <exprstmt> := ;
 *            |  <expr> ;
 * <condstmt> := if ( <expr> ) <stmt>
 *            |  if ( <expr> ) <stmt> else <stmt>
 * <loopstmt> := while ( <expr> ) <stmt>
 *            |  for ( <expr> ; <expr> ; <expr> ) <stmt>
 * <retstmt> := return <expr> ;
 *           |  return ;
 * <jumpstmt> := break ;
 *            |  continue ;
 *
 */

#include <stdbool.h>

typedef struct surgescript_parser_t surgescript_parser_t;
struct surgescript_programpool_t;
struct surgescript_tagsystem_t;

/* create & destroy */
surgescript_parser_t* surgescript_parser_create(struct surgescript_programpool_t* program_pool, struct surgescript_tagsystem_t* tag_system);
surgescript_parser_t* surgescript_parser_destroy(surgescript_parser_t* parser);

/* parse a script */
bool surgescript_parser_parsefile(surgescript_parser_t* parser, const char* absolute_path);
bool surgescript_parser_parsemem(surgescript_parser_t* parser, const char* code_in_memory);

#endif