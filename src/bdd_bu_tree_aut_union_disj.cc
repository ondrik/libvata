/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union with disjunct state sets on BDD bottom-up tree
 *    automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

using VATA::BDDBottomUpTreeAut;
using VATA::Util::Convert;


BDDBottomUpTreeAut VATA::UnionDisjunctStates(const BDDBottomUpTreeAut& lhs,
	const BDDBottomUpTreeAut& rhs)
{
	typedef BDDBottomUpTreeAut::StateType StateType;
	typedef BDDBottomUpTreeAut::StateTuple StateTuple;
	typedef BDDBottomUpTreeAut::TransMTBDD TransMTBDD;


	BDDBottomUpTreeAut::UnionApplyFunctor unionFunc;

	if (BDDBottomUpTreeAut::ShareTransTable(lhs, rhs))
	{	// in case the automata share their transition table
		BDDBottomUpTreeAut result = lhs;

		StateTuple tuple;
		const TransMTBDD& lhsMTBDD = lhs.GetMtbdd(tuple);
		const TransMTBDD& rhsMTBDD = rhs.GetMtbdd(tuple);

		result.SetMtbdd(tuple, unionFunc(lhsMTBDD, rhsMTBDD));

		return result;
	}
	else
	{	// in case the automata have distinct transition tables
		BDDBottomUpTreeAut result = lhs;

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
