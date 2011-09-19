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

		VATA_LOGGER_INFO("Checking " + Convert::ToString(procPair));

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

				VATA_LOGGER_INFO("Processing tuple " + Convert::ToString(tuple));
				VATA_LOGGER_INFO("Antichain: " + Convert::ToString(antichain));
				bool allElementsInAntichain = true;
				for (size_t index = 0; index < arity; ++index)
				{
					if (index == tupleIndexPair.second)
					{	// skip when it is the processed state
						assert(tuple[index] == procState);
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

				for (size_t index = 0; index < arity; ++index)
				{
					if (index == tupleIndexPair.second)
					{	// special processing for the processed state
						stateSetTuple[index] = procSet;
					}
					else
					{
						auto keyRange = antichain.equal_range(tuple[index]);
						// TODO : continue here
					if ( == antichain.end())
					{
						allElementsInAntichain = false;
						break;
					}

					}
				}



				// TODO: do something else
				StateTupleSet tupleSet;


				Aut::ForeachUpSymbolFromTupleAndTupleSetDo(smaller, bigger, tuple, tupleSet, upFctor);
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
