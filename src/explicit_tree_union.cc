/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union on explicit tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/ta_expl/explicit_tree_aut.hh>

using VATA::ExplicitTreeAut;

ExplicitTreeAut ExplicitTreeAut::Union(
	const ExplicitTreeAut&                lhs,
	const ExplicitTreeAut&                rhs,
	AutBase::StateToStateMap*             pTranslMapLhs,
	AutBase::StateToStateMap*             pTranslMapRhs)
{
	typedef AutBase::StateType StateType;
	typedef AutBase::StateToStateTranslator StateToStateTranslator;

	AutBase::StateToStateMap translMapLhs;
	AutBase::StateToStateMap translMapRhs;

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

	StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
	StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);

	ExplicitTreeAut res(lhs.cache_);

	lhs.ReindexStates(res, stateTransLhs);
	rhs.ReindexStates(res, stateTransRhs);

	return res;
}


ExplicitTreeAut ExplicitTreeAut::UnionDisjointStates(
	const ExplicitTreeAut&           lhs,
	const ExplicitTreeAut&           rhs)
{
	ExplicitTreeAut res(lhs);

	assert(rhs.transitions_);

	res.uniqueClusterMap()->insert(rhs.transitions_->begin(), rhs.transitions_->end());
	assert(lhs.transitions_->size() + rhs.transitions_->size() == res.transitions_->size());

	res.finalStates_.insert(rhs.finalStates_.begin(), rhs.finalStates_.end());
	assert(lhs.finalStates_.size() + rhs.finalStates_.size() == res.finalStates_.size());

	return res;
}
