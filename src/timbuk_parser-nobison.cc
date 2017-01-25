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

using VATA::Parsing::AbstrParser;
using VATA::Parsing::TimbukParser;
using VATA::Util::AutDescription;

static AutDescription parseTimbuk(const std::string& str)
{
	AutDescription timbukParse;

	return timbukParse;
}

AutDescription TimbukParser::ParseString(const std::string& str)
{
	AutDescription timbukParse;

	try
	{
		timbukParse = parseTimbuk(str);
	}
	catch (std::exception& ex)
	{
		throw std::runtime_error("Error: \'" + std::string(ex.what()) +
			"\' while parsing \n" + str);
	}

	return timbukParse;
}
