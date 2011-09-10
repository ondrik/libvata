/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of pruning unreachable states of bottom-up BDD tree
 *    automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

using VATA::AutBase;
using VATA::BDDBottomUpTreeAut;

template <>
BDDBottomUpTreeAut VATA::RemoveUnreachableStates<BDDBottomUpTreeAut>(
	const BDDBottomUpTreeAut& aut, AutBase::StateToStateMap* pTranslMap)
{
	// Assertions
	assert(aut.isValid());

	assert(false);
}
