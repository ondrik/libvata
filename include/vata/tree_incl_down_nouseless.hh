/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for function checking tree automata language inclusion
 *    downwards for automata without useless states.
 *
 *****************************************************************************/

#ifndef _TREE_INCL_DOWN_NOUSELESS_HH_
#define _TREE_INCL_DOWN_NOUSELESS_HH_

// VATA headers
#include <vata/vata.hh>

namespace VATA
{
	template <typename Aut>
	bool CheckDownwardTreeInclusionNoUseless(const Aut& smaller,
		const Aut& bigger);
}


/**
 * @brief  Checks language inclusion on tree automata downwards assuming there
 *
 * This is a general method for tree automata to check language inclusion
 * downward. This method assumes that there are no useless states and
 * transitions in the automata.
 *
 * @todo  Write this documentation
 */
template <typename Aut>
bool VATA::CheckDownwardTreeInclusionNoUseless(const Aut& smaller,
	const Aut& bigger)
{
	typedef VATA::Util::Convert Convert;

	typedef typename Aut::StateType StateType;
	typedef typename Aut::StateTuple StateTuple;
	typedef typename Aut::StateTupleSet StateTupleSet;

	typedef typename VATA::Util::OrdVector<StateType> StateSet;

	typedef std::pair<StateType, StateSet> WorkSetElement;

	typedef std::unordered_multimap<typename WorkSetElement::first_type,
		typename WorkSetElement::second_type> WorkSetType;

	typedef std::unordered_multimap<StateType, StateSet> NonInlusionCache;

	typedef std::pair<StateType, StateSet> StateStateSetPair;
	typedef std::unordered_map<StateStateSetPair, bool,
		boost::hash<StateStateSetPair>> StateStateSetPairToBoolMap;

	class DownwardInclusionFunctor
	{
	private:  // data types

		typedef std::vector<unsigned> ChoiceFunctionType;

		class SequentialChoiceFunctionGenerator
		{
		private:  // data types

			typedef ChoiceFunctionType::value_type RangeType;

		private:  // data members

			ChoiceFunctionType currentCf_;

			RangeType range_;
			char lastCnt_;

		public:   // methods

			SequentialChoiceFunctionGenerator(size_t length, RangeType range) :
				currentCf_(length, range-1),
				range_(range),
				lastCnt_(range == 0? 0 : 2)   // 2 because the initial GetNext() decrements it
			{
				// Assertions
				assert(!currentCf_.empty());
			}

			inline const ChoiceFunctionType& GetNext()
			{
				// Assertions
				assert(!IsLast());

				// move to the next choice function
				size_t index = 0;
				while (++currentCf_[index] == range_) {
					currentCf_[index] = 0; // reset this counter
					++index;               // move to the next counter

					if (index == currentCf_.size()) {
						// if we drop out from the n-tuple
						--lastCnt_;
						break;
					}
				}

				return currentCf_;
			}

			inline bool IsLast() const
			{
				return lastCnt_ == 0;
			}
		};

		typedef SequentialChoiceFunctionGenerator ChoiceFunctionGenerator;

	private:  // data members

		const Aut& smaller_;
		const Aut& bigger_;

		bool processingStopped_;
		bool inclusionHolds_;

		WorkSetType& workset_;

		NonInlusionCache& nonIncl_;

		StateStateSetPairToBoolMap& expandCache_;

	private:  // methods

		bool expand(const StateType& smallerState, const StateSet& biggerStateSet)
		{
			auto key = std::make_pair(smallerState, biggerStateSet);

			if (isInWorkset(key))
			{	// in case we returned somewhere we already know
				return true;
			}
			else if (isNoninclusionImplied(key))
			{	// in case we know that the inclusion does not hold
				return false;
			}

			workset_.insert(key);

			DownwardInclusionFunctor innerFctor(*this);
			Aut::ForeachDownSymbolFromStateAndStateSetDo(smaller_, bigger_,
				smallerState, biggerStateSet, innerFctor);

			// erase the element
			bool erased = false;
			for (auto keyRange = workset_.equal_range(smallerState);
				keyRange.first != keyRange.second; ++(keyRange.first))
			{	// for all items with proper key
				if (biggerStateSet == (keyRange.first)->second)
				{	// if we found what we were looking for
					workset_.erase(keyRange.first);
					erased = true;
					break;
				}
			}

			// make sure the element was removed
			assert(erased);

			return innerFctor.InclusionHolds();
		}

		inline void failProcessing()
		{
			inclusionHolds_ = false;
			processingStopped_ = true;
		}

#if 0
		// no antichain
		inline bool isInWorkset(const WorkSetElement& key) const
		{
			for (auto keyRange = workset_.equal_range(elem.first);
				keyRange.first != keyRange.second; ++(keyRange.first))
			{	// for all items with proper key
				const StateSet& wsBigger = (keyRange.first)->second;

				if (elem.second == wsBigger)
				{
					return true;
				}
			}

			return false;
		}
#endif

		// workset antichain
		inline bool isInWorkset(const WorkSetElement& elem) const
		{
			for (auto keyRange = workset_.equal_range(elem.first);
				keyRange.first != keyRange.second; ++(keyRange.first))
			{	// for all items with proper key
				const StateSet& wsBigger = (keyRange.first)->second;

				if (wsBigger.IsSubsetOf(elem.second))
				{	// if there is a smaller set in the workset
					return true;
				}
			}

			return false;
		}

		inline bool isNoninclusionImplied(const WorkSetElement& elem) const
		{
			assert(&elem != nullptr);

			for (auto keyRange = nonIncl_.equal_range(elem.first);
				keyRange.first != keyRange.second; ++(keyRange.first))
			{	// for all items with proper key
				const StateSet& wsBigger = (keyRange.first)->second;

				if (elem.second.IsSubsetOf(wsBigger))
				{	// if there is a bigger set in the workset
					return true;
				}
			}

			return false;
		}

		inline void processFoundNoninclusion(const StateType& smallerState,
			const StateSet& biggerStateSet)
		{
			auto keyRange = nonIncl_.equal_range(smallerState);
			for (auto itRange = keyRange.first; itRange != keyRange.second; ++itRange)
			{	// for all elements for smallerState
				const StateSet& wsBigger = (keyRange.first)->second;
				if (biggerStateSet.IsSubsetOf(wsBigger))
				{	// if there is a bigger set in the workset, skip
					return;
				}
			}

			while (keyRange.first != keyRange.second)
			{	// until we process all elements for smallerState
				const StateSet& wsBigger = (keyRange.first)->second;

				if (wsBigger.IsSubsetOf(biggerStateSet) &&
					(wsBigger.size() < biggerStateSet.size()))
				{	// if there is a _strictly_ smaller set in the workset
					auto nextPtr = keyRange.first;
					++nextPtr;
					nonIncl_.erase(keyRange.first);
					keyRange.first = nextPtr;
				}
				else
				{
					++(keyRange.first);
				}
			}

			nonIncl_.insert(std::make_pair(smallerState, biggerStateSet));
		}

	public:   // methods

		DownwardInclusionFunctor(const Aut& smaller, const Aut& bigger,
			WorkSetType& workset, NonInlusionCache& nonIncl, StateStateSetPairToBoolMap& expandCache) :
			smaller_(smaller),
			bigger_(bigger),
			processingStopped_(false),
			inclusionHolds_(true),
			workset_(workset),
			nonIncl_(nonIncl),
			expandCache_(expandCache)
		{ }

		DownwardInclusionFunctor(DownwardInclusionFunctor& downFctor) :
			smaller_(downFctor.smaller_),
			bigger_(downFctor.bigger_),
			processingStopped_(false),
			inclusionHolds_(true),
			workset_(downFctor.workset_),
			nonIncl_(downFctor.nonIncl_),
			expandCache_(downFctor.expandCache_)
		{ }

		void operator()(const StateTupleSet& lhs, const StateTupleSet& rhs)
		{
			if (lhs.empty())
			{	// in case there is nothing in LHS the inclusion simply holds
				return;
			}

			// arity of the symbol
			const size_t arity = lhs.begin()->size();

			if (arity == 0)
			{	// in case LHS represents a nullary transition (i.e., a leaf symbol)
				assert(lhs.size() == 1);

				if (!rhs.empty())
				{	// in case there also a nullary transition in the RHS
					assert(rhs.size() == 1);
					assert(rhs.begin()->size() == 0);
				}
				else
				{	// in case RHS cannot make this transition
					failProcessing();
					return;
				}
			}
			else
			{	// in case the transition si not nullary
				for (const StateTuple& lhsTuple : lhs)
				{
					// Assertions
					assert(lhsTuple.size() == arity);

					if (rhs.empty())
					{	// in case RHS is empty
						failProcessing();
						return;
					}
					else
					{
						const std::vector<StateTuple>& rhsVector = rhs.ToVector();

						ChoiceFunctionGenerator cfGen(rhsVector.size(), lhsTuple.size());
						while (!cfGen.IsLast())
						{	// for each choice function
							const ChoiceFunctionType& cf = cfGen.GetNext();
							bool found = false;

							for (size_t tuplePos = 0; tuplePos < arity; ++tuplePos)
							{ // for each position of the n-tuple
								StateSet rhsSetForTuplePos;

								for (size_t cfIndex = 0; cfIndex < cf.size(); ++cfIndex)
								{	// for each element in the choice function
									if (cf[cfIndex] == tuplePos)
									{ // in case the choice function for given vector is at
										// current position in the tuple
										assert(cfIndex < rhsVector.size());
										const StateTuple& rhsTuple = rhsVector[cfIndex];
										assert(rhsTuple.size() == arity);

										// insert tuplePos-th state of the cfIndex-th tuple in the
										// RHS into the set
										rhsSetForTuplePos.insert(rhsTuple[tuplePos]);
									}
								}

								typename StateStateSetPairToBoolMap::const_iterator itCache;
								if ((itCache = expandCache_.find(std::make_pair(lhsTuple[tuplePos],
									rhsSetForTuplePos))) != expandCache_.end())
								{	// in case the result is cached
									if (itCache->second)
									{	// in case the inclusion holds
										found = true;
										break;
									}
								}
								else
								{
									bool res = expand(lhsTuple[tuplePos], rhsSetForTuplePos);
									expandCache_.insert(std::make_pair(std::make_pair(lhsTuple[tuplePos],
										rhsSetForTuplePos), res));

									if (res)
									{	// in case inclusion holds for this case
										found = true;
										break;
									}
									else
									{	// in case inclusion does not hold, cache the result
										processFoundNoninclusion(lhsTuple[tuplePos], rhsSetForTuplePos);
									}
								}
							}

							if (!found)
							{	// in case the inclusion does not hold
								failProcessing();
								return;
							}
						}
					}
				}
			}
		}

		inline bool IsProcessingStopped() const
		{
			return processingStopped_;
		}

		inline bool InclusionHolds() const
		{
			return inclusionHolds_;
		}

		inline void Reset()
		{
			inclusionHolds_ = true;
			processingStopped_ = false;
		}
	};

	WorkSetType workset;
	NonInlusionCache nonIncl;
	StateStateSetPairToBoolMap expandCache;

	DownwardInclusionFunctor downFctor(smaller, bigger, workset, nonIncl,
		expandCache);

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
