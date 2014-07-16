/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of template RemoveUnreachableStates() on explicit tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_UNREACH_HH_
#define _VATA_EXPLICIT_TREE_UNREACH_HH_

// VATA headers
#include <vata/explicit_tree_aut.hh>
#include <vata/util/antichain1c.hh>

template <
	class Rel,
	class Index
>
VATA::ExplicitTreeAutCore VATA::ExplicitTreeAutCore::RemoveUnreachableStates(
	const Rel&                                 rel,
	const Index&                               index) const
{
	typedef ExplicitTreeAutCore::StateToTransitionClusterMap
		StateToTransitionClusterMap;
	typedef ExplicitTreeAutCore::StateToTransitionClusterMapPtr
		StateToTransitionClusterMapPtr;
	typedef ExplicitTreeAutCore::TransitionCluster TransitionCluster;
	typedef ExplicitTreeAutCore::TransitionClusterPtr TransitionClusterPtr;
	typedef ExplicitTreeAutCore::TuplePtrSet TuplePtrSet;
	typedef ExplicitTreeAutCore::TuplePtrSetPtr TuplePtrSetPtr;
	typedef ExplicitTreeAutCore::TuplePtr TuplePtr;
	typedef ExplicitTreeAutCore::StateTuple StateTuple;

	struct TupleCmp
	{
		const Rel& rel_;
		const Index& index_;

		TupleCmp(
			const Rel&      rel,
			const Index&    index) :
			rel_(rel),
			index_(index)
		{ }

		bool operator()(
			const TuplePtr&    lhsPtr,
			const TuplePtr&    rhsPtr) const
		{
			assert(nullptr != lhsPtr);
			assert(nullptr != rhsPtr);

			const StateTuple& lhs = *lhsPtr;
			const StateTuple& rhs = *rhsPtr;

			assert(lhs.size() == rhs.size());

			for (size_t i = 0; i < lhs.size(); ++i)
			{
				if (!rel_.get(index_[lhs[i]], index_[rhs[i]]))
				{
					return false;
				}
			}

			return true;
		}
	};

	Util::Antichain1C<AutBase::StateType> finalStates;

	typename Rel::IndexType ind, inv;

	rel.buildIndex(ind, inv);

	for (const StateType& state : this->GetFinalStates())
	{
		if (finalStates.contains(ind[state]))
		{
			continue;
		}

		finalStates.refine(inv[state]);
		finalStates.insert(state);
	}

	std::unordered_set<AutBase::StateType> reachableStates(
		finalStates.data().begin(), finalStates.data().end()
	);

	std::vector<AutBase::StateType> newStates(reachableStates.begin(), reachableStates.end());

	Util::SequentialAntichain1C<TuplePtr> tuples;

	auto newTransitions = StateToTransitionClusterMapPtr(new StateToTransitionClusterMap());

	bool transitionsModified = false;

	while (!newStates.empty())
	{
		auto state = newStates.back();

		newStates.pop_back();

		auto stateClusterIter = transitions_->find(state);

		if (stateClusterIter == transitions_->end())
		{
			continue;
		}

		bool clusterModified = false;

		auto transitionCluster = TransitionClusterPtr(new TransitionCluster());

		for (auto& symbolStateTupleSetPtr : *stateClusterIter->second)
		{
			assert(symbolStateTupleSetPtr.second);

			tuples.clear();

			for (auto& stateTuple : *symbolStateTupleSetPtr.second)
			{
				assert(stateTuple);

				tuples.insert(stateTuple, TupleCmp(rel, index));
			}

			for (auto& stateTuple : tuples.data())
			{
				for (const StateType& state : *stateTuple)
				{
					if (reachableStates.insert(state).second)
					{
						newStates.push_back(state);
					}
				}
			}

			if (tuples.data().size() == symbolStateTupleSetPtr.second->size())
			{
				transitionCluster->insert(symbolStateTupleSetPtr);

				continue;
			}

			clusterModified = true;

			auto tupleSet = TuplePtrSetPtr(new TuplePtrSet());

			for (auto& stateTuple : tuples.data())
			{
				tupleSet->insert(stateTuple);
			}

			transitionCluster->insert(std::make_pair(symbolStateTupleSetPtr.first, tupleSet));
		}

		if (!clusterModified)
		{
			newTransitions->insert(std::make_pair(state, stateClusterIter->second));
			continue;
		}

		transitionsModified = true;

		newTransitions->insert(std::make_pair(state, transitionCluster));
	}

	if (!transitionsModified && (reachableStates.size() == transitions_->size()) &&
		(finalStates.data().size() == finalStates_.size()))
	{
		return *this;
	}

	ExplicitTreeAutCore result(cache_);

	result.finalStates_.insert(finalStates.data().begin(), finalStates.data().end());

	if (!transitionsModified && (reachableStates.size() == transitions_->size()))
	{
		result.transitions_ = transitions_;

		return result;
	}

	result.transitions_ = newTransitions;

	return result;
}

#endif
