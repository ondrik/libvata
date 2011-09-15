/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of inclusion on BDD top-down tree automata
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/bdd_td_tree_aut_op.hh>
#include <vata/tree_incl_down.hh>
#include <vata/tree_incl_down_nouseless.hh>

using VATA::BDDTopDownTreeAut;

template <>
bool VATA::CheckInclusion(const BDDTopDownTreeAut& smaller,
	const BDDTopDownTreeAut& bigger)
{
	// Assertions
	assert(smaller.isValid());
	assert(bigger.isValid());

	return CheckDownwardTreeInclusion(smaller, bigger);
}

template <>
bool VATA::CheckInclusionNoUseless(const BDDTopDownTreeAut& smaller,
	const BDDTopDownTreeAut& bigger)
{
	// Assertions
	assert(smaller.isValid());
	assert(bigger.isValid());

	return CheckDownwardTreeInclusionNoUseless(smaller, bigger);
}
