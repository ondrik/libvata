/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2012  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of GetCandidateTree() on explicit tree automata.
 *
 *****************************************************************************/

// Standard library headers
#include <vector>
#include <unordered_map>
#include <unordered_set>

// VATA headers
#include <vata/vata.hh>

#include "explicit_tree_aut_core.hh"

using VATA::ExplicitTreeAutCore;

ExplicitTreeAutCore ExplicitTreeAutCore::GetCandidateTree() const
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
	std::list<StateType> newStates;

	assert(nullptr != transitions_);

	size_t remaining = 0;

	// Cycle builds information structure about transitions and also
	// saves the reachable transitions (start states)
	for (auto& stateClusterPair : *transitions_)
	{
		assert(stateClusterPair.second);

		for (auto& symbolTupleSetPair : *stateClusterPair.second)
		{
			assert(symbolTupleSetPair.second);

			for (auto& tuple : *symbolTupleSetPair.second)
			{
				assert(tuple);

				auto transitionInfoPtr = TransitionInfoPtr(
					new TransitionInfo(tuple, symbolTupleSetPair.first, stateClusterPair.first)
				);

				if (tuple->empty())
				{
					reachableTransitions.push_back(transitionInfoPtr);

					if (reachableStates.insert(stateClusterPair.first).second)
					{
						newStates.push_back(stateClusterPair.first);
					}

					continue;
				}

				for (auto& s : transitionInfoPtr->childrenSet_)
				{
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

	while (!newStates.empty())
	{
		// find transition which leads from the chosen state from newStates
		auto i = stateMap.find(newStates.front());

		newStates.pop_front();

		if (i == stateMap.end())
		{
			continue;
		}

		// iterate through all transitions
		for (auto& info : i->second)
		{
			assert(info);

			// All states of tuple of transition was used
			if (!info->reachedBy(i->first))
			{
				continue;
			}

			--remaining;

			// Insert state, which is accessible from currently chosen transition
			if (reachableStates.insert(info->state_).second)
			{
				reachableTransitions.push_back(info);

				newStates.push_back(info->state_);
				if (this->IsStateFinal(info->state_))
				{
					goto found_;
				}
			}
		}
	}

found_:

	ExplicitTreeAutCore result(cache_);

	for (const StateType& state : finalStates_)
	{
		if (reachableStates.count(state))
		{
			result.SetStateFinal(state);
		}
	}

	if (!remaining)
	{
		result.transitions_ = transitions_;

		return result.RemoveUnreachableStates();
	}

	for (auto& info : reachableTransitions)
	{
		assert(info);

		result.internalAddTransition(info->children_, info->symbol_, info->state_);
	}

	return result.RemoveUnreachableStates();
}
