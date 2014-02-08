/*****************************************************************************
 *  VATA Tree Automata Library
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
using VATA::InclParam;
using VATA::SimParam;
using VATA::Parsing::TimbukParser;
using VATA::Serialization::TimbukSerializer;
using VATA::Util::AutDescription;
using VATA::Util::Convert;

// Boost headers
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE AutType
#include <boost/test/unit_test.hpp>


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path LOAD_TIMBUK_FILE =
	AUT_DIR / "load_timbuk.txt";

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

const fs::path DOWN_SIM_TIMBUK_FILE =
	AUT_DIR / "down_sim_timbuk.txt";


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

	using StateDict = AutBase::StateDict;
	using SymbolDict = AutType::SymbolDict;

	using StateToStateMap        = AutBase::StateToStateMap;
	using StateToStateTranslWeak = AutType::StateToStateTranslWeak;
	using StateType              = AutBase::StateType;
	using StateTuple             = AutType::StateTuple;
	using SymbolType             = AutType::SymbolType;
	using StringSymbolType       = AutType::StringSymbolType;
	using StateBinaryRelation    = AutType::StateBinaryRelation;

	using StringToStateTranslStrict  = AutType::StringToStateTranslStrict;
	using StringToStateTranslWeak    = AutType::StringToStateTranslWeak;

	using StateBackTranslStrict         = AutType::StateBackTranslStrict;
	using SymbolBackTranslStrict        = AutType::SymbolBackTranslStrict;
	using StringSymbolToSymbolTranslWeak= AutType::StringSymbolToSymbolTranslWeak;

	using Transition = AutType::Transition;

protected:// data members

	TimbukParser parser_;
	TimbukSerializer serializer_;

protected:// methods

	TreeAutFixture() :
		parser_(),
		serializer_()
	{ }

	template <class Automaton>
	void readAut(
		Automaton&                       aut,
		StringToStateTranslWeak&         stateTransl,
		const std::string&               str)
	{
		aut.LoadFromString(parser_, str, stateTransl);
	}

	template <class Automaton>
	void readAut(
		Automaton&              aut,
		StateDict&              stateDict,
		const std::string&      str)
	{
		aut.LoadFromString(parser_, str, stateDict);
	}

	template <class Automaton>
	void readAut(
		Automaton&              aut,
		const std::string&      str)
	{
		aut.LoadFromString(parser_, str);
	}

	template <class Automaton>
	std::string dumpAut(
		const Automaton&           aut,
		const StateDict&           stateDict)
	{
		return aut.DumpToString(serializer_, stateDict);
	}

	void testInclusion(VATA::InclParam& ip)
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

			AutType autSmaller;
			readAut(autSmaller, autSmallerStr);

			AutType autBigger;
			readAut(autBigger, autBiggerStr);

			// prepare the automata
			AutBase::StateType states =
				AutBase::SanitizeAutsForInclusion(autSmaller, autBigger);

			// the simulation (if present)
			StateBinaryRelation sim;

			if (ip.GetUseSimulation())
			{	// if there is simulation, we need to compute it
				AutType unionAut = AutType::UnionDisjointStates(autSmaller, autBigger);
				if (InclParam::e_direction::downward == ip.GetDirection())
				{	// downward direction
					VATA::SimParam sp;
					sp.SetRelation(VATA::SimParam::e_sim_relation::TA_DOWNWARD);
					sp.SetNumStates(states);
					sim = unionAut.ComputeSimulation(sp);
				}
				else if (InclParam::e_direction::upward == ip.GetDirection())
				{	// upward direction
					VATA::SimParam sp;
					sp.SetRelation(VATA::SimParam::e_sim_relation::TA_UPWARD);
					sp.SetNumStates(states);
					sim = unionAut.ComputeSimulation(sp);
				}
				else
				{
					assert(false);       // fail gracefully
				}

				ip.SetSimulation(&sim);
			}

			bool doesInclusionHold = AutType::CheckInclusion(autSmaller, autBigger, ip);

			BOOST_CHECK_MESSAGE(expectedResult == doesInclusionHold,
				"\n\nError checking inclusion " + inputSmallerFile + " <= " +
				inputBiggerFile + ": expected " + Convert::ToString(expectedResult) +
				", got " + Convert::ToString(doesInclusionHold));
		}
	}

	void testDownwardSimulation()
	{
		auto testfileContent = ParseTestFile(DOWN_SIM_TIMBUK_FILE.string());

		for (auto testcase : testfileContent)
		{
			BOOST_REQUIRE_MESSAGE(testcase.size() == 2, "Invalid format of a testcase: " +
				Convert::ToString(testcase));

			std::string inputFile = (AUT_DIR / testcase[0]).string();
			std::string resultFile = (AUT_DIR / testcase[1]).string();

			BOOST_MESSAGE("Computing downward simulation for " + inputFile + "...");

			std::string autStr = VATA::Util::ReadFile(inputFile);
			std::string correctSimStr = VATA::Util::ReadFile(resultFile);

			StateDict stateDict;
			AutType aut;
			readAut(aut, stateDict, autStr);

			StateType stateCnt = 0;
			StateToStateMap stateMap;
			StateToStateTranslWeak stateTrans(stateMap,
				[&stateCnt](const StateType&){return stateCnt++;});

			aut = aut.RemoveUselessStates();
			AutType reindexedAut = aut.ReindexStates(stateTrans);

			stateDict = RebindMap(stateDict, stateMap);

			SimParam sp;
			sp.SetRelation(VATA::SimParam::e_sim_relation::TA_DOWNWARD);
			sp.SetNumStates(stateCnt);
			StateBinaryRelation sim = reindexedAut.ComputeSimulation(sp);

			auto simulationContent = ParseTestFile(resultFile);
			StateBinaryRelation refSim(stateCnt);

			StringToStateTranslStrict stateStrictTrans(stateDict);

			for (auto& simulationLine : simulationContent)
			{	// load the reference relation
				assert(simulationLine.size() == 3);
				assert(simulationLine[1] == "<=");

				StateType firstState;
				StateType secondState;

				StateDict::const_iterator itDictFirst;
				StateDict::const_iterator itDictSecond;
				if (((itDictFirst = stateDict.FindFwd(simulationLine[0]))
					== stateDict.EndFwd()) ||
					((itDictSecond = stateDict.FindFwd(simulationLine[2]))
					== stateDict.EndFwd()))
				{
					continue;
				}

				firstState = itDictFirst->second;
				secondState = itDictSecond->second;

				refSim.set(firstState, secondState, true);
			}

			for (const auto& firstStringStatePair : stateDict)
			{
				for (const auto& secondStringStatePair : stateDict)
				{
					const std::string& firstName = firstStringStatePair.first;
					const StateType& firstState = firstStringStatePair.second;
					const std::string& secondName = secondStringStatePair.first;
					const StateType& secondState = secondStringStatePair.second;

					BOOST_CHECK_MESSAGE(sim.get(firstState, secondState)
						== refSim.get(firstState, secondState),
						"Invalid simulation value for (" + firstName + ", " + secondName +
						"): got "
						+ Convert::ToString(sim.get(firstState, secondState)) + ", expected " +
						Convert::ToString(refSim.get(firstState, secondState)));
				}
			}
		}
	}

	template <
		class AutProcFunc>
	void runOnAutomataSet(
		AutProcFunc           procFunc)
	{
		auto testfileContent = ParseTestFile(LOAD_TIMBUK_FILE.string());
		for (auto testcase : testfileContent)
		{
			BOOST_REQUIRE_MESSAGE(testcase.size() == 1, "Invalid format of a testcase: " +
				Convert::ToString(testcase));

			std::string filename = (AUT_DIR / testcase[0]).string();
			std::string autStr = VATA::Util::ReadFile(filename);

			StateDict stateDict;
			AutType aut;
			readAut(aut, stateDict, autStr);

			procFunc(aut, stateDict, filename);
		}
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

		StateDict stateDict;
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

BOOST_AUTO_TEST_CASE(adding_transitions)
{
	auto testfileContent = ParseTestFile(ADD_TRANS_TIMBUK_FILE.string());

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

		StateType state(0);

		AutType aut;
		StateDict stateDict;
		StringToStateTranslWeak stateTransl(
			/* state dictionary */ stateDict,
			/* generator of new states */ [&state](const std::string&){return state++;});

		readAut(aut, stateTransl, autStr);

		AutDescription autDesc = parser_.ParseString(autStr);

		AutDescription transDesc = parser_.ParseString(transStr);

		for (const AutDescription::Transition& trans : transDesc.transitions)
		{
			const std::string& parStr = trans.third;
			const std::string& symbolStr = trans.second;

			// get the parent state
			StateType parState = stateTransl(parStr);

			if (transDesc.finalStates.find(parStr) != transDesc.finalStates.end())
			{	// if the parent state is final
				aut.SetStateFinal(parState);
			}

			SymbolType symbol = (*aut.GetAlphabet()->GetSymbolTransl())(AutType::ToStringSymbolType(symbolStr, trans.first.size()));

			StateTuple children;
			for (const std::string& childStr : trans.first)
			{	// for each child
				StateType childState = stateTransl(childStr);

				if (transDesc.finalStates.find(childStr) != transDesc.finalStates.end())
				{	// if the child state is final
					aut.SetStateFinal(childState);
				}

				children.push_back(childState);
			}

			aut.AddTransition(children, symbol, parState);
		}

		std::string autTransStr = aut.DumpToString(serializer_, stateDict);

		AutDescription descOut = parser_.ParseString(autTransStr);
		AutDescription descCorrect = parser_.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descOut == descCorrect,
			"\n\nExpecting:\n===========\n" + autCorrectStr +
			"===========\n\nGot:\n===========\n" + autTransStr + "\n===========");
	}



//	// get state "q"
//	//BDDTopDownTreeAut::StateType stateQ = stateDict.TranslateFwd("q");
//	// get state "p"
//	BDDTopDownTreeAut::StateType stateP = stateDict.TranslateFwd("p");
//	// insert state "qa"
//	BDDTopDownTreeAut::StateType stateQA = aut.AddState();
//	stateDict.Insert(std::make_pair("qa", stateQA));
//
//	// add the following transition: a -> qa, to the description ...
//	AutDescription::Transition newTransition(std::vector<std::string>(), "a", "qa");
//	descCorrect.transitions.insert(newTransition);
//	// ... and to the automaton
//	aut.AddSimplyTransition(BDDTopDownTreeAut::StateTuple(),
//		BDDTopDownTreeAut::TranslateStringToSymbol("a"), stateQA);
//
//	// add the following transition: a(qa, qa) -> p, to the description ...
//	std::vector<std::string> childrenStr;
//	childrenStr.push_back("qa");
//	childrenStr.push_back("qa");
//	newTransition = AutDescription::Transition(childrenStr, "a", "p");
//	descCorrect.transitions.insert(newTransition);
//	// ... and to the automaton
//	BDDTopDownTreeAut::StateTuple children;
//	children.push_back(stateQA);
//	children.push_back(stateQA);
//	aut.AddSimplyTransition(children, BDDTopDownTreeAut::TranslateStringToSymbol("a"),
//		stateP);
//
//	std::string autOut = aut.DumpToString(serializer_, &stateDict);
//	AutDescription descOut = parser_.ParseString(autOut);
//
//	BOOST_CHECK_MESSAGE(descCorrect == descOut,
//		"\n\nExpecting:\n===========\n" +
//		serializer_.Serialize(descCorrect) +
//		"===========\n\nGot:\n===========\n" + autOut + "\n===========");
}


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

		StateDict stateDictLhs;
		AutType autLhs;
		readAut(autLhs, stateDictLhs, autLhsStr);
		AutDescription autLhsDesc = parser_.ParseString(autLhsStr);

		StateDict stateDictRhs;
		AutType autRhs(autLhs.GetTransTable());
		readAut(autRhs, stateDictRhs, autRhsStr);
		AutDescription autRhsDesc = parser_.ParseString(autRhsStr);

		AutType autUnion = VATA::Union(autLhs, autRhs);
		StateDict stateDictUnion =
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

		StateDict stateDictLhs;
		AutType autLhs;
		readAut(autLhs, stateDictLhs, autLhsStr);
		AutDescription autLhsDesc = parser_.ParseString(autLhsStr);

		StateDict stateDictRhs;
		AutType autRhs;
		readAut(autRhs, stateDictRhs, autRhsStr);
		AutDescription autRhsDesc = parser_.ParseString(autRhsStr);

		AutBase::StateToStateMap stateTranslMapLhs;
		AutBase::StateToStateMap stateTranslMapRhs;
		AutType autUnion = AutType::Union(autLhs, autRhs, &stateTranslMapLhs,
			&stateTranslMapRhs);
		StateDict stateDictUnion =
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

		StateDict stateDictLhs;
		AutType autLhs;
		readAut(autLhs, stateDictLhs, autLhsStr);
		AutDescription autLhsDesc = parser_.ParseString(autLhsStr);

		StateDict stateDictRhs;
		AutType autRhs;
		readAut(autRhs, stateDictRhs, autRhsStr);
		AutDescription autRhsDesc = parser_.ParseString(autRhsStr);

		AutBase::ProductTranslMap translMap;
		AutType autIntersect = AutType::Intersection(autLhs, autRhs, &translMap);

		StateDict stateDictIsect = VATA::Util::CreateProductStringToStateMap(
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

		StateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);
		AutDescription autDesc = parser_.ParseString(autStr);

		StateToStateMap translMap;
		AutType autNoUnreach = aut.RemoveUnreachableStates();
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

		StateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);
		AutDescription autDesc = parser_.ParseString(autStr);

		AutType autNoUseless = aut.RemoveUselessStates();
		std::string autNoUselessStr = dumpAut(autNoUseless, stateDict);

		AutDescription descOutNoUseless = parser_.ParseString(autNoUselessStr);
		AutDescription descCorrectNoUseless = parser_.ParseString(autCorrectStr);

		BOOST_CHECK_MESSAGE(descCorrectNoUseless == descOutNoUseless,
			"\n\nExpecting:\n===========\n" +
			serializer_.Serialize(descCorrectNoUseless) +
			"===========\n\nGot:\n===========\n" + autNoUselessStr + "\n===========");
	}
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_rec_nosim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::downward);
	ip.SetUseRecursion(true);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_opt_rec_nosim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::downward);
	ip.SetUseDownwardCacheImpl(true);
	ip.SetUseRecursion(true);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(final_states_test)
{
	this->runOnAutomataSet(
		[](const AutType& aut, const StateDict& stateDict, const std::string& filename)
		{
			BOOST_MESSAGE("Checking final states for " + filename + "...");
			for (const StateType& state : aut.GetFinalStates())
			{
				BOOST_REQUIRE_MESSAGE(aut.IsStateFinal(state),
					"Inconsistent final state: " + Convert::ToString(state));
			}
		});
}
