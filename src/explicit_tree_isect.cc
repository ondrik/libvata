/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of Intersection() on explicit tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

// Standard library headers
#include <vector>


#include "explicit_tree_aut_core.hh"

using VATA::ExplicitTreeAutCore;

ExplicitTreeAutCore ExplicitTreeAutCore::Intersection(
	const ExplicitTreeAutCore&           lhs,
	const ExplicitTreeAutCore&           rhs,
	VATA::AutBase::ProductTranslMap*     pTranslMap)
{
	VATA::AutBase::ProductTranslMap translMap;

	if (nullptr == pTranslMap)
	{
		pTranslMap = &translMap;
	}

	ExplicitTreeAutCore res(lhs.cache_);

	std::vector<const VATA::AutBase::ProductTranslMap::value_type*> stack;

	for (const StateType& s : lhs.finalStates_)
	{
		for (const StateType& t : rhs.finalStates_)
		{
			auto u = pTranslMap->insert(
				std::make_pair(std::make_pair(s, t), pTranslMap->size())
			).first;

			res.SetStateFinal(u->second);

			stack.push_back(&*u);
		}
	}

	auto transitions = res.transitions_;

	while (!stack.empty())
	{
		auto p = stack.back();

		stack.pop_back();

		auto leftCluster = ExplicitTreeAutCore::genericLookup(
			*lhs.transitions_, p->first.first);

		if (!leftCluster)
		{
			continue;
		}

		auto rightCluster = ExplicitTreeAutCore::genericLookup(
			*rhs.transitions_, p->first.second);

		if (!rightCluster)
		{
			continue;
		}

		assert(leftCluster);

		ExplicitTreeAutCore::TransitionClusterPtr cluster(nullptr);

		for (auto& leftSymbolStateTupleSetPtr : *leftCluster)
		{
			auto rightTupleSet =
				ExplicitTreeAutCore::genericLookup(
					*rightCluster, leftSymbolStateTupleSetPtr.first);

			if (!rightTupleSet)
			{
				continue;
			}

			if (!cluster)
			{
				cluster = transitions->uniqueCluster(p->second);
			}

			auto tuplePtrSet = cluster->uniqueTuplePtrSet(leftSymbolStateTupleSetPtr.first);

			for (auto& leftTuplePtr : *leftSymbolStateTupleSetPtr.second)
			{
				for (auto& rightTuplePtr : *rightTupleSet)
				{
					assert(leftTuplePtr->size() == rightTuplePtr->size());

					ExplicitTreeAutCore::StateTuple children;

					for (size_t i = 0; i < leftTuplePtr->size(); ++i)
					{
						auto u = pTranslMap->insert(
							std::make_pair(
								std::make_pair((*leftTuplePtr)[i], (*rightTuplePtr)[i]),
								pTranslMap->size()
							)
						);

						if (u.second)
						{
							stack.push_back(&*u.first);
						}

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
