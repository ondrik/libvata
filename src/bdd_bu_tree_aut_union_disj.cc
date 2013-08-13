/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union with disjoint state sets on BDD bottom-up tree
 *    automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

#include "bdd_bu_tree_aut_core.hh"

using BDDBUTreeAutCore   = VATA::BDDBUTreeAutCore;

using StateType     = BDDBUTreeAutCore::StateType;
using StateTuple    = BDDBUTreeAutCore::StateTuple;
using TransMTBDD    = BDDBUTreeAutCore::TransMTBDD;


BDDBUTreeAutCore BDDBUTreeAutCore::UnionDisjointStates(
	const BDDBUTreeAutCore&   lhs,
	const BDDBUTreeAutCore&   rhs)
{
	BDDBUTreeAutCore::UnionApplyFunctor unionFunc;

	if (BDDBUTreeAutCore::ShareTransTable(lhs, rhs))
	{	// in case the automata share their transition table
		BDDBUTreeAutCore result = lhs;

		StateTuple tuple;
		const TransMTBDD& lhsMTBDD = lhs.GetMtbdd(tuple);
		const TransMTBDD& rhsMTBDD = rhs.GetMtbdd(tuple);

		result.SetMtbdd(tuple, unionFunc(lhsMTBDD, rhsMTBDD));

		return result;
	}
	else
	{	// in case the automata have distinct transition tables
		BDDBUTreeAutCore result = lhs;

		for (const StateType& rhsFst : rhs.GetFinalStates())
		{
			result.SetStateFinal(rhsFst);
		}

		for (auto tupleBddPair : rhs.GetTransTable())
		{
			result.SetMtbdd(tupleBddPair.first, tupleBddPair.second);
		}

		result.SetMtbdd(StateTuple(),
			unionFunc(lhs.GetMtbdd(StateTuple()), rhs.GetMtbdd(StateTuple())));

		return result;
	}
}
