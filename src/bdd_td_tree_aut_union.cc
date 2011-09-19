/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union on BDD top-down tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_td_tree_aut_op.hh>
#include <vata/mtbdd/apply1func.hh>

using VATA::BDDTopDownTreeAut;

// Standard library headers
#include <unordered_map>


BDDTopDownTreeAut VATA::Union(const BDDTopDownTreeAut& lhs,
	const BDDTopDownTreeAut& rhs, AutBase::StateToStateMap* pTranslMapLhs,
	AutBase::StateToStateMap* pTranslMapRhs)
{
	// Assertions
	assert(lhs.isValid());
	assert(rhs.isValid());

	typedef BDDTopDownTreeAut::StateType StateType;
	typedef BDDTopDownTreeAut::StateSet StateSet;
	typedef BDDTopDownTreeAut::StateTuple StateTuple;
	typedef BDDTopDownTreeAut::StateTupleSet StateTupleSet;
	typedef AutBase::StateToStateMap StateToStateMap;

	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class RewriterApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<RewriterApplyFunctor,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // data members

		BDDTopDownTreeAut& aut_;
		StateToStateMap& dict_;

	public:   // methods

		RewriterApplyFunctor(BDDTopDownTreeAut& aut, StateToStateMap& dict) :
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
		BDDTopDownTreeAut result = lhs;
		result.copyStates(rhs);

		assert(result.isValid());
		return result;
	}
	else
	{	// in case the automata have distinct transition tables

		// start by copying the LHS automaton
		BDDTopDownTreeAut result = lhs;

		StateToStateMap translMapLhs;
		if (pTranslMapLhs != nullptr)
		{	// copy states
			for (const StateType& state : lhs.GetStates())
			{
				pTranslMapLhs->insert(std::make_pair(state, state));
			}
		}

		StateToStateMap translMapRhs;
		if (pTranslMapRhs == nullptr)
		{
			pTranslMapRhs = &translMapRhs;
		}

		RewriterApplyFunctor rewriter(result, *pTranslMapRhs);

		for (StateSet::const_iterator itSt = rhs.GetStates().begin();
			itSt != rhs.GetStates().end(); ++itSt)
		{	// for all states in the RHS automaton
			StateType translState = result.safelyTranslateToState(*itSt, *pTranslMapRhs);

			BDDTopDownTreeAut::TransMTBDD newMtbdd = rewriter(rhs.getMtbdd(*itSt));

			result.setMtbdd(translState, newMtbdd);
		}

		for (StateSet::const_iterator itFst = rhs.GetFinalStates().begin();
			itFst != rhs.GetFinalStates().end(); ++itFst)
		{	// for all final states in the RHS automaton
			result.SetStateFinal(result.safelyTranslateToState(*itFst, *pTranslMapRhs));
		}

		assert(result.isValid());

		return result;
	}
}
