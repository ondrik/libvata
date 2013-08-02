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
#include <vata/ta_expl/explicit_tree_aut.hh>

// Standard library headers
#include <vector>

namespace VATA {

	ExplicitTreeAut Intersection(
		const ExplicitTreeAut& lhs,
		const ExplicitTreeAut& rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr);

}

VATA::ExplicitTreeAut VATA::Intersection(
	const VATA::ExplicitTreeAut& lhs,
	const VATA::ExplicitTreeAut& rhs,
	VATA::AutBase::ProductTranslMap* pTranslMap)
{
	VATA::AutBase::ProductTranslMap translMap;

	if (!pTranslMap)
		pTranslMap = &translMap;

	ExplicitTreeAut res(lhs.cache_);

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

		auto leftCluster = ExplicitTreeAut::genericLookup(*lhs.transitions_, p->first.first);

		if (!leftCluster)
			continue;

		auto rightCluster = ExplicitTreeAut::genericLookup(*rhs.transitions_, p->first.second);

		if (!rightCluster)
			continue;

		assert(leftCluster);

		ExplicitTreeAut::TransitionClusterPtr cluster(nullptr);

		for (auto& leftSymbolStateTupleSetPtr : *leftCluster) {

			auto rightTupleSet =
				ExplicitTreeAut::genericLookup(*rightCluster, leftSymbolStateTupleSetPtr.first);

			if (!rightTupleSet)
				continue;

			if (!cluster)
				cluster = transitions->uniqueCluster(p->second);

			auto tuplePtrSet = cluster->uniqueTuplePtrSet(leftSymbolStateTupleSetPtr.first);

			for (auto& leftTuplePtr : *leftSymbolStateTupleSetPtr.second) {

				for (auto& rightTuplePtr : *rightTupleSet) {

					assert(leftTuplePtr->size() == rightTuplePtr->size());

					ExplicitTreeAut::StateTuple children;

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

//					res.AddTransition(children, leftSymbolStateTupleSetPtr.first, p->second);
					tuplePtrSet->insert(res.tupleLookup(children));

				}

			}

		}

	}

	return res;

}

#endif
