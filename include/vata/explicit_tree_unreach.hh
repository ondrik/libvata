/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
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

// Standard library headers
#include <vector>
#include <unordered_set>

namespace VATA {

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> RemoveUnreachableStates(
		const ExplicitTreeAut<SymbolType>& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr);

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

#endif
