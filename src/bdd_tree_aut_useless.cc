/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of pruning useless states of BDD tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_tree_aut_op.hh>

// Standard library headers
#include <stack>

using VATA::AutBase;
using VATA::BDDTreeAut;
using VATA::Util::Convert;

template <>
BDDTreeAut VATA::RemoveUselessStates<BDDTreeAut>(const BDDTreeAut& aut,
	AutBase::StateToStateMap* pTranslMap)
{
	// Assertions
	assert(aut.isValid());

	assert(false);

	return aut;
}
