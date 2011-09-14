/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union on BDD bottom-up tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut_op.hh>
#include <vata/mtbdd/apply1func.hh>

using VATA::BDDBottomUpTreeAut;
using VATA::Util::Convert;

// Standard library headers
#include <unordered_map>


template <>
BDDBottomUpTreeAut VATA::Union<BDDBottomUpTreeAut>(const BDDBottomUpTreeAut& lhs,
	const BDDBottomUpTreeAut& rhs, AutBase::StateToStateMap* pTranslMap)
{
	// Assertions
	assert(lhs.isValid());
	assert(rhs.isValid());

	typedef BDDBottomUpTreeAut::StateType StateType;
	typedef BDDBottomUpTreeAut::StateSet StateSet;
	typedef BDDBottomUpTreeAut::StateTuple StateTuple;
	typedef BDDBottomUpTreeAut::TransMTBDD TransMTBDD;
	typedef AutBase::StateToStateMap StateToStateMap;

	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class RewriterApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<RewriterApplyFunctor,
		StateSet, StateSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // data members

		BDDBottomUpTreeAut& aut_;
		StateToStateMap& dict_;

	public:   // methods

		RewriterApplyFunctor(BDDBottomUpTreeAut& aut, StateToStateMap& dict) :
			aut_(aut),
			dict_(dict)
		{ }

		inline StateSet ApplyOperation(const StateSet& value)
		{
			StateSet result;

			for (const StateType& state : value)
			{ // for every state
				result.insert(aut_.safelyTranslateToState(state, dict_));
			}

			return result;
		}
	};

	BDDBottomUpTreeAut::UnionApplyFunctor unionFunc;

	if (lhs.GetTransTable() == rhs.GetTransTable())
	{	// in case the automata share their transition table
		BDDBottomUpTreeAut result = lhs;

		StateTuple tuple;
		const TransMTBDD& lhsMTBDD = lhs.getMtbdd(tuple);
		const TransMTBDD& rhsMTBDD = rhs.getMtbdd(tuple);

		result.copyStates(rhs);

		result.setMtbdd(tuple, unionFunc(lhsMTBDD, rhsMTBDD));

		assert(result.isValid());
		return result;
	}
	else
	{	// in case the automata have distinct transition tables

		// start by copying the LHS automaton
		BDDBottomUpTreeAut result = lhs;

		bool deleteTranslMap = false;
		if (pTranslMap == nullptr)
		{
			pTranslMap = new StateToStateMap;
			deleteTranslMap = true;
		}

		StateTuple tuple;
		const TransMTBDD& lhsMTBDD = result.getMtbdd(tuple);

		RewriterApplyFunctor rewriter(result, *pTranslMap);

		for (auto tupleHandlePair : rhs.GetTuples())
		{	// for all states in the RHS automaton
			StateTuple translTuple;
			for (const StateType& state : tupleHandlePair.first)
			{
				translTuple.push_back(result.safelyTranslateToState(state, *pTranslMap));
			}

			BDDBottomUpTreeAut::TransMTBDD newMtbdd =
				rewriter(rhs.getMtbdd(tupleHandlePair.second));

			result.setMtbdd(translTuple, newMtbdd);
		}

		for (auto finalState : rhs.GetFinalStates())
		{	// for all final states in the RHS automaton
			result.SetStateFinal(result.safelyTranslateToState(finalState, *pTranslMap));
		}

		const TransMTBDD& rhsMTBDD = result.getMtbdd(tuple);
		result.setMtbdd(tuple, unionFunc(lhsMTBDD, rhsMTBDD));

		if (deleteTranslMap)
		{
			delete pTranslMap;
		}

		assert(result.isValid());

		return result;
	}
}
