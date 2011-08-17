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
	typedef typename Aut::StateTuple StateTuple;
	typedef typename Aut::StateTupleSet StateTupleSet;

	class DownwardAntichainFunctor
	{
	private:  // data members

		bool processingStopped_;
		bool inclusionHolds_;

	public:   // methods

		DownwardAntichainFunctor() :
			processingStopped_(false),
			inclusionHolds_(true)
		{ }

		void operator()(const StateTupleSet& lhs, const StateTupleSet& rhs)
		{
			if (!lhs.empty() && (lhs.begin()->size() == 0))
			{	// in case LHS represents a nullary transition (i.e., a leaf symbol)
				assert(lhs.size() == 1);

				if (!rhs.empty())
				{	// in case there also a nullary transition in the RHS
					assert(rhs.size() == 1);
					assert(rhs.begin()->size() == 0);
				}
				else
				{	// in case RHS cannot make this transition
					//doesInclusionHold_ = false;
					processingStopped_ = true;
					inclusionHolds_ = false;
				}
			}
			else
			{	// in case the transition si not nullary
				for (const StateTuple& tuple : lhs)
				{
					assert(false);

				}
			}
		}

		inline bool IsProcessingStopped() const
		{
			return processingStopped_;
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
