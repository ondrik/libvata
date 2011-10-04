/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for operations on explicit tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_AUT_OP_HH_
#define _VATA_EXPLICIT_TREE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/explicit_tree_isect.hh>
#include <vata/explicit_tree_useless.hh>
#include <vata/explicit_tree_unreach.hh>
#include <vata/explicit_tree_incl_up.hh>
#include <vata/explicit_lts_sim.hh>
#include <vata/down_tree_incl_fctor.hh>
#include <vata/tree_incl_down.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/convert.hh>

namespace VATA {

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> Union(const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr) {

		typedef AutBase::StateType StateType;
		typedef AutBase::StateToStateTranslator StateToStateTranslator;

		AutBase::StateToStateMap translMapLhs;
		AutBase::StateToStateMap translMapRhs;

		if (!pTranslMapLhs)
			pTranslMapLhs = &translMapLhs;

		if (!pTranslMapRhs)
			pTranslMapRhs = &translMapRhs;

		StateType stateCnt = 0;
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
		StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);

		ExplicitTreeAut<SymbolType> res(lhs.cache_);

		lhs.ReindexStates(res, stateTransLhs);
		rhs.ReindexStates(res, stateTransRhs);

		return res;

	}

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> UnionDisjunctStates(const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs) {

		ExplicitTreeAut<SymbolType> res(lhs);

		assert(rhs.transitions_);

		res.uniqueClusterMap()->insert(rhs.transitions_->begin(), rhs.transitions_->end());
		assert(lhs.transitions_->size() + rhs.transitions_->size() == res.transitions_->size());

		res.finalStates_.insert(rhs.finalStates_.begin(), rhs.finalStates_.end());
		assert(lhs.finalStates_.size() + rhs.finalStates_.size() == res.finalStates_.size());

		return res;

	}

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> Intersection(
		const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr
	);

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> RemoveUselessStates(
		const ExplicitTreeAut<SymbolType>& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr);

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> RemoveUnreachableStates(
		const ExplicitTreeAut<SymbolType>& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr);

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const ExplicitTreeAut<SymbolType>& aut) {

		Util::LTS lts;

		aut.ToDownwardLTS(lts);

		AutBase::StateBinaryRelation res;

		computeSimulation(res, lts, aut.transitions_->size());

		return res;

	}

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const ExplicitTreeAut<SymbolType>& aut)
	{
		assert(&aut != nullptr);

		throw std::runtime_error("Unimplemented");
	}

	template <class SymbolType, class Rel>
	bool CheckUpwardInclusionWithPreorder(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger, const Rel& preorder) {

		return ExplicitUpwardInclusion::Check(smaller, bigger, preorder);

	}

	template <class SymbolType, class Rel>
	bool CheckDownwardInclusionWithPreorder(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger, const Rel& preorder) {

		return CheckDownwardTreeInclusion<ExplicitTreeAut<SymbolType>,
			VATA::DownwardInclusionFunctor>(smaller, bigger, preorder);

	}

	template <class SymbolType>
	bool CheckUpwardInclusion(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		typedef AutBase::StateType StateType;
		typedef AutBase::StateToStateMap StateToStateMap;
		typedef AutBase::StateToStateTranslator StateToStateTranslator;

		StateToStateMap stateMap;

		StateType stateCnt = 0;
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTrans(stateMap, translFunc);

		Explicit::TupleCache tupleCache;

		ExplicitTreeAut<SymbolType> newSmaller(tupleCache), newBigger(tupleCache);

		RemoveUselessStates(smaller).ReindexStates(newSmaller, stateTrans);

		stateMap.clear();

		RemoveUnreachableStates(bigger).ReindexStates(newBigger, stateTrans);

		return CheckUpwardInclusionWithPreorder(
			newBigger, newSmaller, Util::Identity(stateCnt)
		);

	}

	template <class SymbolType>
	bool CheckDownwardInclusion(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		typedef AutBase::StateType StateType;
		typedef AutBase::StateToStateMap StateToStateMap;
		typedef AutBase::StateToStateTranslator StateToStateTranslator;
		typedef ExplicitTreeAut<SymbolType> ExplAut;

		StateType stateCnt = 0;
		StateToStateMap stateMap;
		StateToStateTranslator stateTrans(stateMap,
			[&stateCnt](const StateType&){return stateCnt++;});

		Explicit::TupleCache tupleCache;

		ExplAut newSmaller(tupleCache), newBigger(tupleCache);

		RemoveUselessStates(smaller).ReindexStates(newSmaller, stateTrans);

		stateMap.clear();

		RemoveUselessStates(bigger).ReindexStates(newBigger, stateTrans);

		ExplAut unionAut = UnionDisjunctStates(newSmaller, newBigger);

		return CheckDownwardTreeInclusion<ExplAut, VATA::DownwardInclusionFunctor>(
			newSmaller, newBigger, ComputeDownwardSimulation(unionAut)
		);

	}

	template <class SymbolType>
	bool CheckInclusion(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		return CheckUpwardInclusion(smaller, bigger);

	}

}

#endif
