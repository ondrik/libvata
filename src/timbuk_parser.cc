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
#include <vata/util/fake_file.hh>

using VATA::Parsing::AbstrParser;
using VATA::Parsing::TimbukParser;
using VATA::Util::AutDescription;

void yyrestart(FILE*);
extern int yydebug;
int yyparse(AutDescription&);

void yylex_destroy();

AutDescription TimbukParser::ParseString(const std::string& str)
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
