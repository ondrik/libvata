%{
#include <vata/parsing/timbuk_parser.hh>

GCC_DIAG_OFF(write-strings)

int yylex();
void yyerror(VATA::Parsing::TimbukParser::ReturnType*, char* msg);
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

start:





%%

GCC_DIAG_ON(write-strings)

void yyerror(VATA::Parsing::TimbukParser::ReturnType*, char* msg)
{
  assert(false);
}
