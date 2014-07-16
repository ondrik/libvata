/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of RemoveUnreachableStates() on explicit tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/antichain1c.hh>

// Standard library headers
#include <vector>
#include <unordered_set>


#include "explicit_tree_aut_core.hh"

using VATA::ExplicitTreeAutCore;
using VATA::AutBase;

ExplicitTreeAutCore ExplicitTreeAutCore::RemoveUnreachableStates(
	AutBase::StateToStateMap*            pTranslMap) const
{
	std::unordered_set<StateType> reachableStates(this->GetFinalStates());
	std::vector<StateType> newStates(reachableStates.begin(), reachableStates.end());

	while (!newStates.empty())
	{
		auto cluster = ExplicitTreeAutCore::genericLookup(*transitions_, newStates.back());

		newStates.pop_back();

		if (!cluster)
		{
			continue;
		}

		for (auto& symbolStateTupleSetPtr : *cluster)
		{
			assert(symbolStateTupleSetPtr.second);

			for (auto& stateTuple : *symbolStateTupleSetPtr.second)
			{
				assert(stateTuple);

				for (const StateType& state : *stateTuple)
				{
					if (reachableStates.insert(state).second)
					{
						newStates.push_back(state);
					}
				}
			}
		}
	}

	if (pTranslMap)
	{
		for (const StateType& state : reachableStates)
		{
			pTranslMap->insert(std::make_pair(state, state));
		}
	}

	if (reachableStates.size() == transitions_->size())
	{
		return *this;
	}

	ExplicitTreeAutCore result(cache_);

	result.finalStates_ = finalStates_;
	result.transitions_ = StateToTransitionClusterMapPtr(
		new ExplicitTreeAutCore::StateToTransitionClusterMap()
	);

	for (const StateType& state : reachableStates)
	{
		auto iter = transitions_->find(state);

		if (iter == transitions_->end())
		{
			continue;
		}

		result.transitions_->insert(std::make_pair(state, iter->second));
	}

	return result;
}
