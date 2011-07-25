/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The source code for the parser of Timbuk format.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/parsing/timbuk_parser.hh>

int yyparse(VATA::Parsing::TimbukParser::ReturnType*);


VATA::Parsing::TimbukParser::ReturnType
	VATA::Parsing::TimbukParser::ParseString(const std::string& str)
{
	ReturnType timbukParse;

	if (yyparse(&timbukParse))
	{
		assert(false);
	}
	else
	{
		assert(false);
	}

	return timbukParse;
}
