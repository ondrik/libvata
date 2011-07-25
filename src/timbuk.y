/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The Bison grammar for Timbuk format parser
 *
 *****************************************************************************/

%{
#include <vata/parsing/timbuk_parser.hh>
#include <vata/util/convert.hh>

GCC_DIAG_OFF(write-strings)

#define YYDEBUG 1

int yylex();
extern int yylineno;

using VATA::Util::Convert;

void yyerror(VATA::Parsing::TimbukParser::ReturnType*, char* msg)
{
	throw std::runtime_error("Parser error at line " +
		Convert::ToString(yylineno) + ": " + std::string(msg));
}
%}

%locations
%error-verbose

%parse-param {VATA::Parsing::TimbukParser::ReturnType* timbukParse}

%token OPERATIONS
%token AUTOMATON
%token STATES
%token FINAL_STATES
%token TRANSITIONS

%token NUMBER
%token IDENTIFIER

%token COLON
%token LPAR
%token RPAR
%token ARROW

%token END_OF_FILE

%%

start: OPERATIONS start
	| AUTOMATON start
	| STATES start
	| FINAL_STATES start
	| TRANSITIONS start
	| NUMBER start
	| IDENTIFIER start
	| COLON start
	| LPAR start
	| RPAR start
	| ARROW start
	| END_OF_FILE






%%
