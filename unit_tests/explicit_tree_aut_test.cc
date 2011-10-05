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

const fs::path UNREACHABLE_TIMBUK_FILE =
	AUT_DIR / "td_unreachable_removal_timbuk.txt";


/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

class AutTypeFixture
{
protected:// data types

	typedef VATA::ExplicitTreeAut<size_t> AutType;

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

BOOST_AUTO_TEST_CASE(aut_down_inclusion_sim)
{
	testInclusion(checkDownInclusionWithSimulation);
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_sim_opt)
{
	testInclusion(checkOptDownInclusionWithSimulation);
}

BOOST_AUTO_TEST_CASE(aut_up_inclusion)
{
	testInclusion(checkUpInclusion);
}

BOOST_AUTO_TEST_CASE(aut_up_inclusion_sim)
{
	testInclusion(checkUpInclusionWithSimulation);
}

BOOST_AUTO_TEST_SUITE_END()
