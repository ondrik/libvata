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
#include <vata/util/fake_file.hh>

using VATA::Parsing::AbstrParser;
using VATA::Parsing::TimbukParser;

void yyrestart(FILE*);
extern int yydebug;
int yyparse(VATA::Parsing::TimbukParser::AutDescription&);

void yylex_destroy();

AbstrParser::AutDescription TimbukParser::ParseString(const std::string& str)
{
	AutDescription timbukParse;

//	yydebug = 1;

	VATA::Util::FakeFile fakeFile;
	yyrestart(fakeFile.OpenRead(str));

	try
	{
		yyparse(timbukParse);
	}
	catch (std::exception& ex)
	{
		yylex_destroy();
		throw std::runtime_error("Error: \'" + std::string(ex.what()) +
			"\' while parsing \n" + str);
	}

	yylex_destroy();

	return timbukParse;
}


bool TimbukParser::AutDescription::operator==(const AutDescription& rhs) const
{
	return name == rhs.name &&
		symbols == rhs.symbols &&
		states == rhs.states &&
		finalStates == rhs.finalStates &&
		transitions == rhs.transitions;
}
