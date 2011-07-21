/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Test suite for BDD based tree automaton
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_tree_aut.hh>

using VATA::BDDTreeAut;


// Boost headers
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BDDTreeAut
#include <boost/test/unit_test.hpp>

// testing headers
#include "log_fixture.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/




/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

/**
 * @brief  BDDTreeAut testing fixture
 *
 * Fixture for test of BDDTreeAut with characters as leaves.
 */
class BDDTreeAutFixture : public LogFixture
{
public:   // public types


};


/******************************************************************************
 *                              Start of testing                              *
 ******************************************************************************/


BOOST_FIXTURE_TEST_SUITE(suite, BDDTreeAutFixture)

BOOST_AUTO_TEST_CASE(import_export)
{
	BOOST_CHECK_MESSAGE(1 == 1, "1 != 1");

	BDDTreeAut aut;
}

BOOST_AUTO_TEST_SUITE_END()

