/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for operations on BDD tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TREE_AUT_OP_HH_
#define _VATA_BDD_TREE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_op.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/tree_incl_down.hh>

#include <unordered_set>

namespace VATA
{

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> Union(const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs, AutBase::StateToStateMap* pTranslMap);

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> Intersection(const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs, AutBase::ProductTranslMap* pTranslMap);

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> RemoveUnreachableStates(const ExplicitTreeAut<SymbolType>& aut,
		AutBase::StateToStateMap* pTranslMap) {

		std::unordered_set<AutBase::StateType>
			reachableStates(aut.finalStates_),
			newStates(reachableStates);

		while (newStates.size()) {

			std::vector<const typename ExplicitTreeAut<SymbolType>::TransitionCluster*> clusters;

			for (auto state : newStates) {

				auto cluster = ExplicitTreeAut<SymbolType>::genericLookup(*aut.transitions_, state);

				if (cluster)
					clusters.push_back(cluster);

			}

			newStates.clear();

			for (auto cluster : clusters) {

				assert(cluster);

				for (auto symbolStateTupleSetPtr : *cluster) {

					assert(symbolStateTupleSetPtr.second);

					for (auto stateTuple : *symbolStateTupleSetPtr.second) {

						for (auto state : stateTuple) {

							if (reachableStates.insert(state).second)
								newStates.insert(state);

						}
						
					}

				}

			}

		}

		if (pTranslMap) {
			
			for (auto state : reachableStates)
				pTranslMap->insert(std::make_pair(state, state));

		}

		if (reachableStates.size() == aut.transitions_.size())
			return aut;

		ExplicitTreeAut<SymbolType> result;

		result.finalStates_ = aut.finalStates_;
		result.transitions_ = ExplicitTreeAut<SymbolType>::StateToTransitionClusterMapPtr(
				new typename ExplicitTreeAut<SymbolType>::StateToTransitionClusterMap()
		);

		for (auto state : reachableStates) {

			auto iter = aut.transitions_.find(state);

			if (iter != aut.transitions_.end())
				result.transitions_.insert(std::make_pair(state, iter->second));

		}

		return result;

	}

	template <class SymbolType>
	bool CheckInclusion(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		return CheckDownwardTreeInclusion(smaller, bigger);

	}

}

#endif
