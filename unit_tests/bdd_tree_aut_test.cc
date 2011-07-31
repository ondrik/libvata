/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Test suite for BDD based tree automaton
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

using VATA::BDDTreeAut;
using VATA::Util::Convert;
using VATA::Parsing::TimbukParser;
using VATA::Serialization::TimbukSerializer;
using VATA::Util::AutDescription;

// Boost headers
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BDDTreeAut
#include <boost/test/unit_test.hpp>

// testing headers
#include "log_fixture.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const char* AUT_A1 =
	"Ops \n"
	"Automaton A1\n"
	"States\n"
	"Final States\n"
	"Transitions\n"
	;

const char* AUT_A2 =
	"Ops a:0 b:1 c:2\n"
	"Automaton A2\n"
	"States p\n"
	"Final States p\n"
	"Transitions\n"
	"a -> p\n"
	;

const char* IMPORT_EXPORT_TEST_CASES[] = {AUT_A1, AUT_A2};
const size_t IMPORT_EXPORT_TEST_CASES_SIZE = (sizeof(IMPORT_EXPORT_TEST_CASES) /
	sizeof(const char*));


/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

/**
 * @brief  BDDTreeAut testing fixture
 *
 * Fixture for test of BDDTreeAut with characters as leaves.
 */
class BDDTreeAutFixture : public LogFixture
{ };


/******************************************************************************
 *                              Start of testing                              *
 ******************************************************************************/


BOOST_FIXTURE_TEST_SUITE(suite, BDDTreeAutFixture)

BOOST_AUTO_TEST_CASE(import_export)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	for (size_t i = 0; i < IMPORT_EXPORT_TEST_CASES_SIZE; ++i)
	{
		BDDTreeAut aut1;

		BDDTreeAut::StringToStateDict stateDict;
		aut1.LoadFromString(parser, IMPORT_EXPORT_TEST_CASES[i], &stateDict);

		std::string autOut = aut1.DumpToString(serializer, &stateDict);

		AutDescription descOrig = parser.ParseString(IMPORT_EXPORT_TEST_CASES[i]);
		AutDescription descOut = parser.ParseString(autOut);

		BOOST_CHECK_MESSAGE(descOrig == descOut,
			"\n\nExpecting:\n===========\n" +
			std::string(IMPORT_EXPORT_TEST_CASES[i]) +
			"===========\n\nGot:\n===========\n" + autOut + "\n===========");
	}
}

BOOST_AUTO_TEST_SUITE_END()

