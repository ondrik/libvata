/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for Intersection() on explicit tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_ISECT_HH_
#define _VATA_EXPLICIT_TREE_ISECT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_tree_aut.hh>

// Standard library headers
#include <vector>

namespace VATA {

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> Intersection(
		const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr);

}

template <class SymbolType>
VATA::ExplicitTreeAut<SymbolType> VATA::Intersection(
	const VATA::ExplicitTreeAut<SymbolType>& lhs,
	const VATA::ExplicitTreeAut<SymbolType>& rhs,
	VATA::AutBase::ProductTranslMap* pTranslMap = nullptr) {

	typedef VATA::ExplicitTreeAut<SymbolType> ExplicitTA;

	VATA::AutBase::ProductTranslMap translMap;

	if (!pTranslMap)
		pTranslMap = &translMap;

	ExplicitTA res(lhs.cache_);

	std::vector<const VATA::AutBase::ProductTranslMap::value_type*> stack;

	for (auto& s : lhs.finalStates_) {

		for (auto& t : rhs.finalStates_) {

			auto u = pTranslMap->insert(
				std::make_pair(std::make_pair(s, t), pTranslMap->size())
			).first;

			res.SetStateFinal(u->second);

			stack.push_back(&*u);

		}

	}

	auto transitions = res.transitions_;

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

		typename ExplicitTreeAut<SymbolType>::TransitionClusterPtr cluster(nullptr);

		for (auto& leftSymbolStateTupleSetPtr : *leftCluster) {

			auto rightTupleSet =
				ExplicitTA::genericLookup(*rightCluster, leftSymbolStateTupleSetPtr.first);

			if (!rightTupleSet)
				continue;

			typename ExplicitTreeAut<SymbolType>::TuplePtrSetPtr tuplePtrSet(nullptr);

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

					if (!tuplePtrSet) {

						if (!cluster)
							cluster = transitions->uniqueCluster(p->second);

						tuplePtrSet = cluster->uniqueTuplePtrSet(leftSymbolStateTupleSetPtr.first);

					}

//					res.AddTransition(children, leftSymbolStateTupleSetPtr.first, p->second);
					tuplePtrSet->insert(res.tupleLookup(children));

				}

			}

		}

	}

	return res;

}

#endif
