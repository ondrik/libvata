/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
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
	template <class Aut, template <class> class UpwardInclFctor>
	bool CheckUpwardTreeInclusion(const Aut& smaller, const Aut& bigger);
}

/**
 * @brief  Checks language inclusion on tree automata upwards
 *
 * This is a general method for tree automata to check language inclusion
 * upward.
 *
 * @todo  Write this documentation
 */
template <class Aut, template <class> class UpwardInclFctor>
bool VATA::CheckUpwardTreeInclusion(const Aut& smaller, const Aut& bigger)
{
	typedef UpwardInclFctor<Aut> InclFctor;
	typedef typename InclFctor::StateType StateType;
	typedef typename InclFctor::StateSet StateSet;
	typedef typename InclFctor::StateTuple StateTuple;
	typedef typename InclFctor::StateTupleSet StateTupleSet;

	typedef typename std::vector<std::set<StateType>> StateSetTuple;

	typedef typename InclFctor::AntichainType AntichainType;

	typedef typename Aut::IndexValueArray IndexValueArray;

	typedef VATA::Util::Convert Convert;

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

	while (!workset.empty())
	{
		auto procPair = *(workset.begin());
		const StateType& procState = procPair.first;
		const StateSet& procSet = procPair.second;
		workset.erase(workset.begin());

		//VATA_LOGGER_INFO("Processing " + Convert::ToString(procPair));
		//VATA_LOGGER_INFO("Antichain " + Convert::ToString(antichain));

		// get all tuples with the smaller state from the smaller automaton
		IndexValueArray smallerTuples = smaller.GetTuples().GetItemsWith(
			procState, smaller.GetStates());

		for (size_t arity = 0; arity < smallerTuples.size(); ++arity)
		{	// for each arity of left-hand side in the smaller aut
			for (auto tupleIndexPair : smallerTuples[arity])
			{
				// Assertions
				assert(tupleIndexPair.first.size() == arity);

				const StateTuple& tuple = tupleIndexPair.first;

				bool allElementsInAntichain = true;
				for (size_t index = 0; index < arity; ++index)
				{
					if (tuple[index] == procState)
					{	// skip when it is the processed state
						continue;
					}

					if (antichain.find(tuple[index]) == antichain.end())
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
				StateSetTuple stateSetTuple(arity);
				bool isEmpty = false;

				for (size_t index = 0; index < arity; ++index)
				{
					if (tuple[index] == procState)
					{	// special processing for the processed state
						stateSetTuple[index].insert(procSet.begin(), procSet.end());
					}
					else
					{
						auto keyRange = antichain.equal_range(tuple[index]);
						assert(keyRange.first != antichain.end());

						for (; keyRange.first != keyRange.second; ++(keyRange.first))
						{
							stateSetTuple[index].insert((keyRange.first)->second.begin(),
								(keyRange.first)->second.end());
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
		}

		if (!upFctor.InclusionHolds())
		{	// in case a counterexample was found
			return false;
		}
	}

	return true;
}

#endif