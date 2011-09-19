/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
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
#include <vata/util/antichain2c.hh>


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

	typedef std::pair<StateType, StateSet> ACPair;

private:  // data types

	struct ACComparer{bool operator()(const ACPair& lhs, const ACPair& rhs)
	{
		return lhs.second.IsSubsetOf(rhs.second);
	}};

	struct ACComparerStrict{bool operator()(const ACPair& lhs, const ACPair& rhs)
	{
		return lhs.second.IsSubsetOf(rhs.second) &&
			lhs.second.size() < rhs.second.size();
	}};

	typedef VATA::Util::Convert Convert;

public:   // data types

	typedef VATA::Util::Antichain2C
	<
		StateType,
		StateSet,
		ACComparer,
		ACComparerStrict
	> AntichainType;

private:  // data members

	const Aut& smaller_;
	const Aut& bigger_;

	AntichainType& workset_;
	AntichainType& antichain_;

	bool processingStopped_;
	bool inclusionHolds_;

private:  // methods

	inline void failProcessing()
	{
		inclusionHolds_ = false;
		processingStopped_ = true;
	}

	bool isImplied(const StateType& smallerState, const StateSet& biggerSet) const
	{
		return antichain_.find(std::make_pair(smallerState, biggerSet)) !=
			antichain_.end();
	}

	inline void cachePair(const StateType& smallerState,
		const StateSet& biggerSet) const
	{
		antichain_.insert(std::make_pair(smallerState, biggerSet));
	}

	inline void addToWorkset(const StateType& smallerState,
		const StateSet& biggerSet) const
	{
		workset_.insert(std::make_pair(smallerState, biggerSet));
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
		VATA_LOGGER_INFO("Processing " + Convert::ToString(lhs) + " <= " + Convert::ToString(rhs));

		for (auto stateLhsElem : lhs)
		{
			const StateType& stateLhs = lhsElemAccess(stateLhsElem);
			if (!isImplied(stateLhs, rhs))
			{	// in case the pair should be explored
				bool rhsHasFinal = false;
				if (smaller_.IsStateFinal(stateLhs))
				{	// if the state is final in the smaller automaton
					VATA_LOGGER_INFO("Smaller is final");
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
						VATA_LOGGER_INFO("Found a counterexample");
						failProcessing();
						return;
					}
				}

				cachePair(stateLhs, rhs);
				addToWorkset(stateLhs, rhs);
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
