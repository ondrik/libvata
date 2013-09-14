/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *		Implementation of union operation for explicit finite word automata.
 *
 *****************************************************************************/

// VATA headers
#include "explicit_finite_aut_core.hh"

using VATA::ExplicitFiniteAutCore;

ExplicitFiniteAutCore ExplicitFiniteAutCore::Union(
	const ExplicitFiniteAutCore& lhs,
	const ExplicitFiniteAutCore& rhs,
	AutBase::StateToStateMap* pTranslMapLhs,
	AutBase::StateToStateMap* pTranslMapRhs)
{
	/*
	 * If the maps are not given
	 * it creates own new maps
	 */
	StateToStateMap translMapLhs;
	StateToStateMap translMapRhs;

	if (!pTranslMapLhs) {
		pTranslMapLhs = &translMapLhs;
	}

	if (!pTranslMapRhs) {
		pTranslMapRhs = &translMapRhs;
	}

	// New translation function
	StateType stateCnt = 0;
	auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

	StateToStateTranslWeak stateTransLhs(*pTranslMapLhs, translFunc);
	StateToStateTranslWeak stateTransRhs(*pTranslMapRhs, translFunc);

	ExplicitFiniteAutCore res;

	lhs.ReindexStates(res, stateTransLhs);
	rhs.ReindexStates(res, stateTransRhs);


	return res;
}


ExplicitFiniteAutCore ExplicitFiniteAutCore::UnionDisjointStates(
	const ExplicitFiniteAutCore &lhs,
	const ExplicitFiniteAutCore &rhs)
{
	ExplicitFiniteAutCore res(lhs);

	// Use uniqueCluster function, not explicitly transitions_,
	// because of the possibility of the need of creating the
	// new clusterMap
	res.uniqueClusterMap()->insert(rhs.transitions_->begin(),
		rhs.transitions_->end());
	assert(lhs.transitions_->size() + rhs.transitions_->size() == res.transitions_->size());

	res.startStates_.insert(rhs.startStates_.begin(),
		rhs.startStates_.end());

	res.startStateToSymbols_.insert(rhs.startStateToSymbols_.begin(),
		rhs.startStateToSymbols_.end());

	res.finalStates_.insert(rhs.finalStates_.begin(),
		rhs.finalStates_.end());
	return res;
}
