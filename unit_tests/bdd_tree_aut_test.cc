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
{
protected:// methods

//	static BDDTreeAut::StringToStateDict suffixize(
//		const BDDTreeAut::StringToStateDict& dict, const std::string& suffix)
//	{
//		BDDTreeAut::StringToStateDict result;
//
//		for (BDDTreeAut::StringToStateDict::const_iterator itDict = dict.begin();
//			itDict != dict.end(); ++itDict)
//		{	// transfer all states
//			result.insert(std::make_pair(itDict->first + suffix, itDict->second));
//		}
//
//		return result;
//	}
};

/******************************************************************************
 *                              Start of testing                              *
 ******************************************************************************/


BOOST_FIXTURE_TEST_SUITE(suite, BDDTreeAutFixture)

BOOST_AUTO_TEST_CASE(timbuk_import_export)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	for (auto autTest : TIMBUK_AUTOMATA)
	{
		BDDTreeAut aut;

		BDDTreeAut::StringToStateDict stateDict;
		aut.LoadFromString(parser, autTest, &stateDict);

		std::string autOut = aut.DumpToString(serializer, &stateDict);

		AutDescription descOrig = parser.ParseString(autTest);
		AutDescription descOut = parser.ParseString(autOut);

		BOOST_CHECK_MESSAGE(descOrig == descOut,
			"\n\nExpecting:\n===========\n" +
			std::string(autTest) +
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


BOOST_AUTO_TEST_CASE(aut_union_simple)
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

	BDDTreeAut autUnion12 = VATA::Union(autU1, autU2);
	BDDTreeAut::StringToStateDict autUnion12StateDict =
		autU1StateDict.Union(autU2StateDict);

	std::string autUnion12Str = autUnion12.DumpToString(serializer,
		&autUnion12StateDict);
	AutDescription descOutU12 = parser.ParseString(autUnion12Str);

	AutDescription descCorrectU12 = parser.ParseString(AUT_TIMBUK_UNION_12_RESULT);

	BOOST_CHECK_MESSAGE(descCorrectU12 == descOutU12,
		"\n\nExpecting:\n===========\n" +
		serializer.Serialize(descCorrectU12) +
		"===========\n\nGot:\n===========\n" + autUnion12Str + "\n===========");
}


BOOST_AUTO_TEST_CASE(aut_union_trans_table_copy)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	BDDTreeAut autU1;
	BDDTreeAut::StringToStateDict autU1StateDict;
	autU1.LoadFromString(parser, AUT_TIMBUK_UNION_1, &autU1StateDict);
	AutDescription autU1Desc = parser.ParseString(AUT_TIMBUK_UNION_1);

	BDDTreeAut autU3;
	BDDTreeAut::StringToStateDict autU3StateDict;
	autU3.LoadFromString(parser, AUT_TIMBUK_UNION_3, &autU3StateDict);
	AutDescription autU3Desc = parser.ParseString(AUT_TIMBUK_UNION_3);

	BDDTreeAut autUnion13 = VATA::Union(autU1, autU3);

	std::string autUnion13Str = autUnion13.DumpToString(serializer);
	AutDescription descOutU13 = parser.ParseString(autUnion13Str);

	AutDescription descCorrectU13 = parser.ParseString(AUT_TIMBUK_UNION_13_RESULT);

	BOOST_CHECK_MESSAGE(descCorrectU13 == descOutU13,
		"\n\nExpecting:\n===========\n" +
		serializer.Serialize(descCorrectU13) +
		"===========\n\nGot:\n===========\n" + autUnion13Str + "\n===========");
}

BOOST_AUTO_TEST_CASE(aut_intersection)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	BDDTreeAut autI1;
	autI1.LoadFromString(parser, AUT_TIMBUK_ISECT_1);
	AutDescription autI1Desc = parser.ParseString(AUT_TIMBUK_ISECT_1);

	BDDTreeAut autI2;
	autI2.LoadFromString(parser, AUT_TIMBUK_ISECT_2);
	AutDescription autI2Desc = parser.ParseString(AUT_TIMBUK_UNION_2);

	BDDTreeAut autIsect12 = VATA::Intersection(autI1, autI2);

	std::string autIsect12Str = autIsect12.DumpToString(serializer);
	AutDescription descOutI12 = parser.ParseString(autIsect12Str);

	AutDescription descCorrectI12 = parser.ParseString(AUT_TIMBUK_ISECT_12_RESULT);

	BOOST_CHECK_MESSAGE(descCorrectI12 == descOutI12,
		"\n\nExpecting:\n===========\n" +
		serializer.Serialize(descCorrectI12) +
		"===========\n\nGot:\n===========\n" + autIsect12Str + "\n===========");
}

BOOST_AUTO_TEST_CASE(aut_remove_unreachable)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	BDDTreeAut aut1;
	aut1.LoadFromString(parser, AUT_TIMBUK_UNREACHABLE_1);
	AutDescription autI1Desc = parser.ParseString(AUT_TIMBUK_UNREACHABLE_1);

	BDDTreeAut aut1UF = VATA::RemoveUnreachableStates(aut1);

	std::string aut1UFStr = aut1UF.DumpToString(serializer);
	AutDescription descOutUF1 = parser.ParseString(aut1UFStr);

	AutDescription descCorrectUF1 = parser.ParseString(AUT_TIMBUK_UNREACHABLE_1_RESULT);

	BOOST_CHECK_MESSAGE(descCorrectUF1 == descOutUF1,
		"\n\nExpecting:\n===========\n" +
		serializer.Serialize(descCorrectUF1) +
		"===========\n\nGot:\n===========\n" + aut1UFStr + "\n===========");
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	for (auto inclTest : TIMBUK_AUTOMATA_INCL)
	{
		BDDTreeAut autA;
		autA.LoadFromString(parser, inclTest.first.first);

		BDDTreeAut autB;
		autB.LoadFromString(parser, inclTest.first.second);

		bool doesInclusionHold = VATA::CheckInclusion(autA, autB);

		BOOST_CHECK_MESSAGE(inclTest.second == doesInclusionHold,
			"\n\nError checking inclusion of:\n===========\n" +
			autA.DumpToString(serializer) +
			"===========\n"
			"and\n===========\n" +
			autB.DumpToString(serializer) +
			"===========\n\nGot: " + Convert::ToString(doesInclusionHold) +
			", expected was " + Convert::ToString(inclTest.second));
	}
}

BOOST_AUTO_TEST_SUITE_END()

