/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of intersection on BDD bottom-up tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

using VATA::AutBase;
using VATA::BDDBottomUpTreeAut;

template <>
BDDBottomUpTreeAut VATA::Intersection<BDDBottomUpTreeAut>(
	const BDDBottomUpTreeAut& lhs, const BDDBottomUpTreeAut& rhs,
	AutBase::ProductTranslMap* pTranslMap)
{
	// Assertions
	assert(lhs.isValid());
	assert(rhs.isValid());

	assert(false);
}
