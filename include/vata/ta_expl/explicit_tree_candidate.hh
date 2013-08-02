/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2012  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for GetCandidateTree() on explicit tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_CANDIDATE_HH_
#define _VATA_EXPLICIT_TREE_CANDIDATE_HH_

// Standard library headers
#include <vector>
#include <unordered_map>
#include <unordered_set>

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_tree_aut.hh>

namespace VATA {

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> GetCandidateTree(const ExplicitTreeAut<SymbolType>& aut);

}

template <class SymbolType>
VATA::ExplicitTreeAut<SymbolType> VATA::GetCandidateTree(
	const VATA::ExplicitTreeAut<SymbolType>& aut) {

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

			assert(this->childrenSet_.count(state));

			this->childrenSet_.erase(state);

			return this->childrenSet_.empty();

		}

	};

	typedef std::shared_ptr<TransitionInfo> TransitionInfoPtr;

	std::unordered_map<StateType, std::vector<TransitionInfoPtr>> stateMap;
	std::unordered_set<StateType> reachableStates;
	std::vector<TransitionInfoPtr> reachableTransitions;
	std::list<StateType> newStates;

	assert(aut.transitions_);

	size_t remaining = 0;

  // Cycle builds information structure about transitions and also
  // saves the reachable transitions (start states)
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

          // Add a new pair to stateMap and also add the transitionInfor 
          // ptr to the second item of the pair in one step
					stateMap.insert( 
						std::make_pair(s, std::vector<TransitionInfoPtr>())
					).first->second.push_back(transitionInfoPtr);

					++remaining;

				}

			}

		}

	}

	while (!newStates.empty()) {

    // find transition which leads from the chosen state from newStates
		auto i = stateMap.find(newStates.front());

		newStates.pop_front();

		if (i == stateMap.end())
			continue;

    // iterate through all transitions
		for (auto& info : i->second) {

			assert(info);

      // All states of tuple of transition was used
			if (!info->reachedBy(i->first))
				continue;

			--remaining;

      // Insert state, which is accessible from currently chosen transition 
			if (reachableStates.insert(info->state_).second) {

				reachableTransitions.push_back(info);

				newStates.push_back(info->state_);
				if (aut.IsFinalState(info->state_))
					goto found_;

			}

		}

	}
found_:
	ExplicitTA result(aut.cache_);

	for (auto& state : aut.finalStates_) {

		if (reachableStates.count(state))
			result.SetStateFinal(state);

	}

	if (!remaining) {

		result.transitions_ = aut.transitions_;

		return RemoveUnreachableStates(result);

	}

	for (auto& info : reachableTransitions) {

		assert(info);

		result.internalAddTransition(info->children_, info->symbol_, info->state_);

	}

	return RemoveUnreachableStates(result);

}

#endif
