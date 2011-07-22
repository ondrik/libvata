/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a BDD-based tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TREE_AUT_HH_
#define _VATA_BDD_TREE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/abstr_aut.hh>
#include <vata/mtbdd/apply2func.hh>
#include <vata/util/ord_vector.hh>
#include <vata/util/vector_map.hh>

// Standard library headers
#include <stdint.h>

namespace VATA { class BDDTreeAut; }

class VATA::BDDTreeAut
	: public AbstrAut
{
public:   // public data types

	typedef uint32_t StateType;

private:  // private data types

	typedef std::vector<StateType> StateTuple;
	typedef VATA::Util::OrdVector<StateType> StateSet;

	typedef VATA::MTBDDPkg::OndriksMTBDD<StateSet> MTBDD;

	typedef VATA::Util::VectorMap<StateType, MTBDD> TupleToMTBDDTableType;

private:  // private data members

	StateSet states_;
	StateSet finalStates_;
	TupleToMTBDDTableType mtbddMap_;

private:  // private methods

	inline bool isValid() const
	{
		if (!std::includes(states_.begin(), states_.end(),
			finalStates_.begin(), finalStates_.end()))
		{	// in case the set of final states is not a subset of the set of states
			return false;
		}

		return true;
	}

	void copyStates(const BDDTreeAut& src)
	{
		// Assertions
		assert(isValid());
		assert(src.isValid());
		assert(states_.HaveEmptyIntersection(src.states_));

		states_.insert(src.states_);
		finalStates_.insert(src.finalStates_);

		assert(isValid());
	}

	const MTBDD& getMtbdd(const StateTuple& tuple) const
	{
		// Assertions
		assert(isValid());

		return mtbddMap_.GetValue(tuple);
	}

	void setMtbdd(const StateTuple& tuple, const MTBDD& mtbdd)
	{
		// Assertions
		assert(isValid());

		mtbddMap_.SetValue(tuple, mtbdd);
	}

	static BDDTreeAut* makeUnionBU(const BDDTreeAut& lhs, const BDDTreeAut& rhs,
			const std::string& params)
	{
		// Assertions
		assert(lhs.isValid());
		assert(rhs.isValid());
		assert(lhs.states_.HaveEmptyIntersection(rhs.states_));

		class UnionApplyFunctor
			: public VATA::MTBDDPkg::AbstractApply2Functor<StateSet, StateSet, StateSet>
		{
		public:
			virtual StateSet ApplyOperation(const StateSet& lhs, const StateSet& rhs)
			{
				return lhs.Union(rhs);
			}
		};

		BDDTreeAut* result = new BDDTreeAut(lhs);
		result->copyStates(rhs);

		const MTBDD& lhsMtbdd = lhs.getMtbdd(StateTuple());
		const MTBDD& rhsMtbdd = rhs.getMtbdd(StateTuple());

		UnionApplyFunctor unionFunc;
		MTBDD resultMtbdd = unionFunc(lhsMtbdd, rhsMtbdd);

		result->setMtbdd(StateTuple(), resultMtbdd);

		return result;
	}


public:   // public methods

	explicit BDDTreeAut(const std::string& params)
		: states_(),
			finalStates_(),
			mtbddMap_(MTBDD(StateSet()))
	{
		// Assertions
		assert(isValid());
	}

	BDDTreeAut(const BDDTreeAut& aut)
		: states_(aut.states_),
			finalStates_(aut.finalStates_),
			mtbddMap_(aut.mtbddMap_)
	{
		// Assertions
		assert(isValid());
	}

	inline bool HasEmptyStateSet() const
	{
		// Assertions
		assert(isValid());

		return states_.empty();
	}

};

#endif
