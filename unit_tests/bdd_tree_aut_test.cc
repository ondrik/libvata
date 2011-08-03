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
#include <vata/bdd_tree_aut_op.hh>
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
	AutDescription descCorrect = parser.ParseString(AUT_TIMBUK_A4);

	// get state "q"
	//BDDTreeAut::StateType stateQ = stateDict.TranslateFwd("q");
	// get state "p"
	BDDTreeAut::StateType stateP = stateDict.TranslateFwd("p");
	// insert state "qa"
	BDDTreeAut::StateType stateQA = aut.AddState();
	stateDict.Insert(std::make_pair("qa", stateQA));

	// add the following transition: a -> qa, to the description ...
	AutDescription::Transition newTransition(std::vector<std::string>(), "a", "qa");
	descCorrect.transitions.insert(newTransition);
	// ... and to the automaton
	aut.AddSimplyTransition(BDDTreeAut::StateTuple(),
		BDDTreeAut::TranslateStringToSymbol("a"), stateQA);

	// add the following transition: a(qa, qa) -> p, to the description ...
	std::vector<std::string> childrenStr;
	childrenStr.push_back("qa");
	childrenStr.push_back("qa");
	newTransition = AutDescription::Transition(childrenStr, "a", "p");
	descCorrect.transitions.insert(newTransition);
	// ... and to the automaton
	BDDTreeAut::StateTuple children;
	children.push_back(stateQA);
	children.push_back(stateQA);
	aut.AddSimplyTransition(children, BDDTreeAut::TranslateStringToSymbol("a"),
		stateP);

	std::string autOut = aut.DumpToString(serializer, &stateDict);
	AutDescription descOut = parser.ParseString(autOut);

	BOOST_CHECK_MESSAGE(descCorrect == descOut,
		"\n\nExpecting:\n===========\n" +
		serializer.Serialize(descCorrect) +
		"===========\n\nGot:\n===========\n" + autOut + "\n===========");
}


BOOST_AUTO_TEST_CASE(aut_union)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	BDDTreeAut autU1;
	BDDTreeAut::StringToStateDict autU1StateDict;
	autU1.LoadFromString(parser, AUT_TIMBUK_UNION_1, &autU1StateDict);
	AutDescription autU1Desc = parser.ParseString(AUT_TIMBUK_UNION_1);

	BDDTreeAut autU2(autU1.GetTransTable());
	BDDTreeAut::StringToStateDict autU2StateDict;
	autU2.LoadFromString(parser, AUT_TIMBUK_UNION_2, &autU2StateDict);
	AutDescription autU2Desc = parser.ParseString(AUT_TIMBUK_UNION_2);

	BDDTreeAut autU3;
	BDDTreeAut::StringToStateDict autU3StateDict;
	autU3.LoadFromString(parser, AUT_TIMBUK_UNION_3, &autU3StateDict);
	AutDescription autU3Desc = parser.ParseString(AUT_TIMBUK_UNION_3);

	BDDTreeAut autUnion12 = VATA::Union(autU1, autU2);
}


BOOST_AUTO_TEST_SUITE_END()

