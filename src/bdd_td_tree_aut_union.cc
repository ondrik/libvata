/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union on BDD top-down tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

// Standard library headers
#include <unordered_map>


#include "bdd_td_tree_aut_core.hh"
#include "mtbdd/apply1func.hh"

using VATA::BDDTDTreeAutCore;
using VATA::Util::Convert;



BDDTDTreeAutCore BDDTDTreeAutCore::Union(
	const BDDTDTreeAutCore&     lhs,
	const BDDTDTreeAutCore&     rhs,
	AutBase::StateToStateMap*    pTranslMapLhs,
	AutBase::StateToStateMap*    pTranslMapRhs)
{
	typedef BDDTDTreeAutCore::StateType StateType;
	typedef BDDTDTreeAutCore::StateTuple StateTuple;
	typedef BDDTDTreeAutCore::StateTupleSet StateTupleSet;
	typedef AutBase::StateToStateMap StateToStateMap;
	typedef VATA::Util::TranslatorWeak<typename AutBase::StateToStateMap>
		StateToStateTranslator;

	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class RewriterApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<
		RewriterApplyFunctor,
		StateTupleSet,
		StateTupleSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // data members

		BDDTDTreeAutCore& aut_;
		StateToStateTranslator& trans_;

	public:   // methods

		RewriterApplyFunctor(
			BDDTDTreeAutCore&              aut,
			StateToStateTranslator&        trans) :
			aut_(aut),
			trans_(trans)
		{ }

		StateTupleSet ApplyOperation(const StateTupleSet& value)
		{
			StateTupleSet result;

			// TODO: nicer for
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


	if (BDDTDTreeAutCore::ShareTransTable(lhs, rhs))
	{	// in case the automata share their transition table
		BDDTDTreeAutCore result = lhs;
		result.finalStates_.insert(rhs.finalStates_.begin(), rhs.finalStates_.end());

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

		BDDTDTreeAutCore result;

		StateType stateCnt = 0;
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
		lhs.ReindexStates(result, stateTransLhs);

		StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);
		rhs.ReindexStates(result, stateTransRhs);

		return result;
	}
}
