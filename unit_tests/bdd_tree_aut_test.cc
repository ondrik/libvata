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

	BDDTreeAut aut1;

	aut1.LoadFromString(parser, AUT_A1);

	std::string aut1Out = aut1.DumpToString(serializer);

	AutDescription descOrig = parser.ParseString(AUT_A1);
	AutDescription descOut = parser.ParseString(aut1Out);

	BOOST_CHECK_MESSAGE(descOrig == descOut, "Error while checking \n" +
		std::string(AUT_A1) + "\n\nGot:\n" + aut1Out);
}

BOOST_AUTO_TEST_SUITE_END()

