/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The Flex grammar for Timbuk format parser
 *
 *****************************************************************************/

%option noyywrap
%option yylineno
/* %option debug */

%{

// VATA headers
#include <vata/vata.hh>
#include <vata/util/convert.hh>

// Bison header
#include "timbuk.y.hh"

GCC_DIAG_OFF(unused-function)
%}

DIGIT        [0-9]
IDENTCHAR    [a-zA-Z_\[\]\|]|{DIGIT}
WHITE        [ \t\n]

NUMBER       {DIGIT}+
IDENT        {IDENTCHAR}+

%%

"Ops"           {return OPERATIONS;}
"Automaton"     {return AUTOMATON;}
"States"        {return STATES;}
"Final States"  {return FINAL_STATES;}
"Transitions"   {return TRANSITIONS;}

{NUMBER}        {yylval.svalue = strdup(yytext); return NUMBER;}
{IDENT}         {yylval.svalue = strdup(yytext); return IDENTIFIER;}
{WHITE}

":"             {return COLON;}
","             {return COMMA;}
"("             {return LPAR;}
")"             {return RPAR;}
"->"            {return ARROW;}

<<EOF>>         {return END_OF_FILE;}

%%
