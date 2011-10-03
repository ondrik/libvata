/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Test suite for BDD based bottom-up tree automaton
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_bu_tree_aut_op.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/bdd_td_tree_aut_op.hh>

// testing headers
#include "log_fixture.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path LOAD_TIMBUK_FILE =
	AUT_DIR / "load_timbuk.txt";

const fs::path UNREACHABLE_TIMBUK_FILE =
	AUT_DIR / "bu_unreachable_removal_timbuk.txt";

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

const fs::path INVERT_TIMBUK_FILE =
	AUT_DIR / "load_timbuk.txt";

const fs::path DOWN_SIM_TIMBUK_FILE =
	AUT_DIR / "down_sim_timbuk.txt";

/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

class AutTypeFixture
{
protected:// data types

	typedef VATA::BDDBottomUpTreeAut AutType;
	typedef VATA::BDDTopDownTreeAut AutTypeInverted;

	typedef AutType::StateType StateType;
	typedef AutType::StateToStateMap StateToStateMap;
	typedef AutType::StateToStateTranslator StateToStateTranslator;
	typedef AutType::StateBinaryRelation StateBinaryRelation;
	typedef AutType::StringToStateDict StringToStateDict;

	typedef VATA::Util::TranslatorStrict<StringToStateDict>
		StringToStateStrictTranslator;

private:  // constants

	static const size_t BDD_SIZE = 16;

protected:// data members

	AutType::SymbolType nextSymbol_;

protected:// methods

	AutTypeFixture() :
		nextSymbol_(BDD_SIZE, 0)
	{ }

	virtual ~AutTypeFixture() { }
};

#include "tree_aut_test.hh"


BOOST_AUTO_TEST_CASE(aut_inversion)
{
	AutTypeInverted::SetSymbolDictPtr(&AutType::GetSymbolDict());
	auto testfileContent = ParseTestFile(INVERT_TIMBUK_FILE.string());

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 1, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string filename = (AUT_DIR / testcase[0]).string();
		BOOST_MESSAGE("Inverting automaton " + filename + "...");
		std::string autStr = VATA::Util::ReadFile(filename);

		StringToStateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);
		aut = RemoveUselessStates(aut);
		AutTypeInverted invertAut = aut.GetTopDownAut();
		std::string autOut = dumpAut(invertAut, stateDict);

		StringToStateDict stateDictRef;
		AutTypeInverted refAut;
		readAut(refAut, stateDictRef, autStr);
		refAut = RemoveUselessStates(refAut);
		std::string refOut = dumpAut(refAut, stateDictRef);

		AutDescription descOrig = parser_.ParseString(refOut);
		AutDescription descOut = parser_.ParseString(autOut);
		BOOST_CHECK_MESSAGE(descOrig == descOut,
			"\n\nExpecting:\n===========\n" +
			std::string(refOut) +
			"===========\n\nGot:\n===========\n" + autOut + "\n===========");
	}
}

BOOST_AUTO_TEST_CASE(aut_down_simulation)
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

		StringToStateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);

		StateType stateCnt = 0;
		StateToStateMap stateMap;
		StateToStateTranslator stateTrans(stateMap,
			[&stateCnt](const StateType&){return stateCnt++;});

		aut = VATA::RemoveUselessStates(aut);
		AutType reindexedAut;
		aut.ReindexStates(reindexedAut, stateTrans);
		for (const StateType& fst : aut.GetFinalStates())
		{
			reindexedAut.SetStateFinal(stateTrans(fst));
		}

		stateDict = RebindMap(stateDict, stateMap);

		StateBinaryRelation sim = VATA::ComputeDownwardSimulation(
			reindexedAut);

		auto simulationContent = ParseTestFile(resultFile);
		StateBinaryRelation refSim(stateCnt);

		StringToStateStrictTranslator stateStrictTrans(stateDict);

		for (auto& simulationLine : simulationContent)
		{	// load the reference relation
			assert(simulationLine.size() == 3);
			assert(simulationLine[1] == "<=");

			StateType firstState;
			StateType secondState;

			StringToStateDict::const_iterator itDictFirst;
			StringToStateDict::const_iterator itDictSecond;
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

BOOST_AUTO_TEST_CASE(aut_down_inclusion_sim)
{
	testInclusion(VATA::CheckDownwardInclusionWithSimulation);
}

BOOST_AUTO_TEST_CASE(aut_up_inclusion)
{
	testInclusion(VATA::CheckUpwardInclusion);
}

BOOST_AUTO_TEST_SUITE_END()
