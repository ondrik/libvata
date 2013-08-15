/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union with disjoint state sets on BDD top-down tree
 *    automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

#include "bdd_td_tree_aut_core.hh"

using VATA::BDDTDTreeAutCore;

BDDTDTreeAutCore BDDTDTreeAutCore::UnionDisjointStates(
	const BDDTDTreeAutCore&   lhs,
	const BDDTDTreeAutCore&   rhs)
{
	BDDTDTreeAutCore::UnionApplyFunctor unionFunc;

	if (BDDTDTreeAutCore::ShareTransTable(lhs, rhs))
	{	// in case the automata share their transition table
		BDDTDTreeAutCore result = lhs;

		for (const StateType& fst : rhs.GetFinalStates())
		{
			result.SetStateFinal(fst);
		}

		return result;
	}
	else
	{	// in case the automata have distinct transition tables
		BDDTDTreeAutCore result = lhs;

		for (const StateType& rhsFst : rhs.GetFinalStates())
		{
			result.SetStateFinal(rhsFst);
		}

		for (auto tupleBddPair : rhs.GetStates())
		{
			result.SetMtbdd(tupleBddPair.first, tupleBddPair.second);
		}

		return result;
	}
}
