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
#include <vata/aut_base.hh>
#include <vata/bdd_tree_aut.hh>
#include <vata/bdd_tree_aut_op.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/util.hh>

using VATA::AutBase;
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
 *                                    Types                                   *
 ******************************************************************************/

typedef AutBase::StringToStateDict StringToStateDict;

typedef AutBase::StateToStateMap StateToStateMap;
typedef AutBase::StateType StateType;
typedef BDDTreeAut::StateTuple StateTuple;


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path UNREACHABLE_TIMBUK_FILE =
	AUT_DIR / "unreachable_removal_timbuk.txt";

const fs::path INCLUSION_TIMBUK_FILE =
	AUT_DIR / "inclusion_timbuk.txt";

const fs::path UNION_TIMBUK_FILE =
	AUT_DIR / "union_timbuk.txt";

const fs::path INTERSECTION_TIMBUK_FILE =
	AUT_DIR / "intersection_timbuk.txt";

const fs::path ADD_TRANS_TIMBUK_FILE =
	AUT_DIR / "add_trans_timbuk.txt";


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

	auto filenames = GetTimbukAutFilenames();
	for (const std::string& filename : filenames)
	{
		BOOST_MESSAGE("Loading automaton " + filename + "...");
		std::string autStr = VATA::Util::ReadFile(filename);

		BDDTreeAut aut;

		StringToStateDict stateDict;
		aut.LoadFromString(parser, autStr, &stateDict);

		std::string autOut = aut.DumpToString(serializer, &stateDict);

		AutDescription descOrig = parser.ParseString(autStr);
		AutDescription descOut = parser.ParseString(autOut);

		BOOST_CHECK_MESSAGE(descOrig == descOut,
			"\n\nExpecting:\n===========\n" +
			std::string(autStr) +
			"===========\n\nGot:\n===========\n" + autOut + "\n===========");
	}
}

BOOST_AUTO_TEST_CASE(adding_transitions)
{
	auto testfileContent = ParseTestFile(ADD_TRANS_TIMBUK_FILE.string());

	TimbukParser parser;
	TimbukSerializer serializer;

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 3, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string inputAutFile = (AUT_DIR / testcase[0]).string();
		std::string inputTransFile = (AUT_DIR / testcase[1]).string();
		std::string resultFile = (AUT_DIR / testcase[2]).string();

		BOOST_MESSAGE("Adding transitions from " + inputTransFile + " to " +
			inputAutFile + "...");

		std::string autStr = VATA::Util::ReadFile(inputAutFile);
		std::string transStr = VATA::Util::ReadFile(inputTransFile);
		std::string autCorrectStr = VATA::Util::ReadFile(resultFile);

		BDDTreeAut aut;
		StringToStateDict stateDict;
		aut.LoadFromString(parser, autStr, &stateDict);
		AutDescription autDesc = parser.ParseString(autStr);

		AutDescription transDesc = parser.ParseString(transStr);

		for (const AutDescription::Transition& trans : transDesc.transitions)
		{
			const std::string& parStr = trans.third;
			const std::string& symbolStr = trans.second;

			// get the parent state
			StateType parState;
			StringToStateDict::const_iterator itDict;
			if ((itDict = stateDict.FindFwd(parStr)) == stateDict.EndFwd())
			{
				parState = aut.AddState();
				stateDict.insert(std::make_pair(parStr, parState));
			}
			else
			{
				parState = itDict->second;
			}

			if (transDesc.finalStates.find(parStr) != transDesc.finalStates.end())
			{	// if the parent state is made final
				aut.SetStateFinal(parState);
			}

			const BDDTreeAut::SymbolType& symbol =
				aut.SafelyTranslateStringToSymbol(symbolStr);

			StateTuple children;
			for (auto childStr : trans.first)
			{	// for each child
				StateType childState;
				if ((itDict = stateDict.FindFwd(childStr)) == stateDict.EndFwd())
				{
					childState = aut.AddState();
					stateDict.insert(std::make_pair(childStr, childState));
				}
				else
				{
					childState = itDict->second;
				}

				if (transDesc.finalStates.find(childStr) != transDesc.finalStates.end())
				{	// if the parent state is made final
					aut.SetStateFinal(childState);
				}

				children.push_back(childState);
			}

			aut.AddSimplyTransition(children, symbol, parState);
		}

		std::string autTransStr = aut.DumpToString(serializer, &stateDict);

		AutDescription descOut = parser.ParseString(autTransStr);
		AutDescription descCorrect = parser.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descOut == descCorrect,
			"\n\nExpecting:\n===========\n" + autCorrectStr +
			"===========\n\nGot:\n===========\n" + autTransStr + "\n===========");
	}



//	// get state "q"
//	//BDDTreeAut::StateType stateQ = stateDict.TranslateFwd("q");
//	// get state "p"
//	BDDTreeAut::StateType stateP = stateDict.TranslateFwd("p");
//	// insert state "qa"
//	BDDTreeAut::StateType stateQA = aut.AddState();
//	stateDict.Insert(std::make_pair("qa", stateQA));
//
//	// add the following transition: a -> qa, to the description ...
//	AutDescription::Transition newTransition(std::vector<std::string>(), "a", "qa");
//	descCorrect.transitions.insert(newTransition);
//	// ... and to the automaton
//	aut.AddSimplyTransition(BDDTreeAut::StateTuple(),
//		BDDTreeAut::TranslateStringToSymbol("a"), stateQA);
//
//	// add the following transition: a(qa, qa) -> p, to the description ...
//	std::vector<std::string> childrenStr;
//	childrenStr.push_back("qa");
//	childrenStr.push_back("qa");
//	newTransition = AutDescription::Transition(childrenStr, "a", "p");
//	descCorrect.transitions.insert(newTransition);
//	// ... and to the automaton
//	BDDTreeAut::StateTuple children;
//	children.push_back(stateQA);
//	children.push_back(stateQA);
//	aut.AddSimplyTransition(children, BDDTreeAut::TranslateStringToSymbol("a"),
//		stateP);
//
//	std::string autOut = aut.DumpToString(serializer, &stateDict);
//	AutDescription descOut = parser.ParseString(autOut);
//
//	BOOST_CHECK_MESSAGE(descCorrect == descOut,
//		"\n\nExpecting:\n===========\n" +
//		serializer.Serialize(descCorrect) +
//		"===========\n\nGot:\n===========\n" + autOut + "\n===========");
}


BOOST_AUTO_TEST_CASE(aut_union_simple)
{
	auto testfileContent = ParseTestFile(UNION_TIMBUK_FILE.string());

	TimbukParser parser;
	TimbukSerializer serializer;

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 3, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string inputLhsFile = (AUT_DIR / testcase[0]).string();
		std::string inputRhsFile = (AUT_DIR / testcase[1]).string();
		std::string resultFile = (AUT_DIR / testcase[2]).string();

		BOOST_MESSAGE("Performing union of " + inputLhsFile + " and " +
			inputRhsFile + "...");

		std::string autLhsStr = VATA::Util::ReadFile(inputLhsFile);
		std::string autRhsStr = VATA::Util::ReadFile(inputRhsFile);
		std::string autCorrectStr = VATA::Util::ReadFile(resultFile);

		BDDTreeAut autLhs;
		StringToStateDict stateDictLhs;
		autLhs.LoadFromString(parser, autLhsStr, &stateDictLhs);
		AutDescription autLhsDesc = parser.ParseString(autLhsStr);

		BDDTreeAut autRhs(autLhs.GetTransTable());
		StringToStateDict stateDictRhs;
		autRhs.LoadFromString(parser, autRhsStr, &stateDictRhs);
		AutDescription autRhsDesc = parser.ParseString(autRhsStr);

		BDDTreeAut autUnion = VATA::Union(autLhs, autRhs);
		StringToStateDict stateDictUnion =
			VATA::Util::CreateUnionStringToStateMap(stateDictLhs, stateDictRhs);

		std::string autUnionStr = autUnion.DumpToString(serializer, &stateDictUnion);

		AutDescription descOut = parser.ParseString(autUnionStr);
		AutDescription descCorrect = parser.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descOut == descCorrect,
			"\n\nExpecting:\n===========\n" + autCorrectStr +
			"===========\n\nGot:\n===========\n" + autUnionStr + "\n===========");
	}
}


BOOST_AUTO_TEST_CASE(aut_union_trans_table_copy)
{
	auto testfileContent = ParseTestFile(UNION_TIMBUK_FILE.string());

	TimbukParser parser;
	TimbukSerializer serializer;

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 3, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string inputLhsFile = (AUT_DIR / testcase[0]).string();
		std::string inputRhsFile = (AUT_DIR / testcase[1]).string();
		std::string resultFile = (AUT_DIR / testcase[2]).string();

		BOOST_MESSAGE("Performing union of " + inputLhsFile + " and " +
			inputRhsFile + "...");

		std::string autLhsStr = VATA::Util::ReadFile(inputLhsFile);
		std::string autRhsStr = VATA::Util::ReadFile(inputRhsFile);
		std::string autCorrectStr = VATA::Util::ReadFile(resultFile);

		BDDTreeAut autLhs;
		StringToStateDict stateDictLhs;
		autLhs.LoadFromString(parser, autLhsStr, &stateDictLhs);
		AutDescription autLhsDesc = parser.ParseString(autLhsStr);

		BDDTreeAut autRhs;
		StringToStateDict stateDictRhs;
		autRhs.LoadFromString(parser, autRhsStr, &stateDictRhs);
		AutDescription autRhsDesc = parser.ParseString(autRhsStr);

		AutBase::StateToStateMap stateTranslMap;
		BDDTreeAut autUnion = VATA::Union(autLhs, autRhs, &stateTranslMap);
		StringToStateDict stateDictUnion =
			VATA::Util::CreateUnionStringToStateMap(stateDictLhs, stateDictRhs,
				&stateTranslMap);

		std::string autUnionStr = autUnion.DumpToString(serializer, &stateDictUnion);

		AutDescription descOut = parser.ParseString(autUnionStr);
		AutDescription descCorrect = parser.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descOut == descCorrect,
			"\n\nExpecting:\n===========\n" + autCorrectStr +
			"===========\n\nGot:\n===========\n" + autUnionStr + "\n===========");
	}
}


BOOST_AUTO_TEST_CASE(aut_intersection)
{
	auto testfileContent = ParseTestFile(INTERSECTION_TIMBUK_FILE.string());

	TimbukParser parser;
	TimbukSerializer serializer;

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 3, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string inputLhsFile = (AUT_DIR / testcase[0]).string();
		std::string inputRhsFile = (AUT_DIR / testcase[1]).string();
		std::string resultFile = (AUT_DIR / testcase[2]).string();

		BOOST_MESSAGE("Performing intersection of " + inputLhsFile + " and "
			+ inputRhsFile + "...");

		std::string autLhsStr = VATA::Util::ReadFile(inputLhsFile);
		std::string autRhsStr = VATA::Util::ReadFile(inputRhsFile);
		std::string autCorrectStr = VATA::Util::ReadFile(resultFile);

		BDDTreeAut autLhs;
		StringToStateDict stateDictLhs;
		autLhs.LoadFromString(parser, autLhsStr, &stateDictLhs);
		AutDescription autLhsDesc = parser.ParseString(autLhsStr);

		BDDTreeAut autRhs;
		StringToStateDict stateDictRhs;
		autRhs.LoadFromString(parser, autRhsStr, &stateDictRhs);
		AutDescription autRhsDesc = parser.ParseString(autRhsStr);

		AutBase::ProductTranslMap translMap;
		BDDTreeAut autIntersect = VATA::Intersection(autLhs, autRhs, &translMap);

		StringToStateDict stateDictIsect = VATA::Util::CreateProductStringToStateMap(
			stateDictLhs, stateDictRhs, translMap);

		std::string autIntersectStr = autIntersect.DumpToString(serializer,
			&stateDictIsect);

		AutDescription descOut = parser.ParseString(autIntersectStr);
		AutDescription descCorrect = parser.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descOut == descCorrect,
			"\n\nExpecting:\n===========\n" + autCorrectStr +
			"===========\n\nGot:\n===========\n" + autIntersectStr + "\n===========");
	}
}

BOOST_AUTO_TEST_CASE(aut_remove_unreachable)
{
	auto testfileContent = ParseTestFile(UNREACHABLE_TIMBUK_FILE.string());

	TimbukParser parser;
	TimbukSerializer serializer;

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 2, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string inputFile = (AUT_DIR / testcase[0]).string();
		std::string resultFile = (AUT_DIR / testcase[1]).string();

		BOOST_MESSAGE("Removing unreachable states from " + inputFile + "...");

		std::string autStr = VATA::Util::ReadFile(inputFile);
		std::string autCorrectStr = VATA::Util::ReadFile(resultFile);

		BDDTreeAut aut;
		StringToStateDict stateDict;
		aut.LoadFromString(parser, autStr, &stateDict);
		AutDescription autDesc = parser.ParseString(autStr);

		StateToStateMap translMap;
		BDDTreeAut autNoUnreach = VATA::RemoveUnreachableStates(aut, &translMap);

		stateDict = VATA::Util::RebindMap(stateDict, translMap);
		std::string autNoUnreachStr =
			autNoUnreach.DumpToString(serializer, &stateDict);

		AutDescription descOutNoUnreach = parser.ParseString(autNoUnreachStr);
		AutDescription descCorrectNoUnreach = parser.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descCorrectNoUnreach == descOutNoUnreach,
			"\n\nExpecting:\n===========\n" +
			serializer.Serialize(descCorrectNoUnreach) +
			"===========\n\nGot:\n===========\n" + autNoUnreachStr + "\n===========");
	}
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion)
{
	auto testfileContent = ParseTestFile(INCLUSION_TIMBUK_FILE.string());

	TimbukParser parser;
	TimbukSerializer serializer;

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 3, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string inputSmallerFile = (AUT_DIR / testcase[0]).string();
		std::string inputBiggerFile = (AUT_DIR / testcase[1]).string();
		unsigned expectedResult = static_cast<bool>(
			Convert::FromString<unsigned>(testcase[2]));

		BOOST_MESSAGE("Testing inclusion " + inputSmallerFile + " <= " +
			inputBiggerFile  + "...");

		std::string autSmallerStr = VATA::Util::ReadFile(inputSmallerFile);
		std::string autBiggerStr = VATA::Util::ReadFile(inputBiggerFile);

		BDDTreeAut autSmaller;
		autSmaller.LoadFromString(parser, autSmallerStr);

		BDDTreeAut autBigger;
		autBigger.LoadFromString(parser, autBiggerStr);

		bool doesInclusionHold = VATA::CheckInclusion(autSmaller, autBigger);

		BOOST_CHECK_MESSAGE(expectedResult == doesInclusionHold,
			"\n\nError checking inclusion " + inputSmallerFile + " <= " +
			inputBiggerFile + ": expected " + Convert::ToString(expectedResult) +
			", got " + Convert::ToString(doesInclusionHold));
	}
}

BOOST_AUTO_TEST_SUITE_END()

