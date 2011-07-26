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

using VATA::Parsing::TimbukParser;

void yyrestart(FILE*);
extern int yydebug;
int yyparse(VATA::Parsing::TimbukParser::ReturnType&);

void yylex_destroy();

TimbukParser::ReturnType TimbukParser::ParseString(const std::string& str)
{
	ReturnType timbukParse;

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


bool TimbukParser::ReturnType::operator==(const ReturnType& rhs) const
{
	return name == rhs.name &&
		symbols == rhs.symbols &&
		states == rhs.states &&
		finalStates == rhs.finalStates &&
		transitions == rhs.transitions;
}
