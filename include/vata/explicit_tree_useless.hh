/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for RemoveUselessStates() on explicit tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_USELESS_HH_
#define _VATA_EXPLICIT_TREE_USELESS_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_tree_aut.hh>

// Standard library headers
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace VATA {

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> RemoveUselessStates(
		const ExplicitTreeAut<SymbolType>& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr);

}

template <class SymbolType>
VATA::ExplicitTreeAut<SymbolType> VATA::RemoveUselessStates(
	const VATA::ExplicitTreeAut<SymbolType>& aut,
	VATA::AutBase::StateToStateMap* pTranslMap = nullptr) {

	typedef VATA::ExplicitTreeAut<SymbolType> ExplicitTA;

	typedef typename ExplicitTA::StateType StateType;
	typedef typename ExplicitTA::TuplePtr TuplePtr;

	struct TransitionInfo {

		TuplePtr children_;
		SymbolType symbol_;
		StateType state_;

		std::set<StateType> childrenSet_;

		TransitionInfo(const TuplePtr& children, const SymbolType& symbol, const StateType& state)
			: children_(children), symbol_(symbol), state_(state),
			childrenSet_(children->begin(), children->end()) {
		}

		bool reachedBy(const StateType& state) {

			assert(this->childrenSet.count(state));

			this->childrenSet_.erase(state);

			return this->childrenSet_.empty();

		}

	};

	typedef std::shared_ptr<TransitionInfo> TransitionInfoPtr;

	std::unordered_map<StateType, std::vector<TransitionInfoPtr>> stateMap;
	std::unordered_set<StateType> reachableStates;
	std::vector<TransitionInfoPtr> reachableTransitions;
	std::vector<StateType> newStates;

	for (auto& stateClusterPair : *aut.transitions_) {

		assert(stateClusterPair.second);

		for (auto& symbolTupleSetPair : *stateClusterPair.second) {

			assert(symbolTupleSetPair.second);

			for (auto& tuple : *symbolTupleSetPair.second) {

				assert(tuple);

				auto transitionInfoPtr = TransitionInfoPtr(
					new TransitionInfo(tuple, symbolTupleSetPair.first, stateClusterPair.first)
				);

				if (tuple->empty()) {

					reachableTransitions.push_back(transitionInfoPtr);

					if (reachableStates.insert(stateClusterPair.first).second)
						newStates.push_back(stateClusterPair.first);

					continue;

				}

				for (auto& s : transitionInfoPtr->childrenSet_) {

					stateMap.insert(
						std::make_pair(s, std::vector<TransitionInfoPtr>())
					).first->second.push_back(transitionInfoPtr);
				
				}

			}

		}

	}

	while (!newStates.empty()) {

		auto i = stateMap.find(newStates.back());

		newStates.pop_back();

		if (i == stateMap.end())
			continue;

		for (auto& info : i->second) {

			assert(info);

			if (!info->reachedBy(i->first))
				continue;

			reachableTransitions.push_back(info);

			if (reachableStates.insert(info->state_).second)
				newStates.push_back(info->state_);

		}

	}

	if (pTranslMap) {

		for (auto& state : reachableStates)
			pTranslMap->insert(std::make_pair(state, state));

	}

	ExplicitTA result(aut.cache_);

	for (auto& state : aut.finalStates_) {

		if (reachableStates.count(state))
			result.SetStateFinal(state);

	}

	if (reachableStates.size() == aut.transitions_->size()) {

		result.transitions_ = aut.transitions_;

		return result;

	}

	for (auto& info : reachableTransitions) {

		assert(info);

		result.internalAddTransition(info->children_, info->symbol_, info->state_);

	}

	return result;

}

#endif
