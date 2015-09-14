/*****************************************************************************
 *  VATA Tree Automata Library
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
#include <vata/util/aut_description.hh>

// maybe improve the path to something more relative?
#include "../build/src/timbuk.l.hh"

using VATA::Parsing::AbstrParser;
using VATA::Parsing::TimbukParser;
using VATA::Util::AutDescription;

extern int yydebug;
int yyparse(AutDescription&);

// void yylex_destroy();

AutDescription TimbukParser::ParseString(const std::string& str)
{
	AutDescription timbukParse;

	// yydebug = 1;

	YY_BUFFER_STATE bfSt = nullptr;
	try
	{
		bfSt = yy_scan_string(str.c_str());
		yyparse(timbukParse);
		yy_delete_buffer(bfSt);
	}
	catch (std::exception& ex)
	{
		yy_delete_buffer(bfSt);
		// yylex_destroy();
		throw std::runtime_error("Error: \'" + std::string(ex.what()) +
			"\' while parsing \n" + str);
	}

	// yylex_destroy();

	return timbukParse;
}
