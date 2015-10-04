/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The Bison grammar for Timbuk format parser
 *
 *****************************************************************************/

%require "2.7.1"

%{
// #define YYDEBUG 1
%}

%code requires {
// VATA headers
#include <vata/parsing/timbuk_parser.hh>
#include <vata/util/aut_description.hh>
#include <vata/util/convert.hh>

// standard library headers
#include <algorithm>

GCC_DIAG_OFF(write-strings)

int yylex();
extern int yylineno;

using VATA::Util::Convert;
using VATA::Util::AutDescription;

inline void yyerror(AutDescription&, const char* msg)
{
  throw std::runtime_error("Parser error at line " +
    Convert::ToString(yylineno) + ": " + std::string(msg));
}

static AutDescription::StateTuple global_tuple;
}

%locations
%error-verbose

%parse-param {AutDescription& timbukParse}

%token OPERATIONS    "Ops"
%token AUTOMATON     "Automaton"
%token STATES        "States"
%token FINAL_STATES  "Final States"
%token TRANSITIONS   "Transitions"

%token NUMBER        "<number>"
%token IDENTIFIER    "<identifier>"

%token COLON         "\":\""
%token LPAR          "\"(\""
%token RPAR          "\")\""
%token ARROW         "\"->\""
%token COMMA         "\",\""

%token END_OF_FILE 0 "end-of-file"

%union
{
  char* svalue;
}

%type <svalue> IDENTIFIER
%type <svalue> NUMBER

%type<svalue> automaton_name
%type<svalue> state
%type<svalue> ident

%start start

%%

start: OPERATIONS operation_list
       AUTOMATON automaton_name
       STATES state_list
       FINAL_STATES final_state_list
       TRANSITIONS transition_list
	{
		timbukParse.name = $4;
		free($4);
	}
	;

operation_list: ident COLON NUMBER operation_list
	{
		timbukParse.symbols.insert(std::make_pair($1,
			Convert::FromString<unsigned>($3)));

		free($1);
		free($3);
	}
	|
	;

automaton_name: ident
  ;

state_list: state_list state
	{
		timbukParse.states.insert($2);
		free($2);
	}
	|
	;

final_state_list: final_state_list state
	{
		timbukParse.finalStates.insert($2);
		free($2);
	}
	|
	;

state: ident COLON NUMBER
	{
		$$ = $1;
	}
	| ident
	;

transition_list: transition_list transition
	|
	;

transition: ident LPAR transition_states RPAR ARROW state
	{
		std::reverse(global_tuple.begin(), global_tuple.end());
		timbukParse.transitions.insert(
			AutDescription::Transition(global_tuple, $1, $6));
		free($1);
		free($6);
	}
	| ident ARROW state
	{
		timbukParse.transitions.insert(
			AutDescription::Transition(AutDescription::StateTuple(), $1, $3));
		free($1);
		free($3);
	}
	;

transition_states: ident COMMA transition_states
	{
		global_tuple.push_back($1);
		free($1);
	}
	| ident
	{
		global_tuple.clear();
		global_tuple.push_back($1);
		free($1);
	}
	|
	{
		global_tuple.clear();
	}
	;

ident: IDENTIFIER
	| NUMBER
	;


%%
