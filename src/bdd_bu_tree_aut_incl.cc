/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of inclusion on BDD bottom-up tree automata
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

using VATA::BDDBottomUpTreeAut;

template <>
bool VATA::CheckInclusionNoUseless(const BDDBottomUpTreeAut& smaller,
	const BDDBottomUpTreeAut& bigger)
{
	// Assertions
	assert(smaller.isValid());
	assert(bigger.isValid());

	return CheckInclusion(smaller, bigger);
}

template <>
bool VATA::CheckInclusion(const BDDBottomUpTreeAut& smaller,
	const BDDBottomUpTreeAut& bigger)
{
	// Assertions
	assert(smaller.isValid());
	assert(bigger.isValid());

	throw std::runtime_error("Unimplemented");
}
