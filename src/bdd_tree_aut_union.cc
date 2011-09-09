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
BDDTreeAut VATA::Union<BDDTreeAut>(const BDDTreeAut& lhs, const BDDTreeAut& rhs,
	AutBase::StateToStateMap* pTranslMap)
{
	// Assertions
	assert(lhs.isValid());
	assert(rhs.isValid());

	typedef BDDTreeAut::StateType StateType;
	typedef BDDTreeAut::StateSet StateSet;
	typedef BDDTreeAut::StateTuple StateTuple;
	typedef BDDTreeAut::StateTupleSet StateTupleSet;
	typedef AutBase::StateToStateMap StateToStateMap;

	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class RewriterApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<RewriterApplyFunctor,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // data members

		BDDTreeAut& aut_;
		StateToStateMap& dict_;

	public:   // methods

		RewriterApplyFunctor(BDDTreeAut& aut, StateToStateMap& dict) :
			aut_(aut),
			dict_(dict)
		{ }

		inline StateTupleSet ApplyOperation(const StateTupleSet& value)
		{
			StateTupleSet result;

			for (StateTupleSet::const_iterator itSts = value.begin();
				itSts != value.end(); ++itSts)
			{	// for every tuple
				const StateTuple& tup = *itSts;

				StateTuple resTuple;
				for (StateTuple::const_iterator itTup = tup.begin();
					itTup != tup.end(); ++itTup)
				{	// for every element in the tuple
					resTuple.push_back(aut_.safelyTranslateToState(*itTup, dict_));
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

		bool deleteTranslMap = false;
		if (pTranslMap == nullptr)
		{
			pTranslMap = new StateToStateMap;
			deleteTranslMap = true;
		}

		RewriterApplyFunctor rewriter(result, *pTranslMap);

		for (StateSet::const_iterator itSt = rhs.GetStates().begin();
			itSt != rhs.GetStates().end(); ++itSt)
		{	// for all states in the RHS automaton
			StateType translState = result.safelyTranslateToState(*itSt, *pTranslMap);

			BDDTreeAut::TransMTBDD newMtbdd = rewriter(rhs.getMtbdd(*itSt));

			result.setMtbdd(translState, newMtbdd);
		}

		for (StateSet::const_iterator itFst = rhs.GetFinalStates().begin();
			itFst != rhs.GetFinalStates().end(); ++itFst)
		{	// for all final states in the RHS automaton
			result.SetStateFinal(result.safelyTranslateToState(*itFst, *pTranslMap));
		}

		if (deleteTranslMap)
		{
			delete pTranslMap;
		}

		assert(result.isValid());

		return result;
	}
}