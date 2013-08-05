/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *		Implementation of union operation for explicit finite word automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_UNION_HH_
#define _VATA_EXPLICIT_FINITE_UNION_HH_

// VATA headers
#include <vata/finite_aut/explicit_finite_aut.hh>

template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::ExplicitFiniteAut<SymbolType>::Union(
	const ExplicitFiniteAut<SymbolType>& lhs,
	const ExplicitFiniteAut<SymbolType>& rhs,
	AutBase::StateToStateMap* pTranslMapLhs,
	AutBase::StateToStateMap* pTranslMapRhs)
{
	typedef AutBase::StateType StateType;
	typedef AutBase::StateToStateTranslator StateToStateTranslator;

	/*
	 * If the maps are not given
	 * it creates own new maps
	 */
	AutBase::StateToStateMap translMapLhs;
	AutBase::StateToStateMap translMapRhs;

	if (!pTranslMapLhs) {
		pTranslMapLhs = &translMapLhs;
	}

	if (!pTranslMapRhs) {
		pTranslMapRhs = &translMapRhs;
	}

	// New translation function
	StateType stateCnt = 0;
	auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

	StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
	StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);

	ExplicitFiniteAut<SymbolType> res;

	lhs.ReindexStates(res, stateTransLhs);
	rhs.ReindexStates(res, stateTransRhs);


	return res;
}


template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::ExplicitFiniteAut<SymbolType>::UnionDisjointStates(
		const ExplicitFiniteAut<SymbolType> &lhs,
		const ExplicitFiniteAut<SymbolType> &rhs)
{
	ExplicitFiniteAut<SymbolType> res(lhs);

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


#endif
