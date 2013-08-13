/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union on BDD bottom-up tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

#include "bdd_bu_tree_aut_core.hh"

// Standard library headers
#include <unordered_map>

using VATA::BDDBUTreeAutCore;
using VATA::Util::Convert;


BDDBUTreeAutCore BDDBUTreeAutCore::Union(
	const BDDBUTreeAutCore&        lhs,
	const BDDBUTreeAutCore&        rhs,
	AutBase::StateToStateMap*      pTranslMapLhs,
	AutBase::StateToStateMap*      pTranslMapRhs)
{
	typedef BDDBUTreeAutCore::StateType StateType;
	typedef BDDBUTreeAutCore::StateTuple StateTuple;
	typedef BDDBUTreeAutCore::TransMTBDD TransMTBDD;
	typedef VATA::AutBase::StateToStateMap StateToStateMap;
	typedef BDDBUTreeAutCore::StateToStateTranslator StateToStateTranslator;


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
		StateToStateMap translMapLhs;
		if (pTranslMapLhs == nullptr)
		{	// copy states
			pTranslMapLhs = &translMapLhs;
		}

		StateToStateMap translMapRhs;
		if (pTranslMapRhs == nullptr)
		{
			pTranslMapRhs = &translMapRhs;
		}

		BDDBUTreeAutCore result;

		StateType stateCnt = 0;
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
		TransMTBDD lhsMtbdd = lhs.ReindexStates(result, stateTransLhs);

		StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);
		TransMTBDD rhsMtbdd = rhs.ReindexStates(result, stateTransRhs);

		result.SetMtbdd(StateTuple(), unionFunc(lhsMtbdd, rhsMtbdd));

		return result;
	}
}
