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

// VATA headers
#include <vata/parsing/timbuk_parser.hh>
#include <vata/util/aut_description.hh>
#include <vata/util/convert.hh>

// standard library headers
#include <algorithm>

GCC_DIAG_OFF(write-strings)

#define YYDEBUG 1

int yylex();
extern int yylineno;

using VATA::Util::Convert;
using VATA::Util::AutDescription;

void yyerror(AutDescription&, char* msg)
{
	throw std::runtime_error("Parser error at line " +
		Convert::ToString(yylineno) + ": " + std::string(msg));
}

AutDescription::StateTuple global_tuple;
%}

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

%union TokenType
{
  char* svalue;
}

%type <svalue> IDENTIFIER
%type <svalue> NUMBER

%type<svalue> automaton_name
%type<svalue> state
%type<svalue> ident

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

state_list: state state_list
	{
		timbukParse.states.insert($1);
		free($1);
	}
  |
  ;

final_state_list: state final_state_list
	{
		timbukParse.finalStates.insert($1);
		free($1);
	}
  |
  ;

state: ident COLON NUMBER
	{
		$$ = $1;
	}
  | ident
  ;

transition_list: transition transition_list
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
	;

ident: IDENTIFIER
	| NUMBER
	;


%%
