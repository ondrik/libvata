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
	AUT_DIR / "invert_timbuk.txt";


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

BOOST_AUTO_TEST_SUITE_END()

