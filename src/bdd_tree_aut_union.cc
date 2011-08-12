/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union on BDD tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_tree_aut_op.hh>
#include <vata/mtbdd/apply1func.hh>

using VATA::BDDTreeAut;

// Standard library headers
#include <unordered_map>


template <>
BDDTreeAut VATA::Union<BDDTreeAut>(const BDDTreeAut& lhs, const BDDTreeAut& rhs)
{
	// Assertions
	assert(lhs.isValid());
	assert(rhs.isValid());

	typedef BDDTreeAut::StateType StateType;
	typedef BDDTreeAut::StateSet StateSet;
	typedef BDDTreeAut::StateTuple StateTuple;
	typedef BDDTreeAut::StateTupleSet StateTupleSet;
	typedef std::unordered_map<StateType, StateType> StateToStateHT;

	class RewriterApplyFunctor :
		public VATA::MTBDDPkg::AbstractApply1Functor<StateTupleSet, StateTupleSet>
	{
	private:  // data members

		BDDTreeAut* pAut_;
		StateToStateHT* pDict_;

	private:  // methods

		RewriterApplyFunctor(const RewriterApplyFunctor&);
		RewriterApplyFunctor& operator=(const RewriterApplyFunctor&);

	public:   // methods

		RewriterApplyFunctor(BDDTreeAut* pAut, StateToStateHT* pDict) :
			pAut_(pAut),
			pDict_(pDict)
		{
			// Assertions
			assert(pAut_ != static_cast<BDDTreeAut*>(0));
			assert(pDict_ != static_cast<StateToStateHT*>(0));
		}

		virtual StateTupleSet ApplyOperation(const StateTupleSet& value)
		{
			// Assertions
			assert(pAut_ != static_cast<BDDTreeAut*>(0));
			assert(pDict_ != static_cast<StateToStateHT*>(0));

			StateTupleSet result;

			for (StateTupleSet::const_iterator itSts = value.begin();
				itSts != value.end(); ++itSts)
			{	// for every tuple
				const StateTuple& tup = *itSts;

				StateTuple resTuple;
				for (StateTuple::const_iterator itTup = tup.begin();
					itTup != tup.end(); ++itTup)
				{	// for every element in the tuple
					resTuple.push_back(pAut_->safelyTranslateToState(*itTup, *pDict_));
				}

				result.insert(resTuple);
			}

			return result;
		}
	};

	if (lhs.GetTransTable() == rhs.GetTransTable())
	{	// in case the automata share their transition table
		BDDTreeAut result = lhs;
		result.copyStates(rhs);

		assert(result.isValid());
		return result;
	}
	else
	{	// in case the automata have distinct transition tables

		// start by copying the LHS automaton
		BDDTreeAut result = lhs;

		StateToStateHT stateDict;
		RewriterApplyFunctor rewriter(&result, &stateDict);

		for (StateSet::const_iterator itSt = rhs.GetStates().begin();
			itSt != rhs.GetStates().end(); ++itSt)
		{	// for all states in the RHS automaton
			StateType translState = result.safelyTranslateToState(*itSt, stateDict);

			BDDTreeAut::TransMTBDD newMtbdd = rewriter(rhs.getMtbdd(*itSt));

			result.setMtbdd(translState, newMtbdd);
		}

		for (StateSet::const_iterator itFst = rhs.GetFinalStates().begin();
			itFst != rhs.GetFinalStates().end(); ++itFst)
		{	// for all final states in the RHS automaton
			result.SetStateFinal(result.safelyTranslateToState(*itFst, stateDict));
		}

		assert(result.isValid());

		return result;
	}
}
