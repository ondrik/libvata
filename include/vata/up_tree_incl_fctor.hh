/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file with the upward tree automata language inclusion
 *    checking functor.
 *
 *****************************************************************************/

#ifndef _VATA_UP_TREE_INCL_FCTOR_HH_
#define _VATA_UP_TREE_INCL_FCTOR_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>


namespace VATA
{
	template <class Aut>
	class UpwardInclusionFunctor;
}

template <class Aut>
class VATA::UpwardInclusionFunctor
{
public:   // data types

	typedef Aut AutType;
	typedef typename AutType::StateType StateType;
	typedef typename AutType::StateSet StateSet;
	typedef typename AutType::StateTuple StateTuple;
	typedef typename AutType::StateTupleSet StateTupleSet;

	typedef VATA::Util::Antichain2Cv2
	<
		StateType,
		StateSet
	> AntichainType;

private:  // data types

	typedef VATA::Util::Convert Convert;

private:  // data members

	const Aut& smaller_;
	const Aut& bigger_;

	AntichainType& workset_;
	AntichainType& antichain_;

	bool processingStopped_;
	bool inclusionHolds_;

private:  // methods

	/**
	 * @brief  Less than or equal comparer of StateSets
	 *
	 * Returns @p true if @p lhs is a subset of @p rhs, @p false otherwise.
	 *
	 * @param[in]  lhs  The left-hand side
	 * @param[in]  rhs  The right-hand side
	 *
	 * @returns  @p true if @p lhs is a subset of @p rhs, @p false otherwise
	 */
	static bool lteComparer(const StateSet& lhs, const StateSet& rhs)
	{
		return lhs.IsSubsetOf(rhs);
	}

	/**
	 * @brief  Greater than or equal comparer of StateSets
	 *
	 * Returns @p true if @p lhs is a superset of @p rhs, @p false otherwise.
	 *
	 * @param[in]  lhs  The left-hand side
	 * @param[in]  rhs  The right-hand side
	 *
	 * @returns  @p true if @p lhs is a superset of @p rhs, @p false otherwise
	 */
	static bool gteComparer(const StateSet& lhs, const StateSet& rhs)
	{
		return rhs.IsSubsetOf(lhs);
	}

	inline void failProcessing()
	{
		inclusionHolds_ = false;
		processingStopped_ = true;
	}

	bool isImplied(const StateType& smallerState, const StateSet& biggerSet) const
	{
		std::vector<StateType> tmpList = {smallerState};
		return antichain_.contains(tmpList, biggerSet, lteComparer);
	}

	inline void cachePair(const StateType& smallerState,
		const StateSet& biggerSet) const
	{
		std::vector<StateType> tmpList = {smallerState};

		if (!antichain_.contains(tmpList, biggerSet, lteComparer))
		{	// if the element is not implied by the antichain
			antichain_.refine(tmpList, biggerSet, gteComparer);
			antichain_.insert(smallerState, biggerSet);
			addToWorkset(smallerState, biggerSet);
		}
	}

	inline void addToWorkset(const StateType& smallerState,
		const StateSet& biggerSet) const
	{
		std::vector<StateType> tmpList = {smallerState};

		if (!workset_.contains(tmpList, biggerSet, lteComparer))
		{	// if the element is not implied by the antichain
			workset_.refine(tmpList, biggerSet, gteComparer);
			workset_.insert(smallerState, biggerSet);
		}
	}

public:   // methods

	UpwardInclusionFunctor(const AutType& smaller, const AutType& bigger,
		AntichainType& workset, AntichainType& antichain) :
		smaller_(smaller),
		bigger_(bigger),
		workset_(workset),
		antichain_(antichain),
		processingStopped_(false),
		inclusionHolds_(true)
	{ }

	template <class ElementAccessorLHS, class ElementAccessorRHS>
	void operator()(const StateSet& lhs, ElementAccessorLHS lhsElemAccess,
		const StateSet& rhs, ElementAccessorRHS rhsElemAccess)
	{
		for (auto stateLhsElem : lhs)
		{
			const StateType& stateLhs = lhsElemAccess(stateLhsElem);
			if (!isImplied(stateLhs, rhs))
			{	// in case the pair should be explored
				bool rhsHasFinal = false;
				if (smaller_.IsStateFinal(stateLhs))
				{	// if the state is final in the smaller automaton
					for (auto stateRhsElem : rhs)
					{
						const StateType& stateRhs = rhsElemAccess(stateRhsElem);
						if (bigger_.IsStateFinal(stateRhs))
						{
							rhsHasFinal = true;
							break;
						}
					}

					if (!rhsHasFinal)
					{	// in case there is a counterexample
						failProcessing();
						return;
					}
				}

				cachePair(stateLhs, rhs);
			}
		}
	}

	inline bool InclusionHolds() const
	{
		return inclusionHolds_;
	}

	inline bool IsProcessingStopped() const
	{
		return processingStopped_;
	}
};

#endif
