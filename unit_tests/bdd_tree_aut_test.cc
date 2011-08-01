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
using VATA::MTBDDPkg::VarAsgn;
using VATA::Parsing::TimbukParser;
using VATA::Serialization::TimbukSerializer;
using VATA::Util::AutDescription;
using VATA::Util::Convert;

// Boost headers
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BDDTreeAut
#include <boost/test/unit_test.hpp>

// testing headers
#include "log_fixture.hh"
#include "aut_db.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/



/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

/**
 * @brief  BDDTreeAut testing fixture
 *
 * Fixture for test of BDDTreeAut
 */
class BDDTreeAutFixture : public LogFixture
{ };


/******************************************************************************
 *                              Start of testing                              *
 ******************************************************************************/


BOOST_FIXTURE_TEST_SUITE(suite, BDDTreeAutFixture)

BOOST_AUTO_TEST_CASE(timbuk_import_export)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	for (size_t i = 0; i < TIMBUK_AUTOMATA_SIZE; ++i)
	{
		BDDTreeAut aut;

		BDDTreeAut::StringToStateDict stateDict;
		aut.LoadFromString(parser, TIMBUK_AUTOMATA[i], &stateDict);

		std::string autOut = aut.DumpToString(serializer, &stateDict);

		AutDescription descOrig = parser.ParseString(TIMBUK_AUTOMATA[i]);
		AutDescription descOut = parser.ParseString(autOut);

		BOOST_CHECK_MESSAGE(descOrig == descOut,
			"\n\nExpecting:\n===========\n" +
			std::string(TIMBUK_AUTOMATA[i]) +
			"===========\n\nGot:\n===========\n" + autOut + "\n===========");
	}
}


BOOST_AUTO_TEST_CASE(adding_transitions)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	BDDTreeAut aut;

	BDDTreeAut::StringToStateDict stateDict;
	aut.LoadFromString(parser, AUT_TIMBUK_A4, &stateDict);

	// add the following transition: a -> q
	AutDescription::Transition newTransition(std::vector<std::string>(), "a", "q");

	assert(aut.GetFinalStates().size() == 1);
	BDDTreeAut::StateType parent = aut.GetFinalStates()[0];

	aut.AddTransition(BDDTreeAut::StateTuple(),
		BDDTreeAut::TranslateStringToSymbol("a"), parent);

	std::string autOut = aut.DumpToString(serializer, &stateDict);
	AutDescription descOut = parser.ParseString(autOut);

	AutDescription descCorrect = parser.ParseString(AUT_TIMBUK_A4);
	descCorrect.transitions.insert(newTransition);

	BOOST_CHECK_MESSAGE(descCorrect == descOut,
		"\n\nExpecting:\n===========\n" +
		serializer.Serialize(descCorrect) +
		"===========\n\nGot:\n===========\n" + autOut + "\n===========");
}


BOOST_AUTO_TEST_SUITE_END()

