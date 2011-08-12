/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for function checking tree automata language inclusion
 *    downwards.
 *
 *****************************************************************************/

#ifndef _TREE_INCL_DOWN_HH_
#define _TREE_INCL_DOWN_HH_

// VATA headers
#include <vata/vata.hh>

namespace VATA
{
	template <typename Aut>
	bool CheckDownwardTreeInclusion(const Aut& smaller, const Aut& bigger);
}


/**
 * @brief  Checks language inclusion on tree automata downwards
 *
 * This is a general method for tree automata to check language inclusion
 * downward.
 *
 * @todo  Write this documentation
 */
template <typename Aut>
bool VATA::CheckDownwardTreeInclusion(const Aut& smaller, const Aut& bigger)
{
	typedef typename Aut::StateTupleSet StateTupleSet;

	class DownwardAntichainFunctor
	{

	public:

		void operator()(const StateTupleSet& lhs, const StateTupleSet& rhs)
		{
			assert(&lhs != nullptr);
			assert(&rhs != nullptr);
			assert(false);

		}
	};

	DownwardAntichainFunctor downFctor;

	for (auto itSmSt = smaller.GetFinalStates().cbegin();
		itSmSt != smaller.GetFinalStates().cend(); ++itSmSt)
	{	// for each final state of the smaller automaton
		Aut::ForeachDownSymbolFromStateAndStateSetDo(smaller, bigger,
			*itSmSt, bigger.GetFinalStates(), downFctor);
	}

	assert(false);
}

#endif
