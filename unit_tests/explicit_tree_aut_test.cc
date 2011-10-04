/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Test suite for explicit tree automaton
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/explicit_tree_aut_op.hh>

// testing headers
#include "log_fixture.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path LOAD_TIMBUK_FILE =
	AUT_DIR / "load_timbuk.txt";

const fs::path UNREACHABLE_TIMBUK_FILE =
	AUT_DIR / "td_unreachable_removal_timbuk.txt";

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

class AutTypeFixture
{
protected:// data types

	typedef VATA::ExplicitTreeAut<size_t> AutType;

	typedef AutType::StateType StateType;
	typedef AutType::StateToStateMap StateToStateMap;
	typedef AutType::StateToStateTranslator StateToStateTranslator;
	typedef AutType::StateBinaryRelation StateBinaryRelation;
	typedef AutType::StringToStateDict StringToStateDict;

	typedef VATA::Util::TranslatorStrict<StringToStateDict>
		StringToStateStrictTranslator;

protected:// data members

	AutType::SymbolType nextSymbol_;

protected:// methods

	AutTypeFixture() :
		nextSymbol_(0)
	{ }

	virtual ~AutTypeFixture() { }
};

#include "tree_aut_test.hh"

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
