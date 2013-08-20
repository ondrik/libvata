/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file with the optimised downward tree automata language inclusion
 *    checking functor (assuming that the automata do not have useless states).
 *
 *****************************************************************************/

#ifndef _VATA_DOWN_TREE_OPT_INCL_FCTOR_HH_
#define _VATA_DOWN_TREE_OPT_INCL_FCTOR_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>

#include "util/cache.hh"


namespace VATA
{
	template <class Aut, class Rel>
	class OptDownwardInclusionFunctor;
}

// NOTE: the automata cannot have useless states
template <class Aut, class Rel>
class VATA::OptDownwardInclusionFunctor
{
public:   // data types

	typedef typename Aut::StateType StateType;
	typedef typename VATA::Util::OrdVector<StateType> StateSet;
	typedef typename Aut::StateTuple StateTuple;
	typedef typename Aut::DownInclStateTupleSet StateTupleSet;
	typedef typename Aut::DownInclStateTupleVector StateTupleVector;

	typedef typename Util::Cache<
		StateSet, std::function<void(const StateSet*)>
	> BiggerTypeCache;

	typedef typename BiggerTypeCache::TPtr BiggerType;

	typedef Util::CachedBinaryOp<const StateSet*, const StateSet*, bool> LteCache;

	typedef std::pair<StateType, BiggerType> WorkSetElement;

	typedef std::set<WorkSetElement> ConsequentType;

	typedef std::unordered_multimap<typename WorkSetElement::first_type,
		typename WorkSetElement::second_type> WorkSetType;

	typedef Rel Relation;
	typedef typename Relation::IndexType IndexType;

	typedef VATA::Util::Antichain2Cv2
	<
		StateType,
		BiggerType
	> InclAntichainType;

	typedef VATA::Util::Antichain2Cv2
	<
		StateType,
		BiggerType
	> NonInclAntichainType;

	class SetComparerSmaller
	{
	private:  // data types

		class NonCachedLte
		{
		private:

			const typename Relation::IndexType& ind_;

		public:

			NonCachedLte(const typename Relation::IndexType& ind) :
				ind_(ind)
			{ }

			inline bool operator()(const StateSet* x, const StateSet* y) const
			{
				// Assertions
				assert(x != nullptr);
				assert(y != nullptr);

				for (const auto& s : *x)
				{
					assert(s < ind_.size());

					const auto& s1 = ind_[s];
					const auto& s2 = *y;

					auto i1 = s1.begin(), i2 = s2.begin();

					bool found = false;
					while (i1 != s1.end() && i2 != s2.end())
					{
						if (*i1 < *i2)
						{
							++i1;
						}
						else if (*i2 < *i1)
						{
							++i2;
						}
						else
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						return false;
					}
				}

				return true;
			}
		};


	private:  // data members

		LteCache& lteCache_;
		NonCachedLte noncachedLte_;

	public:   // methods

		explicit SetComparerSmaller(LteCache& lteCache,
			const typename Relation::IndexType& ind) :
			lteCache_(lteCache),
			noncachedLte_(ind)
		{ }

		bool operator()(const BiggerType& lhs, const BiggerType& rhs) const
		{
			// Assertions
			assert(lhs != nullptr);
			assert(rhs != nullptr);

			if (lhs == rhs)
			{
				return true;
			}
			else
			{
				return lteCache_.lookup(lhs.get(), rhs.get(), noncachedLte_);
			}
		}
	};

	class SetComparerBigger
	{
	private:  // data members

		SetComparerSmaller& smallerCmp_;

	public:   // methods

		SetComparerBigger(SetComparerSmaller& smallerCmp) :
			smallerCmp_(smallerCmp)
		{ }

		// TODO: this could be done better
		bool operator()(const BiggerType& lhs, const BiggerType& rhs) const
		{
			return smallerCmp_(rhs, lhs);
		}
	};

private:  // data types

	typedef VATA::Util::Convert Convert;

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

	BiggerTypeCache& biggerTypeCache_;

	bool processingStopped_;
	bool inclusionHolds_;

	WorkSetType& workset_;

	InclAntichainType& incl_;
	NonInclAntichainType& nonIncl_;

	InclAntichainType childrenCache_;

	const Relation& preorder_;

	const IndexType& preorderSmaller_;
	const IndexType& preorderBigger_;

	const SetComparerSmaller& smallerComparer_;
	const SetComparerBigger& biggerComparer_;

	InclAntichainType& ant_;
	ConsequentType& cons_;

private:  // methods

	std::tuple<bool, InclAntichainType, ConsequentType> expand(
		const StateType& smallerState, const BiggerType& biggerStateSet)
	{
		auto key = std::make_pair(smallerState, biggerStateSet);

		bool res;
		WorkSetElement elem;

		if (std::get<0>(std::tie(res, elem) = isInWorkset(key)))
		{	// in case we returned somewhere we already know
			InclAntichainType antec;
			antec.insert(elem.first, elem.second);
			return std::make_tuple(true, antec, ConsequentType());
		}
		else if (isInclusionImplied(key))
		{	// in case we know that the inclusion does hold
			return std::make_tuple(true, InclAntichainType(), ConsequentType());
		}
		else if (isNoninclusionImplied(key))
		{	// in case we know that the inclusion does not hold
			return std::make_tuple(false, InclAntichainType(), ConsequentType());
		}
		else if (isImpliedByChildren(key))
		{
			return std::make_tuple(true, InclAntichainType(), ConsequentType());
		}
		else if (IsImpliedByPreorder(key))
		{
			return std::make_tuple(true, InclAntichainType(), ConsequentType());
		}

		workset_.insert(key);

		InclAntichainType antecedent;
		ConsequentType consequent;

		OptDownwardInclusionFunctor innerFctor(*this, antecedent, consequent);
		Aut::ForeachDownSymbolFromStateAndStateSetDo(smaller_, bigger_,
			smallerState, *biggerStateSet, innerFctor);

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
		if (!erased)
		{
			assert(false);       // fail gracefully
		}

		// cache the result
		if (innerFctor.InclusionHolds())
		{
			processFoundInclusion(smallerState, biggerStateSet);
		}
		else
		{
			processFoundNoninclusion(smallerState, biggerStateSet);
		}

		if (antecedent.empty())
		{
			for (const auto& consElem : consequent)
			{
				processFoundGlobalInclusion(consElem.first, consElem.second);
			}

			consequent.clear();
		}

		return std::make_tuple(innerFctor.InclusionHolds(), antecedent, consequent);
	}

	inline void failProcessing()
	{
		inclusionHolds_ = false;
		processingStopped_ = true;
	}

	// workset antichain
	inline std::tuple<bool, WorkSetElement> isInWorkset(
		const WorkSetElement& elem) const
	{
		for (auto stateSetPair : workset_)
		{
			if (preorder_.get(elem.first, stateSetPair.first))
			{	// if the pair is worth processing
				if (smallerComparer_(stateSetPair.second, elem.second))
				{
					return std::make_tuple(true, stateSetPair);
				}
			}
		}

		return std::make_tuple(false, WorkSetElement());
	}

	inline bool isImpliedByChildren(const WorkSetElement& elem) const
	{
		return childrenCache_.contains(preorderBigger_[elem.first],
			elem.second, smallerComparer_);
	}

	inline bool isNoninclusionImplied(const WorkSetElement& elem) const
	{
		return nonIncl_.contains(preorderSmaller_[elem.first],
			elem.second, biggerComparer_);
	}

	inline bool isInclusionImplied(const WorkSetElement& elem) const
	{
		return incl_.contains(preorderBigger_[elem.first],
			elem.second, smallerComparer_);
	}

	inline void processFoundInclusion(const StateType& smallerState,
		const BiggerType& biggerStateSet)
	{
		if (!childrenCache_.contains(preorderBigger_[smallerState], biggerStateSet,
			smallerComparer_))
		{	// if the element is not implied by the antichain
			childrenCache_.refine(preorderSmaller_[smallerState], biggerStateSet,
				biggerComparer_);
			childrenCache_.insert(smallerState, biggerStateSet);
		}
	}

	inline void processFoundGlobalInclusion(const StateType& smallerState,
		const BiggerType& biggerStateSet)
	{
		if (!incl_.contains(preorderBigger_[smallerState], biggerStateSet,
			smallerComparer_))
		{	// if the element is not implied by the antichain
			incl_.refine(preorderSmaller_[smallerState], biggerStateSet,
				biggerComparer_);
			incl_.insert(smallerState, biggerStateSet);
		}
	}

	inline void processFoundNoninclusion(const StateType& smallerState,
		const BiggerType& biggerStateSet)
	{
		if (!nonIncl_.contains(preorderSmaller_[smallerState], biggerStateSet,
			biggerComparer_))
		{	// if the element is not implied by the antichain
			nonIncl_.refine(preorderBigger_[smallerState], biggerStateSet,
				smallerComparer_);
			nonIncl_.insert(smallerState, biggerStateSet);
		}
	}

public:   // methods

	OptDownwardInclusionFunctor(const Aut& smaller, const Aut& bigger,
		BiggerTypeCache& biggerTypeCache,
		WorkSetType& workset, InclAntichainType& incl,
		NonInclAntichainType& nonIncl, const Relation& preorder,
		const IndexType& preorderSmaller, const IndexType& preorderBigger,
		const SetComparerSmaller& smallerComparer,
		const SetComparerBigger& biggerComparer,
		InclAntichainType& ant, ConsequentType& cons) :
		smaller_(smaller),
		bigger_(bigger),
		biggerTypeCache_(biggerTypeCache),
		processingStopped_(false),
		inclusionHolds_(true),
		workset_(workset),
		incl_(incl),
		nonIncl_(nonIncl),
		childrenCache_(),
		preorder_(preorder),
		preorderSmaller_(preorderSmaller),
		preorderBigger_(preorderBigger),
		smallerComparer_(smallerComparer),
		biggerComparer_(biggerComparer),
		ant_(ant),
		cons_(cons)
	{ }

	OptDownwardInclusionFunctor(
		OptDownwardInclusionFunctor& downFctor,
		InclAntichainType& ant, ConsequentType& cons) :
		smaller_(downFctor.smaller_),
		bigger_(downFctor.bigger_),
		biggerTypeCache_(downFctor.biggerTypeCache_),
		processingStopped_(false),
		inclusionHolds_(true),
		workset_(downFctor.workset_),
		incl_(downFctor.incl_),
		nonIncl_(downFctor.nonIncl_),
		childrenCache_(),
		preorder_(downFctor.preorder_),
		preorderSmaller_(downFctor.preorderSmaller_),
		preorderBigger_(downFctor.preorderBigger_),
		smallerComparer_(downFctor.smallerComparer_),
		biggerComparer_(downFctor.biggerComparer_),
		ant_(ant),
		cons_(cons)
	{ }

	inline bool IsImpliedByPreorder(const WorkSetElement& elem) const
	{
		for (const StateType& biggerState : *(elem.second))
		{
			if (preorder_.get(elem.first, biggerState))
			{
				return true;
			}
		}

		return false;
	}

	template <class ElementAccessorLHS, class ElementAccessorRHS>
	void operator()(const StateTupleSet& lhs, ElementAccessorLHS lhsElemAccess,
		const StateTupleSet& rhs, ElementAccessorRHS rhsElemAccess)
	{
		if (lhs.empty())
		{	// in case there is nothing in LHS the inclusion simply holds
			return;
		}

		// arity of the symbol
		const size_t arity = lhsElemAccess(*lhs.begin()).size();

		if (arity == 0)
		{	// in case LHS represents a nullary transition (i.e., a leaf symbol)
			assert(lhs.size() == 1);

			if (!rhs.empty())
			{	// in case there also a nullary transition in the RHS
				assert(rhs.size() == 1);
				assert(rhsElemAccess(*rhs.begin()).size() == 0);
				return;
			}
			else
			{	// in case RHS cannot make this transition
				failProcessing();
				return;
			}
		}
		else
		{	// in case the transition si not nullary
			if (rhs.empty())
			{	// in case RHS is empty
				failProcessing();
				return;
			}

			for (auto lhsTupleCont : lhs)
			{
				const StateTuple& lhsTuple = lhsElemAccess(lhsTupleCont);

				// Assertions
				assert(lhsTuple.size() == arity);

				// first check whether there is a bigger tuple
				bool valid = false;
				for (auto rhsTupleCont : rhs)
				{
					const StateTuple& rhsTuple = rhsElemAccess(rhsTupleCont);

					valid = true;
					for (size_t i = 0; i < arity; ++i)
					{
						bool res;
						InclAntichainType ant;
						ConsequentType cons;
						std::tie(res, ant, cons)
							= expand(lhsTuple[i], biggerTypeCache_.lookup(StateSet(rhsTuple[i])));
						if (!res)
						{
							valid = false;
							break;
						}
						else
						{
							StateType antElemFirst;
							BiggerType antElemSecond;
							while (ant.get(antElemFirst, antElemSecond))
							{
								if (!ant_.contains(preorderSmaller_[antElemFirst],
									antElemSecond, smallerComparer_))
								{	// if the element is not implied by the antichain
									ant_.refine(preorderSmaller_[antElemFirst], antElemSecond,
										smallerComparer_);
									ant_.insert(antElemFirst, antElemSecond);
								}
							}

							cons_.insert(cons.begin(), cons.end());
						}
					}

					if (valid)
					{
						break;
					}
				}

				if (valid)
				{	// in case there was a bigger tuple
					continue;
				}

				// in case there is not a bigger tuple

				// TODO: could be done more smartly (without conversion to vector)
				StateTupleVector rhsVector = Aut::StateTupleSetToVector(rhs);

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
								const StateTuple& rhsTuple = rhsElemAccess(rhsVector[cfIndex]);
								assert(rhsTuple.size() == arity);

								// insert tuplePos-th state of the cfIndex-th tuple in the
								// RHS into the set
								rhsSetForTuplePos.insert(rhsTuple[tuplePos]);
							}
						}

						if (rhsSetForTuplePos.empty())
						{	// in case the right-hand side set is empty, we exploit the
							// fact that there are no useless states in any of the automata
							continue;
						}

						bool res;
						InclAntichainType ant;
						ConsequentType cons;
						std::tie(res, ant, cons) =
							expand(lhsTuple[tuplePos], biggerTypeCache_.lookup(rhsSetForTuplePos));
						if (res)
						{	// in case inclusion holds for this case
							found = true;

							StateType antElemFirst;
							BiggerType antElemSecond;
							while (ant.get(antElemFirst, antElemSecond))
							{
								if (!ant_.contains(preorderSmaller_[antElemFirst],
									antElemSecond, smallerComparer_))
								{	// if the element is not implied by the antichain
									ant_.refine(preorderSmaller_[antElemFirst], antElemSecond,
										smallerComparer_);
									ant_.insert(antElemFirst, antElemSecond);
								}
							}

							cons_.insert(cons.begin(), cons.end());
							break;
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

#endif
