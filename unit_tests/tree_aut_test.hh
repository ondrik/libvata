/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Common tests for testing various tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>
#include <vata/util/util.hh>

using VATA::AutBase;
using VATA::Parsing::TimbukParser;
using VATA::Serialization::TimbukSerializer;
using VATA::Util::AutDescription;
using VATA::Util::Convert;

// Boost headers
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE AutType
#include <boost/test/unit_test.hpp>

// testing headers
#include "log_fixture.hh"





/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path LOAD_TIMBUK_FILE =
	AUT_DIR / "load_timbuk.txt";

const fs::path UNREACHABLE_TIMBUK_FILE =
	AUT_DIR / "unreachable_removal_timbuk.txt";

const fs::path USELESS_TIMBUK_FILE =
	AUT_DIR / "useless_removal_timbuk.txt";

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
 * @brief  TreeAut testing fixture
 *
 * Fixture for test of TreeAut
 */
class TreeAutFixture : public LogFixture, public AutTypeFixture
{
protected:// data types

	typedef AutTypeFixture::AutType AutType;

	typedef AutBase::StringToStateDict StringToStateDict;

	typedef AutBase::StateToStateMap StateToStateMap;
	typedef AutBase::StateType StateType;
	typedef AutType::StateTuple StateTuple;

	typedef VATA::Util::TranslatorStrict<AutBase::StringToStateDict::MapBwdType>
		StateBackTranslatorStrict;

	typedef VATA::Util::TranslatorStrict<AutType::StringToSymbolDict::MapBwdType>
		SymbolBackTranslatorStrict;

protected:// data members

	TimbukParser parser_;
	TimbukSerializer serializer_;

	AutType::StringToSymbolDict symbolDict_;

protected:// methods

	TreeAutFixture() :
		parser_(),
		serializer_(),
		symbolDict_()
	{
		AutType::SetSymbolDictPtr(&symbolDict_);
		AutType::SetNextSymbolPtr(&nextSymbol_);
	}

	void readAut(AutType& aut, StringToStateDict& stateDict, const std::string& str)
	{
		aut.LoadFromString(parser_, str, stateDict);
	}

	template <class Automaton>
	std::string dumpAut(const Automaton& aut, const StringToStateDict& stateDict)
	{
		return aut.DumpToString(serializer_,
			StateBackTranslatorStrict(stateDict.GetReverseMap()),
			SymbolBackTranslatorStrict(Automaton::GetSymbolDict().GetReverseMap()));
	}
};

/******************************************************************************
 *                              Start of testing                              *
 ******************************************************************************/


BOOST_FIXTURE_TEST_SUITE(suite, TreeAutFixture)

BOOST_AUTO_TEST_CASE(timbuk_import_export)
{
	auto testfileContent = ParseTestFile(LOAD_TIMBUK_FILE.string());

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 1, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string filename = (AUT_DIR / testcase[0]).string();
		BOOST_MESSAGE("Loading automaton " + filename + "...");
		std::string autStr = VATA::Util::ReadFile(filename);

		StringToStateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);

		std::string autOut = dumpAut(aut, stateDict);

		AutDescription descOrig = parser_.ParseString(autStr);
		AutDescription descOut = parser_.ParseString(autOut);

		BOOST_CHECK_MESSAGE(descOrig == descOut,
			"\n\nExpecting:\n===========\n" +
			std::string(autStr) +
			"===========\n\nGot:\n===========\n" + autOut + "\n===========");
	}
}

//BOOST_AUTO_TEST_CASE(adding_transitions)
//{
//	auto testfileContent = ParseTestFile(ADD_TRANS_TIMBUK_FILE.string());
//
//	for (auto testcase : testfileContent)
//	{
//		BOOST_REQUIRE_MESSAGE(testcase.size() == 3, "Invalid format of a testcase: " +
//			Convert::ToString(testcase));
//
//		std::string inputAutFile = (AUT_DIR / testcase[0]).string();
//		std::string inputTransFile = (AUT_DIR / testcase[1]).string();
//		std::string resultFile = (AUT_DIR / testcase[2]).string();
//
//		BOOST_MESSAGE("Adding transitions from " + inputTransFile + " to " +
//			inputAutFile + "...");
//
//		std::string autStr = VATA::Util::ReadFile(inputAutFile);
//		std::string transStr = VATA::Util::ReadFile(inputTransFile);
//		std::string autCorrectStr = VATA::Util::ReadFile(resultFile);
//
//		AutType aut;
//		StringToStateDict stateDict;
//		aut.LoadFromString(parser_, autStr, &stateDict);
//		AutDescription autDesc = parser_.ParseString(autStr);
//
//		AutDescription transDesc = parser_.ParseString(transStr);
//
//		for (const AutDescription::Transition& trans : transDesc.transitions)
//		{
//			const std::string& parStr = trans.third;
//			const std::string& symbolStr = trans.second;
//
//			// get the parent state
//			StateType parState;
//			StringToStateDict::const_iterator itDict;
//			if ((itDict = stateDict.FindFwd(parStr)) == stateDict.EndFwd())
//			{
//				parState = aut.AddState();
//				stateDict.insert(std::make_pair(parStr, parState));
//			}
//			else
//			{
//				parState = itDict->second;
//			}
//
//			if (transDesc.finalStates.find(parStr) != transDesc.finalStates.end())
//			{	// if the parent state is made final
//				aut.SetStateFinal(parState);
//			}
//
//			const AutType::SymbolType& symbol =
//				aut.SafelyTranslateStringToSymbol(symbolStr);
//
//			StateTuple children;
//			for (auto childStr : trans.first)
//			{	// for each child
//				StateType childState;
//				if ((itDict = stateDict.FindFwd(childStr)) == stateDict.EndFwd())
//				{
//					childState = aut.AddState();
//					stateDict.insert(std::make_pair(childStr, childState));
//				}
//				else
//				{
//					childState = itDict->second;
//				}
//
//				if (transDesc.finalStates.find(childStr) != transDesc.finalStates.end())
//				{	// if the parent state is made final
//					aut.SetStateFinal(childState);
//				}
//
//				children.push_back(childState);
//			}
//
//			aut.AddSimplyTransition(children, symbol, parState);
//		}
//
//		std::string autTransStr = aut.DumpToString(serializer_, &stateDict);
//
//		AutDescription descOut = parser_.ParseString(autTransStr);
//		AutDescription descCorrect = parser_.ParseString(autCorrectStr);
//
//		BOOST_CHECK_MESSAGE(descOut == descCorrect,
//			"\n\nExpecting:\n===========\n" + autCorrectStr +
//			"===========\n\nGot:\n===========\n" + autTransStr + "\n===========");
//	}
//
//
//
////	// get state "q"
////	//BDDTopDownTreeAut::StateType stateQ = stateDict.TranslateFwd("q");
////	// get state "p"
////	BDDTopDownTreeAut::StateType stateP = stateDict.TranslateFwd("p");
////	// insert state "qa"
////	BDDTopDownTreeAut::StateType stateQA = aut.AddState();
////	stateDict.Insert(std::make_pair("qa", stateQA));
////
////	// add the following transition: a -> qa, to the description ...
////	AutDescription::Transition newTransition(std::vector<std::string>(), "a", "qa");
////	descCorrect.transitions.insert(newTransition);
////	// ... and to the automaton
////	aut.AddSimplyTransition(BDDTopDownTreeAut::StateTuple(),
////		BDDTopDownTreeAut::TranslateStringToSymbol("a"), stateQA);
////
////	// add the following transition: a(qa, qa) -> p, to the description ...
////	std::vector<std::string> childrenStr;
////	childrenStr.push_back("qa");
////	childrenStr.push_back("qa");
////	newTransition = AutDescription::Transition(childrenStr, "a", "p");
////	descCorrect.transitions.insert(newTransition);
////	// ... and to the automaton
////	BDDTopDownTreeAut::StateTuple children;
////	children.push_back(stateQA);
////	children.push_back(stateQA);
////	aut.AddSimplyTransition(children, BDDTopDownTreeAut::TranslateStringToSymbol("a"),
////		stateP);
////
////	std::string autOut = aut.DumpToString(serializer_, &stateDict);
////	AutDescription descOut = parser_.ParseString(autOut);
////
////	BOOST_CHECK_MESSAGE(descCorrect == descOut,
////		"\n\nExpecting:\n===========\n" +
////		serializer_.Serialize(descCorrect) +
////		"===========\n\nGot:\n===========\n" + autOut + "\n===========");
//}


#if 0
BOOST_AUTO_TEST_CASE(aut_union_simple)
{
	auto testfileContent = ParseTestFile(UNION_TIMBUK_FILE.string());

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

		StringToStateDict stateDictLhs;
		AutType autLhs;
		readAut(autLhs, stateDictLhs, autLhsStr);
		AutDescription autLhsDesc = parser_.ParseString(autLhsStr);

		StringToStateDict stateDictRhs;
		AutType autRhs(autLhs.GetTransTable());
		readAut(autRhs, stateDictRhs, autRhsStr);
		AutDescription autRhsDesc = parser_.ParseString(autRhsStr);

		AutType autUnion = VATA::Union(autLhs, autRhs);
		StringToStateDict stateDictUnion =
			VATA::Util::CreateUnionStringToStateMap(stateDictLhs, stateDictRhs);

		std::string autUnionStr = dumpAut(autUnion, stateDictUnion);

		AutDescription descOut = parser_.ParseString(autUnionStr);
		AutDescription descCorrect = parser_.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descOut == descCorrect,
			"\n\nExpecting:\n===========\n" + autCorrectStr +
			"===========\n\nGot:\n===========\n" + autUnionStr + "\n===========");
	}
}
#endif


BOOST_AUTO_TEST_CASE(aut_union_trans_table_copy)
{
	auto testfileContent = ParseTestFile(UNION_TIMBUK_FILE.string());

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

		StringToStateDict stateDictLhs;
		AutType autLhs;
		readAut(autLhs, stateDictLhs, autLhsStr);
		AutDescription autLhsDesc = parser_.ParseString(autLhsStr);

		StringToStateDict stateDictRhs;
		AutType autRhs;
		readAut(autRhs, stateDictRhs, autRhsStr);
		AutDescription autRhsDesc = parser_.ParseString(autRhsStr);

		AutBase::StateToStateMap stateTranslMapLhs;
		AutBase::StateToStateMap stateTranslMapRhs;
		AutType autUnion = VATA::Union(autLhs, autRhs, &stateTranslMapLhs,
			&stateTranslMapRhs);
		StringToStateDict stateDictUnion =
			VATA::Util::CreateUnionStringToStateMap(stateDictLhs, stateDictRhs,
				&stateTranslMapLhs, &stateTranslMapRhs);

		std::string autUnionStr = dumpAut(autUnion, stateDictUnion);

		AutDescription descOut = parser_.ParseString(autUnionStr);
		AutDescription descCorrect = parser_.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descOut == descCorrect,
			"\n\nExpecting:\n===========\n" + autCorrectStr +
			"===========\n\nGot:\n===========\n" + autUnionStr + "\n===========");
	}
}


BOOST_AUTO_TEST_CASE(aut_intersection)
{
	auto testfileContent = ParseTestFile(INTERSECTION_TIMBUK_FILE.string());

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

		StringToStateDict stateDictLhs;
		AutType autLhs;
		readAut(autLhs, stateDictLhs, autLhsStr);
		AutDescription autLhsDesc = parser_.ParseString(autLhsStr);

		StringToStateDict stateDictRhs;
		AutType autRhs;
		readAut(autRhs, stateDictRhs, autRhsStr);
		AutDescription autRhsDesc = parser_.ParseString(autRhsStr);

		AutBase::ProductTranslMap translMap;
		AutType autIntersect = VATA::Intersection(autLhs, autRhs, &translMap);

		StringToStateDict stateDictIsect = VATA::Util::CreateProductStringToStateMap(
			stateDictLhs, stateDictRhs, translMap);

		std::string autIntersectStr = dumpAut(autIntersect, stateDictIsect);

		AutDescription descOut = parser_.ParseString(autIntersectStr);
		AutDescription descCorrect = parser_.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descOut == descCorrect,
			"\n\nExpecting:\n===========\n" + autCorrectStr +
			"===========\n\nGot:\n===========\n" + autIntersectStr + "\n===========");
	}
}

BOOST_AUTO_TEST_CASE(aut_remove_unreachable)
{
	auto testfileContent = ParseTestFile(UNREACHABLE_TIMBUK_FILE.string());

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 2, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string inputFile = (AUT_DIR / testcase[0]).string();
		std::string resultFile = (AUT_DIR / testcase[1]).string();

		BOOST_MESSAGE("Removing unreachable states from " + inputFile + "...");

		std::string autStr = VATA::Util::ReadFile(inputFile);
		std::string autCorrectStr = VATA::Util::ReadFile(resultFile);

		StringToStateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);
		AutDescription autDesc = parser_.ParseString(autStr);

		StateToStateMap translMap;
		AutType autNoUnreach = VATA::RemoveUnreachableStates(aut, &translMap);

		stateDict = VATA::Util::RebindMap(stateDict, translMap);
		std::string autNoUnreachStr = dumpAut(autNoUnreach, stateDict);

		AutDescription descOutNoUnreach = parser_.ParseString(autNoUnreachStr);
		AutDescription descCorrectNoUnreach = parser_.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descCorrectNoUnreach == descOutNoUnreach,
			"\n\nExpecting:\n===========\n" +
			serializer_.Serialize(descCorrectNoUnreach) +
			"===========\n\nGot:\n===========\n" + autNoUnreachStr + "\n===========");
	}
}

BOOST_AUTO_TEST_CASE(aut_remove_useless)
{
	auto testfileContent = ParseTestFile(USELESS_TIMBUK_FILE.string());

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 2, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string inputFile = (AUT_DIR / testcase[0]).string();
		std::string resultFile = (AUT_DIR / testcase[1]).string();

		BOOST_MESSAGE("Removing useless states from " + inputFile + "...");

		std::string autStr = VATA::Util::ReadFile(inputFile);
		std::string autCorrectStr = VATA::Util::ReadFile(resultFile);

		StringToStateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);
		AutDescription autDesc = parser_.ParseString(autStr);

		StateToStateMap translMap;
		AutType autNoUseless = VATA::RemoveUselessStates(aut, &translMap);
		stateDict = VATA::Util::RebindMap(stateDict, translMap);
		translMap.clear();
		autNoUseless = VATA::RemoveUnreachableStates(autNoUseless, &translMap);

		stateDict = VATA::Util::RebindMap(stateDict, translMap);
		std::string autNoUselessStr = dumpAut(autNoUseless, stateDict);

		AutDescription descOutNoUseless = parser_.ParseString(autNoUselessStr);
		AutDescription descCorrectNoUseless = parser_.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descCorrectNoUseless == descOutNoUseless,
			"\n\nExpecting:\n===========\n" +
			serializer_.Serialize(descCorrectNoUseless) +
			"===========\n\nGot:\n===========\n" + autNoUselessStr + "\n===========");
	}
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion)
{
	auto testfileContent = ParseTestFile(INCLUSION_TIMBUK_FILE.string());

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

		StringToStateDict stateDictSmaller;
		AutType autSmaller;
		readAut(autSmaller, stateDictSmaller, autSmallerStr);

		StringToStateDict stateDictBigger;
		AutType autBigger;
		readAut(autBigger, stateDictBigger, autBiggerStr);

		bool doesInclusionHold = VATA::CheckInclusion(autSmaller, autBigger);

		BOOST_CHECK_MESSAGE(expectedResult == doesInclusionHold,
			"\n\nError checking inclusion " + inputSmallerFile + " <= " +
			inputBiggerFile + ": expected " + Convert::ToString(expectedResult) +
			", got " + Convert::ToString(doesInclusionHold));
	}
}

BOOST_AUTO_TEST_SUITE_END()

