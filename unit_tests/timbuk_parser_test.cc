/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Test suite for the Timbuk parser
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/util/convert.hh>

using VATA::Parsing::TimbukParser;
using VATA::Util::Convert;

// Boost headers
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TimbukParser
#include <boost/test/unit_test.hpp>

// testing headers
#include "log_fixture.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const char* AUT_EMPTY =
	"Ops\n"
	"Automaton A1\n"
	"States\n"
	"Final States\n"
	"Transitions\n";

const char* AUT_SIMPLE =
	"Ops a:0 b:1 c:2\n"
	"Automaton A1\n"
	"States p q:0 r\n"
	"Final States r\n"
	"Transitions\n"
	"a -> q\n"
	"b(q) -> p\n"
	"c(p, p) -> r\n";

const char* CORRECT_TEST_CASES[] = {AUT_EMPTY, AUT_SIMPLE};
const size_t CORRECT_TEST_CASES_SIZE = (sizeof(CORRECT_TEST_CASES) /
	sizeof(const char*));

const char* AUT_FAIL1 = "";
const char* AUT_FAIL2 = " ";

const char* FAILING_TEST_CASES[] = {AUT_FAIL1, AUT_FAIL2};
const size_t FAILING_TEST_CASES_SIZE = (sizeof(FAILING_TEST_CASES) /
	sizeof(const char*));


/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

/**
 * @brief  TimbukParser testing fixture
 *
 * Fixture for test of TimbukParser
 */
class TimbukParserFixture : public LogFixture
{

protected:  // methods


};


/******************************************************************************
 *                              Start of testing                              *
 ******************************************************************************/


BOOST_FIXTURE_TEST_SUITE(suite, TimbukParserFixture)

BOOST_AUTO_TEST_CASE(correct_format)
{
	TimbukParser parser;

	for (size_t i = 0; i < CORRECT_TEST_CASES_SIZE; ++i)
	{
		TimbukParser::ReturnType aut = parser.ParseString(CORRECT_TEST_CASES[i]);
		std::string dumpedStr = Convert::ToString(aut);
		TimbukParser::ReturnType secondTimeParsed = parser.ParseString(dumpedStr);

		BOOST_CHECK_MESSAGE(aut == secondTimeParsed, "Error while checking " +
			std::string(CORRECT_TEST_CASES[i]));
	}
}

BOOST_AUTO_TEST_CASE(incorrect_format)
{
	TimbukParser parser;

	for (size_t i = 0; i < FAILING_TEST_CASES_SIZE; ++i)
	{
		BOOST_CHECK_THROW(parser.ParseString(FAILING_TEST_CASES[i]), std::exception);
	}
}

BOOST_AUTO_TEST_SUITE_END()

