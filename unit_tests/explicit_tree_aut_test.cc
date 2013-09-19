/*****************************************************************************
 *  VATA Tree Automata Library
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

#include "log_fixture.hh"

/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path UNREACHABLE_TIMBUK_FILE =
	AUT_DIR / "td_unreachable_removal_timbuk.txt";

/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

class AutTypeFixture
{
protected:// data types

	typedef VATA::ExplicitTreeAut AutType;

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
	testDownwardSimulation();
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_nonrec_nosim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::downward);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_nonrec_sim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::downward);
	ip.SetUseSimulation(true);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_rec_sim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::downward);
	ip.SetUseRecursion(true);
	ip.SetUseSimulation(true);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_opt_rec_sim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::downward);
	ip.SetUseDownwardCacheImpl(true);
	ip.SetUseRecursion(true);
	ip.SetUseSimulation(true);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_up_inclusion)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::upward);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_up_inclusion_sim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::upward);
	ip.SetUseSimulation(true);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(iterators)
{
	auto testfileContent = ParseTestFile(LOAD_TIMBUK_FILE.string());
	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 1, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string filename = (AUT_DIR / testcase[0]).string();
		BOOST_MESSAGE("Checking iteration over automaton " + filename + "...");
		std::string autStr = VATA::Util::ReadFile(filename);

		StateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);

		for (const Transition& trans : aut)
		{
			BOOST_REQUIRE_MESSAGE(aut.ContainsTransition(trans),
				"Inconsistent iterator output: " + aut.ToString(trans) +
				" is claimed not to be in aut");
		}
	}
}

BOOST_AUTO_TEST_CASE(accept_iterators)
{
	auto testfileContent = ParseTestFile(LOAD_TIMBUK_FILE.string());
	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 1, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string filename = (AUT_DIR / testcase[0]).string();
		BOOST_MESSAGE("Checking iteration over accepting transitions of " + filename + "...");
		std::string autStr = VATA::Util::ReadFile(filename);

		StateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);

		for (const Transition& trans : aut.GetAcceptTrans())
		{
			BOOST_REQUIRE_MESSAGE(aut.ContainsTransition(trans),
				"Inconsistent iterator output: " + aut.ToString(trans) +
				" is claimed not to be in aut");

			BOOST_REQUIRE_MESSAGE(aut.IsStateFinal(trans.GetParent()),
				"Inconsistent iterator output: " + aut.ToString(trans) +
				" is not accepting");
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
