/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union on BDD bottom-up tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

using VATA::BDDBottomUpTreeAut;

// Standard library headers
#include <unordered_map>


template <>
BDDBottomUpTreeAut VATA::Union<BDDBottomUpTreeAut>(const BDDBottomUpTreeAut& lhs,
	const BDDBottomUpTreeAut& rhs, AutBase::StateToStateMap* pTranslMap)
{
	// Assertions
	assert(lhs.isValid());
	assert(rhs.isValid());

	assert(false);
}
