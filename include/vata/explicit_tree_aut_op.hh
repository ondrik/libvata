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
#include <vata/explicit_tree_aut.hh>
#include <vata/explicit_tree_incl_up.hh>
#include <vata/tree_incl_down.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/convert.hh>

// Standard library headers
#include <list>
#include <unordered_set>


namespace VATA {

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> Union(const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr) {

		typedef AutBase::StateType StateType;
		typedef AutBase::StateToStateTranslator StateToStateTranslator;

		AutBase::StateToStateMap translMapLhs;
		AutBase::StateToStateMap translMapRhs;

		if (!pTranslMapLhs)
			pTranslMapLhs = &translMapLhs;

		if (!pTranslMapRhs)
			pTranslMapRhs = &translMapRhs;

		StateType stateCnt = 0;
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
		StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);

		ExplicitTreeAut<SymbolType> res(lhs.cache_);

		lhs.ReindexStates(res, stateTransLhs);
		rhs.ReindexStates(res, stateTransRhs);

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
	bool CheckUpwardInclusion(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		return CheckUpwardInclusionNoGarbage(
			RemoveUnreachableStates(RemoveUselessStates(smaller)),
			RemoveUnreachableStates(RemoveUselessStates(bigger))
		);

	}

	template <class SymbolType>
	bool CheckUpwardInclusionWithSimulation(
		const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {
		assert(&smaller != nullptr);
		assert(&bigger != nullptr);
		throw std::runtime_error("Unimplemented");
	}

	template <class SymbolType>
	bool CheckDownwardInclusion(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		return CheckDownwardInclusionWithoutUseless(
			RemoveUnreachableStates(RemoveUselessStates(smaller)),
			RemoveUnreachableStates(RemoveUselessStates(bigger))
		);

	}

	template <class SymbolType>
	bool CheckDownwardInclusionWithSimulation(
		const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {
		assert(&smaller != nullptr);
		assert(&bigger != nullptr);
		throw std::runtime_error("Unimplemented");
	}

	template <class SymbolType>
	bool CheckDownwardInclusionWithoutUseless(
		const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		typedef AutBase::StateType StateType;
		typedef AutBase::StateToStateMap StateToStateMap;
		typedef AutBase::StateToStateTranslator StateToStateTranslator;
		typedef ExplicitTreeAut<SymbolType> ExplAut;

		StateType stateCnt = 0;
		StateToStateMap stateMap;
		StateToStateTranslator stateTrans(stateMap,
			[&stateCnt](const StateType&){return stateCnt++;});

		ExplAut newSmaller;
		smaller.ReindexStates(newSmaller, stateTrans);

		ExplAut newBigger;
		bigger.ReindexStates(newBigger, stateTrans);

		VATA::Util::Identity ident(stateCnt);
		return CheckDownwardTreeInclusion<ExplAut,
			VATA::DownwardInclusionFunctor>(newSmaller, newBigger, ident);

	}

	template <class SymbolType>
	bool CheckUpwardInclusionNoGarbage(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		typedef AutBase::StateType StateType;
		typedef AutBase::StateToStateMap StateToStateMap;
		typedef AutBase::StateToStateTranslator StateToStateTranslator;

		StateToStateMap stateMap;

		StateType stateCnt = 0;
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTrans(stateMap, translFunc);

		Explicit::TupleCache tupleCache;

		ExplicitTreeAut<SymbolType> a(tupleCache), b(tupleCache);

		smaller.ReindexStates(a, stateTrans);
		stateMap.clear();

		bigger.ReindexStates(b, stateTrans);

		return ExplicitUpwardInclusion::Check(a, b, Util::Identity(stateCnt));

	}

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const ExplicitTreeAut<SymbolType>& aut)
	{
		assert(&aut != nullptr);

		throw std::runtime_error("Unimplemented");
	}

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const ExplicitTreeAut<SymbolType>& aut)
	{
		assert(&aut != nullptr);

		throw std::runtime_error("Unimplemented");
	}

}

#endif
