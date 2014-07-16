/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of RemoveUselessStates() on explicit tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

// Standard library headers
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "explicit_tree_aut_core.hh"


using VATA::ExplicitTreeAutCore;

using StateToStateMap    = VATA::AutBase::StateToStateMap;

ExplicitTreeAutCore ExplicitTreeAutCore::RemoveUselessStates(
	StateToStateMap*            pTranslMap) const
{
	struct TransitionInfo
	{
		TuplePtr children_;
		SymbolType symbol_;
		StateType state_;

		std::set<StateType> childrenSet_;

		TransitionInfo(
			const TuplePtr&      children,
			const SymbolType&    symbol,
			const StateType&     state) :
			children_(children),
			symbol_(symbol),
			state_(state),
			childrenSet_(children->begin(), children->end())
		{ }

		bool reachedBy(const StateType& state)
		{
			assert(childrenSet_.count(state));

			childrenSet_.erase(state);

			return childrenSet_.empty();
		}
	};

	typedef std::shared_ptr<TransitionInfo> TransitionInfoPtr;

	std::unordered_map<StateType, std::vector<TransitionInfoPtr>> stateMap;
	std::unordered_set<StateType> reachableStates;
	std::vector<TransitionInfoPtr> reachableTransitions;
	std::vector<StateType> newStates;

	assert(nullptr != transitions_);

	size_t remaining = 0;

	for (auto& stateClusterPair : *transitions_) {

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

					++remaining;

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

			--remaining;

			if (reachableStates.insert(info->state_).second)
				newStates.push_back(info->state_);

		}

	}
/*
	if (pTranslMap) {

		for (auto& state : reachableStates)
			pTranslMap->insert(std::make_pair(state, state));

	}
*/
	ExplicitTreeAutCore result(cache_);

	for (auto& state : finalStates_) {

		if (reachableStates.count(state))
			result.SetStateFinal(state);

	}

	if (!remaining) {

		result.transitions_ = transitions_;

		return result.RemoveUnreachableStates(pTranslMap);

	}

	for (auto& info : reachableTransitions) {

		assert(info);

		result.internalAddTransition(info->children_, info->symbol_, info->state_);

	}

	return result.RemoveUnreachableStates(pTranslMap);

}
