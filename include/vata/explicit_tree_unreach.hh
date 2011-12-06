/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for RemoveUnreachableStates() on explicit tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_UNREACH_HH_
#define _VATA_EXPLICIT_TREE_UNREACH_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/antichain1c.hh>

// Standard library headers
#include <vector>
#include <unordered_set>

namespace VATA {

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> RemoveUnreachableStates(const ExplicitTreeAut<SymbolType>& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr);

	template <
		class SymbolType,
		class Rel,
		class Index = Util::IdentityTranslator<AutBase::StateType>
	>
	ExplicitTreeAut<SymbolType> RemoveUnreachableStates(const ExplicitTreeAut<SymbolType>& aut,
		const Rel& rel, const Index& index = Index());

}

template <class SymbolType>
VATA::ExplicitTreeAut<SymbolType> VATA::RemoveUnreachableStates(
	const VATA::ExplicitTreeAut<SymbolType>& aut,
	VATA::AutBase::StateToStateMap* pTranslMap = nullptr) {

	typedef VATA::ExplicitTreeAut<SymbolType> ExplicitTA;
	typedef typename ExplicitTA::StateToTransitionClusterMapPtr
		StateToTransitionClusterMapPtr;

	std::unordered_set<AutBase::StateType> reachableStates(aut.GetFinalStates());
	std::vector<AutBase::StateType> newStates(reachableStates.begin(), reachableStates.end());

	while (!newStates.empty()) {

		auto cluster = ExplicitTA::genericLookup(*aut.transitions_, newStates.back());

		newStates.pop_back();

		if (!cluster)
			continue;

		for (auto& symbolStateTupleSetPtr : *cluster) {

			assert(symbolStateTupleSetPtr.second);

			for (auto& stateTuple : *symbolStateTupleSetPtr.second) {

				assert(stateTuple);

				for (auto& state : *stateTuple) {

					if (reachableStates.insert(state).second)
						newStates.push_back(state);

				}

			}

		}

	}

	if (pTranslMap) {

		for (auto& state : reachableStates)
			pTranslMap->insert(std::make_pair(state, state));

	}

	if (reachableStates.size() == aut.transitions_->size())
		return aut;

	ExplicitTA result(aut.cache_);

	result.finalStates_ = aut.finalStates_;
	result.transitions_ = StateToTransitionClusterMapPtr(
		new typename ExplicitTA::StateToTransitionClusterMap()
	);

	for (auto& state : reachableStates) {

		auto iter = aut.transitions_->find(state);

		if (iter == aut.transitions_->end())
			continue;

		result.transitions_->insert(std::make_pair(state, iter->second));

	}

	return result;

}

template <
	class SymbolType,
	class Rel,
	class Index = VATA::Util::IdentityTranslator<VATA::AutBase::StateType>
>
VATA::ExplicitTreeAut<SymbolType> VATA::RemoveUnreachableStates(
	const VATA::ExplicitTreeAut<SymbolType>& aut, const Rel& rel, const Index& index = Index()) {

	typedef ExplicitTreeAut<SymbolType> ExplicitTA;
	typedef typename ExplicitTA::StateToTransitionClusterMap
		StateToTransitionClusterMap;
	typedef typename ExplicitTA::StateToTransitionClusterMapPtr
		StateToTransitionClusterMapPtr;
	typedef typename ExplicitTA::TransitionCluster TransitionCluster;
	typedef typename ExplicitTA::TransitionClusterPtr TransitionClusterPtr;
	typedef typename ExplicitTA::TuplePtrSet TuplePtrSet;
	typedef typename ExplicitTA::TuplePtrSetPtr TuplePtrSetPtr;
	typedef typename ExplicitTA::TuplePtr TuplePtr;
	typedef typename ExplicitTA::StateTuple StateTuple;

	struct TupleCmp {

		const Rel& rel_;
		const Index& index_;

		TupleCmp(const Rel& rel, const Index& index) : rel_(rel), index_(index) {}

		bool operator()(const TuplePtr& lhsPtr, const TuplePtr& rhsPtr) const {

			assert(lhsPtr);
			assert(rhsPtr);

			const StateTuple& lhs = *lhsPtr;
			const StateTuple& rhs = *rhsPtr;

			assert(lhs.size() == rhs.size());

			for (size_t i = 0; i < lhs.size(); ++i) {

				if (!this->rel_.get(this->index_[lhs[i]], this->index_[rhs[i]]))
					return false;

			}

			return true;

		}

	};
		
	Util::Antichain1C<AutBase::StateType> finalStates;

	typename Rel::IndexType ind, inv;

	rel.buildIndex(ind, inv);

	for (auto& state : aut.GetFinalStates()) {

		if (finalStates.contains(ind[state]))
			continue;

		finalStates.refine(inv[state]);
		finalStates.insert(state);

	}

	std::unordered_set<AutBase::StateType> reachableStates(
		finalStates.data().begin(), finalStates.data().end()
	);

	std::vector<AutBase::StateType> newStates(reachableStates.begin(), reachableStates.end());

	Util::SequentialAntichain1C<TuplePtr> tuples;

	auto newTransitions = StateToTransitionClusterMapPtr(
		new typename ExplicitTA::StateToTransitionClusterMap()
	);

	bool transitionsModified = false;

	while (!newStates.empty()) {

		auto state = newStates.back();

		newStates.pop_back();

		auto stateClusterIter = aut.transitions_->find(state);

		if (stateClusterIter == aut.transitions_->end())
			continue;

		bool clusterModified = false;

		auto transitionCluster = TransitionClusterPtr(new TransitionCluster());

		for (auto& symbolStateTupleSetPtr : *stateClusterIter->second) {

			assert(symbolStateTupleSetPtr.second);

			tuples.clear();

			for (auto& stateTuple : *symbolStateTupleSetPtr.second) {

				assert(stateTuple);

				tuples.insert(stateTuple, TupleCmp(rel, index));

			}

			for (auto& stateTuple : tuples.data()) {

				for (auto& state : *stateTuple) {

					if (reachableStates.insert(state).second)
						newStates.push_back(state);

				}

			}

			if (tuples.data().size() == symbolStateTupleSetPtr.second->size()) {

				transitionCluster->insert(symbolStateTupleSetPtr);

				continue;

			}

			clusterModified = true;

			auto tupleSet = TuplePtrSetPtr(new TuplePtrSet());

			for (auto& stateTuple : tuples.data())

				tupleSet->insert(stateTuple);

			transitionCluster->insert(std::make_pair(symbolStateTupleSetPtr.first, tupleSet));

		}

		if (!clusterModified) {

			newTransitions->insert(std::make_pair(state, stateClusterIter->second));

			continue;

		}

		transitionsModified = true;

		newTransitions->insert(std::make_pair(state, transitionCluster));

	}

	if (!transitionsModified && (reachableStates.size() == aut.transitions_->size()) && 
		(finalStates.data().size() == aut.finalStates_.size()))
		return aut;

	ExplicitTA result(aut.cache_);

	result.finalStates_.insert(finalStates.data().begin(), finalStates.data().end());

	if (!transitionsModified && (reachableStates.size() == aut.transitions_->size())) {

		result.transitions_ = aut.transitions_;

		return result;

	}

	result.transitions_ = newTransitions;
	
	return result;

}

#endif
