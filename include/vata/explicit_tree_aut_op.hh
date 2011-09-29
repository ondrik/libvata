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
#include <list>
#include <unordered_set>

#include <vata/vata.hh>
#include <vata/down_tree_incl_fctor.hh>
#include <vata/down_tree_incl_nouseless_fctor.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/explicit_tree_incl_up.hh>
#include <vata/tree_incl_down.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/convert.hh>


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
		const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr) {

		typedef ExplicitTreeAut<SymbolType> ExplicitTA;

		AutBase::ProductTranslMap translMap;

		if (!pTranslMap)
			pTranslMap = &translMap;

		ExplicitTreeAut<SymbolType> res(lhs.cache_);

		std::vector<const AutBase::ProductTranslMap::value_type*> stack;

		for (auto& s : lhs.finalStates_) {

			for (auto& t : rhs.finalStates_) {

				auto u = pTranslMap->insert(
					std::make_pair(std::make_pair(s, t), pTranslMap->size())
				).first;

				res.SetStateFinal(u->second);

				stack.push_back(&*u);

			}

		}

		while (!stack.empty()) {

			auto p = stack.back();

			stack.pop_back();

			auto leftCluster = ExplicitTA::genericLookup(*lhs.transitions_, p->first.first);

			if (!leftCluster)
				continue;

			auto rightCluster = ExplicitTA::genericLookup(*rhs.transitions_, p->first.second);

			if (!rightCluster)
				continue;

			assert(leftCluster);

			for (auto& leftSymbolStateTupleSetPtr : *leftCluster) {

				auto rightTupleSet =
					ExplicitTA::genericLookup(*rightCluster, leftSymbolStateTupleSetPtr.first);

				if (!rightTupleSet)
					continue;

				for (auto& leftTuplePtr : *leftSymbolStateTupleSetPtr.second) {

					for (auto& rightTuplePtr : *rightTupleSet) {

						assert(leftTuplePtr->size() == rightTuplePtr->size());

						typename ExplicitTA::StateTuple children;

						for (size_t i = 0; i < leftTuplePtr->size(); ++i) {

							auto u = pTranslMap->insert(
								std::make_pair(
									std::make_pair((*leftTuplePtr)[i], (*rightTuplePtr)[i]),
									pTranslMap->size()
								)
							);

							if (u.second)
								stack.push_back(&*u.first);

							children.push_back(u.first->second);

						}

						res.AddTransition(children, leftSymbolStateTupleSetPtr.first, p->second);

					}					
					
				}

			}

		}

		return res;

	}

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> RemoveUselessStates(
		const ExplicitTreeAut<SymbolType>& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr) {

		typedef ExplicitTreeAut<SymbolType> ExplicitTA;

		typedef typename ExplicitTA::StateType StateType;
		typedef typename ExplicitTA::TuplePtr TuplePtr;

		typedef std::tuple<std::set<StateType>, TuplePtr, const SymbolType*, const StateType*> TransitionInfo;

		std::unordered_map<StateType, std::vector<TransitionInfo*>> stateMap;

		std::list<TransitionInfo> transitionInfoBuffer;

		std::unordered_set<StateType> reachableStates;

		std::vector<StateType> newStates;

		for (auto& stateClusterPair : *aut.transitions_) {

			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);

					transitionInfoBuffer.push_back(
						TransitionInfo(
							std::set<StateType>(), tuple, &symbolTupleSetPair.first, &stateClusterPair.first
						)
					);

					if (tuple->empty()) {

						if (reachableStates.insert(stateClusterPair.first).second)
							newStates.push_back(stateClusterPair.first);

						continue;

					}

					for (auto& s : *tuple) {

						if (std::get<0>(transitionInfoBuffer.back()).insert(s).second) {

							stateMap.insert(
								std::make_pair(s, std::vector<TransitionInfo*>())
							).first->second.push_back(&transitionInfoBuffer.back());

						}

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

				std::get<0>(*info).erase(i->first);

				if (!std::get<0>(*info).empty())
					continue;

				assert(std::get<3>(*info));

				if (reachableStates.insert(*std::get<3>(*info)).second)
					newStates.push_back(*std::get<3>(*info));

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

		for (auto& info : transitionInfoBuffer) {

			if (std::get<0>(info).empty()) {

				assert(std::get<2>(info));
				assert(std::get<3>(info));

				result.internalAddTransition(std::get<1>(info), *std::get<2>(info), *std::get<3>(info));

			}

		}

		return result;

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

	template <class SymbolType>
	bool CheckInclusion(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		return CheckInclusionUpwardNoGarbage(
			RemoveUnreachableStates(RemoveUselessStates(smaller)),
			RemoveUnreachableStates(RemoveUselessStates(bigger))
		);

	}

	template <class SymbolType>
	bool CheckInclusionDownward(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		return CheckDownwardTreeInclusion<ExplicitTreeAut<SymbolType>,
			VATA::DownwardInclusionFunctor>(smaller, bigger);

	}

	template <class SymbolType>
	bool CheckInclusionNoUseless(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		return CheckInclusionUpwardNoGarbage(smaller, bigger);

	}

	template <class SymbolType>
	bool CheckInclusionUpwardNoGarbage(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		AutBase::StateToStateMap stateMap;

		Explicit::TupleCache tupleCache;

		ExplicitTreeAut<SymbolType> a(tupleCache), b(tupleCache);
		
		smaller.ReindexStates(a, &stateMap);

		size_t size = stateMap.size();

		stateMap.clear();

		bigger.ReindexStates(b, &stateMap, size);

		return ExplicitUpwardInclusion::Check(a, b, Util::Identity(size + stateMap.size()));

	}

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeSimulation(
		const ExplicitTreeAut<SymbolType>& aut)
	{
		assert(&aut != nullptr);

		throw std::runtime_error("Unimplemented");
	}

}

#endif
