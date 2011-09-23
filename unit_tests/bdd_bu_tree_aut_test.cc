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

class AutTypeFixture
{
protected:// data types

	typedef VATA::BDDBottomUpTreeAut AutType;
	typedef VATA::BDDTopDownTreeAut AutTypeInverted;

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
	auto testfileContent = ParseTestFile(LOAD_TIMBUK_FILE.string());

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
		StateToStateMap translMap;
		AutTypeInverted invertAut = aut.GetTopDownAut(&translMap);

		stateDict = VATA::Util::RebindMap(stateDict, translMap);
		std::string autOut = dumpAut(invertAut, stateDict);

		AutDescription descOrig = parser_.ParseString(autStr);
		AutDescription descOut = parser_.ParseString(autOut);

		BOOST_CHECK_MESSAGE(descOrig == descOut,
			"\n\nExpecting:\n===========\n" +
			std::string(autStr) +
			"===========\n\nGot:\n===========\n" + autOut + "\n===========");
	}
}

BOOST_AUTO_TEST_SUITE_END()

