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
	template
	<
		class Aut,
		template <class, class> class DownwardInclFctor,
		class Rel
	>
	bool CheckDownwardTreeInclusion(const Aut& smaller, const Aut& bigger,
		const Rel& preorder);
}

/**
 * @brief  Checks language inclusion on tree automata downwards
 *
 * This is a general method for tree automata to check language inclusion
 * downward.
 *
 * @todo  Write this documentation
 */
template
<
	class Aut,
	template <class, class> class DownwardInclFctor,
	class Rel
>
bool VATA::CheckDownwardTreeInclusion(const Aut& smaller, const Aut& bigger,
	const Rel& preorder)
{
	typedef DownwardInclFctor<Aut, Rel> InclFctor;

	typedef typename Aut::StateType StateType;
	typedef typename InclFctor::StateSet StateSet;

	typedef typename InclFctor::WorkSetType WorkSetType;
	typedef typename InclFctor::NonInclAntichainType NonInclusionCache;

	typedef VATA::Util::Convert Convert;

	WorkSetType workset;
	NonInclusionCache nonIncl;


	typename Rel::IndexType preorderSmaller;
	typename Rel::IndexType preorderBigger;
	preorder.buildIndex(preorderBigger, preorderSmaller);

	InclFctor downFctor(smaller, bigger, workset, nonIncl, preorder,
		preorderSmaller, preorderBigger);

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
