/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union on explicit tree automata.
 *
 *****************************************************************************/

#include "explicit_tree_aut_core.hh"

using VATA::ExplicitTreeAutCore;

ExplicitTreeAutCore ExplicitTreeAutCore::Union(
	const ExplicitTreeAutCore&            lhs,
	const ExplicitTreeAutCore&            rhs,
	AutBase::StateToStateMap*             pTranslMapLhs,
	AutBase::StateToStateMap*             pTranslMapRhs)
{
	StateToStateMap translMapLhs;
	StateToStateMap translMapRhs;

	if (!pTranslMapLhs)
	{
		pTranslMapLhs = &translMapLhs;
	}

	if (!pTranslMapRhs)
	{
		pTranslMapRhs = &translMapRhs;
	}

	StateType stateCnt = 0;
	auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

	StateToStateTranslWeak stateTransLhs(*pTranslMapLhs, translFunc);
	StateToStateTranslWeak stateTransRhs(*pTranslMapRhs, translFunc);

	ExplicitTreeAutCore res(lhs.cache_);

	lhs.ReindexStates(res, stateTransLhs);
	rhs.ReindexStates(res, stateTransRhs);

	return res;
}


ExplicitTreeAutCore ExplicitTreeAutCore::UnionDisjointStates(
	const ExplicitTreeAutCore&        lhs,
	const ExplicitTreeAutCore&        rhs)
{
	ExplicitTreeAutCore res(lhs);

	assert(rhs.transitions_);

	res.uniqueClusterMap()->insert(rhs.transitions_->begin(), rhs.transitions_->end());
	assert(lhs.transitions_->size() + rhs.transitions_->size() == res.transitions_->size());

	res.finalStates_.insert(rhs.finalStates_.begin(), rhs.finalStates_.end());
	assert(lhs.finalStates_.size() + rhs.finalStates_.size() == res.finalStates_.size());

	return res;
}
