/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for function checking tree automata language inclusion
 *    upwards.
 *
 *****************************************************************************/

#ifndef _VATA_TREE_INCL_UP_HH_
#define _VATA_TREE_INCL_UP_HH_

// VATA headers
#include <vata/vata.hh>

namespace VATA
{
	template <class Aut, template <class> class UpwardInclFctor, class Rel>
	bool CheckUpwardTreeInclusion(const Aut& smaller, const Aut& bigger,
		const Rel& preorder);
}

/**
 * @brief  Checks language inclusion on tree automata upwards
 *
 * This is a general method for tree automata to check language inclusion
 * upward.
 *
 * @todo  Write this documentation
 */
template <class Aut, template <class> class UpwardInclFctor, class Rel>
bool VATA::CheckUpwardTreeInclusion(const Aut& smaller, const Aut& bigger,
	const Rel& /* preorder */)
{
	typedef UpwardInclFctor<Aut> InclFctor;
	typedef typename InclFctor::StateType StateType;
	typedef typename InclFctor::StateSet StateSet;
	typedef typename InclFctor::StateTuple StateTuple;
	typedef typename InclFctor::StateTupleSet StateTupleSet;

	typedef typename std::vector<std::set<StateType>> StateSetTuple;

	typedef typename InclFctor::AntichainType AntichainType;

	class ChoiceFunctionGenerator
	{
	private:  // data types

		typedef typename StateSetTuple::value_type InternalContainerType;
		typedef std::vector<typename InternalContainerType::const_iterator>
			ChoiceFunctionType;
		typedef StateSetTuple DomainType;
		typedef std::vector<StateType> ResultType;


	private:  // data members

		const DomainType& domain_;
		ChoiceFunctionType currentCf_;
		ResultType currentResult_;

		char lastCnt_;

	public:   // methods

		ChoiceFunctionGenerator(const DomainType& domain) :
			domain_(domain),
			currentCf_(),
			currentResult_(),
			lastCnt_(2)   // 2 because the initial GetNext() decrements it
		{
			// Assertions
			assert(!domain_.empty());

			for (auto it = domain_.begin(); it != domain_.end(); ++it)
			{
				assert(!it->empty());
				currentCf_.push_back(--(it->end()));
				currentResult_.push_back(*(it->begin()));
			}

			// Assertions
			assert(currentCf_.size() == domain_.size());
			assert(currentResult_.size() == domain_.size());
		}

		inline const ResultType& GetNext()
		{
			// Assertions
			assert(!IsLast());

			// move to the next choice function
			size_t index = 0;
			while (++(currentCf_[index]) == domain_[index].end()) {
				currentCf_[index] = domain_[index].begin(); // reset this counter
				currentResult_[index] = *(currentCf_[index]);
				++index;               // move to the next counter

				if (index == currentCf_.size()) {
					// if we drop out from the n-tuple
					--lastCnt_;
					index = 0;
					break;
				}
				assert(currentCf_[index] != domain_[index].end());
			}

			currentResult_[index] = *(currentCf_[index]);

			return currentResult_;
		}

		inline bool IsLast() const
		{
			return lastCnt_ == 0;
		}
	};

	AntichainType workset;
	AntichainType antichain;
	InclFctor upFctor(smaller, bigger, workset, antichain);

	StateTuple tuple;
	StateTupleSet tupleSet = {tuple};
	Aut::ForeachUpSymbolFromTupleAndTupleSetDo(smaller, bigger, tuple, tupleSet,
		upFctor);

	if (!upFctor.InclusionHolds())
	{	// in case a counterexample was found
		return false;
	}

	StateType procState;
	StateSet procSet;
	while (workset.get(procState, procSet))
	{
		for (auto tupleBddPair : smaller.GetTransTable())
		{	// for each tuple in the smaller aut
			const StateTuple& tuple = tupleBddPair.first;
			if (std::find(tuple.begin(), tuple.end(), procState) == tuple.end())
			{	// if the tuple does not contain the processed state
				continue;
			}

			bool allElementsInAntichain = true;
			for (size_t index = 0; index < tuple.size(); ++index)
			{
				if (tuple[index] == procState)
				{	// skip when it is the processed state
					continue;
				}

				if (antichain.lookup(tuple[index]) == nullptr)
				{
					allElementsInAntichain = false;
					break;
				}
			}

			if (!allElementsInAntichain)
			{	// if the tuple is not reachable yet
				continue;
			}

			// create a tuple of sets of states
			StateSetTuple stateSetTuple(tuple.size());
			bool isEmpty = false;

			for (size_t index = 0; index < tuple.size(); ++index)
			{
				if (tuple[index] == procState)
				{	// special processing for the processed state
					stateSetTuple[index].insert(procSet.begin(), procSet.end());
				}
				else
				{
					const typename AntichainType::TList* keyList;
					if ((keyList = antichain.lookup(tuple[index])) != nullptr)
					{
						assert(keyList->begin() != keyList->end());

						for (auto& listElem : *(keyList))
						{
							stateSetTuple[index].insert(listElem.begin(), listElem.end());
						}
					}
				}

				if (stateSetTuple[index].empty())
				{
					isEmpty = true;
					break;
				}
			}

			StateTupleSet tupleSet;

			if (!isEmpty)
			{	// if there are some tuples

				// generate all tuples
				ChoiceFunctionGenerator cfGen(stateSetTuple);
				while (!cfGen.IsLast())
				{	// for each choice function
					tupleSet.insert(cfGen.GetNext());
				}
			}

			Aut::ForeachUpSymbolFromTupleAndTupleSetDo(smaller, bigger, tuple,
				tupleSet, upFctor);
		}

		if (!upFctor.InclusionHolds())
		{	// in case a counterexample was found
			return false;
		}
	}

	return true;
}

#endif
