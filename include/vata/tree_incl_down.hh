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

#ifndef _VATA_TREE_INCL_DOWN_HH_
#define _VATA_TREE_INCL_DOWN_HH_

// VATA headers
#include <vata/vata.hh>

namespace VATA
{
	template <class Aut, template <class> class DownwardInclFctor>
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
template <class Aut, template <class> class DownwardInclFctor>
bool VATA::CheckDownwardTreeInclusion(const Aut& smaller, const Aut& bigger)
{
	typedef DownwardInclFctor<Aut> InclFctor;

	typedef typename Aut::StateType StateType;
	typedef typename InclFctor::StateSet StateSet;

	typedef typename InclFctor::WorkSetType WorkSetType;
	typedef typename InclFctor::InclusionCache InclusionCache;
	typedef typename InclFctor::StateStateSetPairToBoolMap
		StateStateSetPairToBoolMap;

	WorkSetType workset;
	InclusionCache nonIncl;
	StateStateSetPairToBoolMap nonInclHT;

	InclFctor downFctor(smaller, bigger, workset, nonIncl, nonInclHT);

	StateSet finalStatesBigger(bigger.GetFinalStates().begin(),
		bigger.GetFinalStates().end());

	for (const StateType& smSt : smaller.GetFinalStates())
	{	// for each final state of the smaller automaton
		downFctor.Reset();
		Aut::ForeachDownSymbolFromStateAndStateSetDo(smaller, bigger,
			smSt, finalStatesBigger, downFctor);

		if (!downFctor.InclusionHolds())
		{	// in case inclusion does not hold for some LHS state
			return false;
		}
	}

	// inclusion holds for all LHS states
	return true;
}

#endif
