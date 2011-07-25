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

using VATA::Parsing::TimbukParser;

// Boost headers
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TimbukParser
#include <boost/test/unit_test.hpp>

// testing headers
#include "log_fixture.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const char* AUT_A1 =
	"Ops a:0 b:1 c:2\n"
	"Automaton A1\n"
	"States\n"
	"Final States\n"
	"Transitions\n";

const char* AUT_A2 =
	"Ops \n"
	"Automaton A2\n"
	"States\n"
	"Final States\n"
	"Transitions\n";



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

BOOST_AUTO_TEST_CASE(import_export)
{
	TimbukParser parser;

	parser.ParseString(AUT_A1);

	BOOST_CHECK_MESSAGE(1 ==1, "1 != 1");
}

BOOST_AUTO_TEST_SUITE_END()

