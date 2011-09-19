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
#include <vata/down_tree_incl_fctor.hh>
#include <vata/down_tree_incl_nouseless_fctor.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/tree_incl_down.hh>

#include <unordered_set>

namespace VATA {

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> Union(const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr) {

		AutBase::StateToStateMap translMapLhs;
		AutBase::StateToStateMap translMapRhs;

		if (!pTranslMapLhs)
			pTranslMapLhs = &translMapLhs;

		if (!pTranslMapRhs)
			pTranslMapRhs = &translMapRhs;

		ExplicitTreeAut<SymbolType> res(lhs.cache_);

		lhs.ReindexStates(res, pTranslMapLhs, 0);
		rhs.ReindexStates(res, pTranslMapRhs, pTranslMapLhs->size());

		return res;

	}

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> Intersection(
		const ExplicitTreeAut<SymbolType>& /* lhs */,
		const ExplicitTreeAut<SymbolType>& /* rhs */,
		AutBase::ProductTranslMap* /* pTranslMap */ = nullptr)
	{
		throw std::runtime_error("Unimplemented");
	}

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> RemoveUselessStates(
		const ExplicitTreeAut<SymbolType>& /* aut */,
		AutBase::StateToStateMap* /* pTranslMap */ = nullptr)
	{
		throw std::runtime_error("Unimplemented");
	}

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> RemoveUnreachableStates(
		const ExplicitTreeAut<SymbolType>& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr) {

		typedef ExplicitTreeAut<SymbolType> ExplicitTA;
		typedef typename ExplicitTA::StateToTransitionClusterMapPtr
			StateToTransitionClusterMapPtr;

		std::unordered_set<AutBase::StateType> reachableStates(aut.GetFinalStates());
		std::vector<AutBase::StateType> newStates(reachableStates.begin(), reachableStates.end());

		while (!newStates.empty()) {

			std::vector<const typename ExplicitTA::TransitionCluster*> clusters;

			for (auto state : newStates) {

				auto cluster = ExplicitTA::genericLookup(*aut.transitions_, state);

				if (cluster)
					clusters.push_back(cluster);

			}

			newStates.clear();

			for (auto cluster : clusters) {

				assert(cluster);

				for (auto symbolStateTupleSetPtr : *cluster) {

					assert(symbolStateTupleSetPtr.second);

					for (auto stateTuple : *symbolStateTupleSetPtr.second) {

						assert(stateTuple);

						for (auto state : *stateTuple) {

							if (reachableStates.insert(state).second)
								newStates.push_back(state);

						}

					}

				}

			}

		}

		if (pTranslMap) {

			for (auto state : reachableStates)
				pTranslMap->insert(std::make_pair(state, state));

		}

		if (reachableStates.size() == aut.transitions_->size())
			return aut;

		ExplicitTA result(aut.cache_);

		result.finalStates_ = aut.finalStates_;
		result.transitions_ = StateToTransitionClusterMapPtr(
				new typename ExplicitTA::StateToTransitionClusterMap()
		);

		for (auto state : reachableStates) {

			auto iter = aut.transitions_->find(state);

			if (iter != aut.transitions_->end())
				result.transitions_->insert(std::make_pair(state, iter->second));

		}

		return result;

	}

	template <class SymbolType>
	bool CheckInclusion(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		return CheckDownwardTreeInclusion<ExplicitTreeAut<SymbolType>,
			VATA::DownwardInclusionFunctor>(smaller, bigger);

	}

	template <class SymbolType>
	bool CheckInclusionNoUseless(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		return CheckDownwardTreeInclusion<ExplicitTreeAut<SymbolType>,
			VATA::DownwardInclusionNoUselessFunctor>(smaller, bigger);

	}

}

#endif
