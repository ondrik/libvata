/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of pruning useless states of BDD bottom-up tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

// Standard library headers
#include <stack>

using VATA::AutBase;
using VATA::BDDBottomUpTreeAut;
using VATA::Util::Convert;

template <>
BDDBottomUpTreeAut VATA::RemoveUselessStates<BDDBottomUpTreeAut>(
	const BDDBottomUpTreeAut& aut, AutBase::StateToStateMap* pTranslMap)
{
	// Assertions
	assert(aut.isValid());

	assert(false);
}
