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
#include <vata/down_tree_incl_fctor.hh>
#include <vata/down_tree_incl_nouseless_fctor.hh>
#include <vata/tree_incl_down.hh>

using VATA::BDDTopDownTreeAut;

bool VATA::CheckInclusion(const BDDTopDownTreeAut& smaller,
	const BDDTopDownTreeAut& bigger)
{
	// Assertions
	assert(smaller.isValid());
	assert(bigger.isValid());

	return CheckDownwardTreeInclusion<BDDTopDownTreeAut,
		VATA::DownwardInclusionFunctor>(smaller, bigger);
}

bool VATA::CheckInclusionNoUseless(const BDDTopDownTreeAut& smaller,
	const BDDTopDownTreeAut& bigger)
{
	// Assertions
	assert(smaller.isValid());
	assert(bigger.isValid());

	return CheckDownwardTreeInclusion<BDDTopDownTreeAut,
		VATA::DownwardInclusionNoUselessFunctor>(smaller, bigger);
}
