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
	typedef VATA::Util::TranslatorWeak<typename AutBase::StateToStateMap>
		StateToStateTranslator;

	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class RewriterApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<RewriterApplyFunctor,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // data members

		BDDTopDownTreeAut& aut_;
		StateToStateTranslator& trans_;

	public:   // methods

		RewriterApplyFunctor(BDDTopDownTreeAut& aut, StateToStateTranslator& trans) :
			aut_(aut),
			trans_(trans)
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
					resTuple.push_back(trans_(*itTup));
				}

				result.insert(resTuple);
			}

			return result;
		}
	};


	if (lhs.GetTransTable() == rhs.GetTransTable())
	{	// in case the automata share their transition table
		BDDTopDownTreeAut result = lhs;
		result.finalStates_.insert(rhs.finalStates_.begin(), rhs.finalStates_.end());

		assert(result.isValid());
		return result;
	}
	else
	{	// in case the automata have distinct transition tables
		StateToStateMap translMapLhs;
		if (pTranslMapLhs == nullptr)
		{	// copy states
			pTranslMapLhs = &translMapLhs;
		}

		StateToStateMap translMapRhs;
		if (pTranslMapRhs == nullptr)
		{
			pTranslMapRhs = &translMapRhs;
		}

		BDDTopDownTreeAut result;

		StateType stateCnt = 0;
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
		lhs.ReindexStates(result, stateTransLhs);
		for (const StateType& lhsFst : lhs.GetFinalStates())
		{
			result.SetStateFinal(stateTransLhs(lhsFst));
		}

		StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);
		rhs.ReindexStates(result, stateTransRhs);
		for (const StateType& rhsFst : rhs.GetFinalStates())
		{
			result.SetStateFinal(stateTransRhs(rhsFst));
		}

		assert(result.isValid());
		return result;
	}
}
