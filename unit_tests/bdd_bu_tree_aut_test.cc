/*****************************************************************************
 *  VATA Tree Automata Library
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

#include <vata/bdd_td_tree_aut.hh>

// testing headers
#include "log_fixture.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path UNREACHABLE_TIMBUK_FILE =
	AUT_DIR / "bu_unreachable_removal_timbuk.txt";

const fs::path INVERT_TIMBUK_FILE =
	AUT_DIR / "load_timbuk.txt";

/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

class AutTypeFixture
{
protected:// data types

	typedef VATA::BDDBottomUpTreeAut AutType;
	typedef VATA::BDDTopDownTreeAut AutTypeInverted;

	typedef AutType::StateDict StateDict;

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
	auto testfileContent = ParseTestFile(INVERT_TIMBUK_FILE.string());

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 1, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string filename = (AUT_DIR / testcase[0]).string();
		BOOST_MESSAGE("Inverting automaton " + filename + "...");
		std::string autStr = VATA::Util::ReadFile(filename);

		StateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);
		aut = aut.RemoveUselessStates();
		AutTypeInverted invertAut = aut.GetTopDownAut();
		std::string autOut = dumpAut(invertAut, stateDict);

		StateDict stateDictRef;
		AutTypeInverted refAut;
		readAut(refAut, stateDictRef, autStr);
		refAut = refAut.RemoveUselessStates();
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
	testDownwardSimulation();
}

BOOST_AUTO_TEST_CASE(aut_up_inclusion)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::upward);
	testInclusion(ip);
}

BOOST_AUTO_TEST_SUITE_END()
