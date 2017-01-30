/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Test suite for BDD based top-down tree automaton
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_td_tree_aut.hh>

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

	typedef VATA::BDDTopDownTreeAut AutType;

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

BOOST_AUTO_TEST_SUITE_END()
