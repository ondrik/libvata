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
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/convert.hh>

using VATA::Parsing::TimbukParser;
using VATA::Serialization::TimbukSerializer;
using VATA::Util::Convert;

// Boost headers
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TimbukParser
#include <boost/test/unit_test.hpp>

// testing headers
#include "log_fixture.hh"
#include "aut_db.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const char* AUT_FAIL1 = "";
const char* AUT_FAIL2 = " ";
const char* AUT_FAIL3 = "Automaton";
const char* AUT_FAIL4 = "j@#(&$*O!@";
const char* AUT_FAIL5 = "Ops";

const char* FAILING_TEST_CASES[] =
{
	AUT_FAIL1,
	AUT_FAIL2,
	AUT_FAIL3,
	AUT_FAIL4,
	AUT_FAIL5
};


/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

/**
 * @brief  TimbukParser testing fixture
 *
 * Fixture for test of TimbukParser
 */
class TimbukParserFixture : public LogFixture
{ };


/******************************************************************************
 *                              Start of testing                              *
 ******************************************************************************/


BOOST_FIXTURE_TEST_SUITE(suite, TimbukParserFixture)

BOOST_AUTO_TEST_CASE(correct_format)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	for (auto autTest : TIMBUK_AUTOMATA)
	{
		TimbukParser::AutDescription desc = parser.ParseString(autTest);
		std::string dumpedStr = serializer.Serialize(desc);
		TimbukParser::AutDescription secondTimeParsed = parser.ParseString(dumpedStr);

		BOOST_CHECK_MESSAGE(desc == secondTimeParsed, "Error while checking \n" +
			std::string(autTest));
	}
}

BOOST_AUTO_TEST_CASE(incorrect_format)
{
	TimbukParser parser;

	for (auto autTest : FAILING_TEST_CASES)
	{
		BOOST_CHECK_THROW(parser.ParseString(autTest), std::exception);
	}
}

BOOST_AUTO_TEST_SUITE_END()

