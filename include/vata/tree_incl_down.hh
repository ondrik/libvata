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
	typedef typename Aut::StateType StateType;
	typedef typename Aut::StateTuple StateTuple;
	typedef typename Aut::StateTupleSet StateTupleSet;

	typedef typename VATA::Util::OrdVector<StateType> StateSet;

	typedef std::pair<StateType, StateSet> WorkSetElement;

	struct LexicOrder
	{
		bool operator()(const WorkSetElement& lhs, const WorkSetElement& rhs) const
		{
			return (lhs.first < rhs.first)? true : lhs.second < rhs.second;
		}
	};

	typedef std::set<WorkSetElement, LexicOrder> WorkSetType;

	class DownwardAntichainFunctor
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

	private:  // methods

		bool expand(const StateType& smallerState, const StateSet& biggerStateSet)
		{
			auto key = std::make_pair(smallerState, biggerStateSet);

			if (workset_.find(key) != workset_.end())
			{	// in case we returned somewhere we already know
				return true;
			}

			auto itInsertedKey = workset_.insert(key).first;

			DownwardAntichainFunctor innerFctor(smaller_, bigger_, workset_);
			Aut::ForeachDownSymbolFromStateAndStateSetDo(smaller_, bigger_,
				smallerState, biggerStateSet, innerFctor);

			// let's hope that the above processing that uses the workset doesn't
			// mess with the iterator
			workset_.erase(itInsertedKey);

			return innerFctor.InclusionHolds();
		}

		void inline failProcessing()
		{
			inclusionHolds_ = false;
			processingStopped_ = true;
		}

	public:   // methods

		DownwardAntichainFunctor(const Aut& smaller, const Aut& bigger,
			WorkSetType& workset) :
			smaller_(smaller),
			bigger_(bigger),
			processingStopped_(false),
			inclusionHolds_(true),
			workset_(workset)
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
						for (const StateType& lhsTupleState : lhsTuple)
						{
							if (!expand(lhsTupleState, StateSet()))
							{	// if a state from LHS can generate a tree
								failProcessing();
								return;
							}
						}
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

								if (expand(lhsTuple[tuplePos], rhsSetForTuplePos))
								{	// in case inclusion holds for this case
									found = true;
									break;
								}
							}

							if (!found)
							{
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

	DownwardAntichainFunctor downFctor(smaller, bigger, workset);

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
