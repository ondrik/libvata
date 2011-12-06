/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union with disjunct state sets on BDD top-down tree
 *    automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_td_tree_aut_op.hh>

using VATA::BDDTopDownTreeAut;
using VATA::Util::Convert;


BDDTopDownTreeAut VATA::UnionDisjunctStates(const BDDTopDownTreeAut& lhs,
	const BDDTopDownTreeAut& rhs)
{
	typedef BDDTopDownTreeAut::StateType StateType;
	typedef BDDTopDownTreeAut::StateTuple StateTuple;
	typedef BDDTopDownTreeAut::TransMTBDD TransMTBDD;


	BDDTopDownTreeAut::UnionApplyFunctor unionFunc;

	if (BDDTopDownTreeAut::ShareTransTable(lhs, rhs))
	{	// in case the automata share their transition table
		BDDTopDownTreeAut result = lhs;

		for (const StateType& fst : rhs.GetFinalStates())
		{
			result.SetStateFinal(fst);
		}

		return result;
	}
	else
	{	// in case the automata have distinct transition tables
		BDDTopDownTreeAut result = lhs;

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
