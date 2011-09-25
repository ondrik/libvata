/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of a BDD-based bottom-up tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/mtbdd/void_apply2func.hh>


using VATA::BDDBottomUpTreeAut;
using VATA::BDDTopDownTreeAut;
using VATA::Parsing::AbstrParser;
using VATA::Util::AutDescription;
using VATA::Util::Convert;


BDDTopDownTreeAut BDDBottomUpTreeAut::GetTopDownAut(
	StateToStateMap* pTranslMap) const
{
	typedef VATA::Util::TranslatorStrict<AutBase::StateToStateMap>
		StateTranslator;

	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class InverterApplyFunctor :
		public VATA::MTBDDPkg::Apply2Functor<InverterApplyFunctor, StateSet,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	private:  // data members

		const StateType& soughtState_;
		const StateTuple& checkedTuple_;

	public:   // methods

		InverterApplyFunctor(const StateType& soughtState,
			const StateTuple& checkedTuple) :
			soughtState_(soughtState),
			checkedTuple_(checkedTuple)
		{ }

		inline StateTupleSet ApplyOperation(const StateSet& lhs,
			const StateTupleSet& rhs)
		{
			StateTupleSet result = rhs;
			if (lhs.find(soughtState_) != lhs.end())
			{
				result.insert(checkedTuple_);
			}

			return result;
		}
	};

	StateToStateMap translMap;
	if (pTranslMap == nullptr)
	{
		pTranslMap = &translMap;
	}

	BDDTopDownTreeAut result;

	StateTranslator transl(*pTranslMap);
	for (const StateType& fst : GetFinalStates())
	{
		result.finalStates_.insert(transl(fst));
	}

	StateType soughtState;
	StateTuple checkedTuple;
	InverterApplyFunctor invertFunc(soughtState, checkedTuple);

	StateHT states;
	for (auto tupleHandlePair : transTable_->GetTupleMap())
	{	// collect states
		for (const StateType& state : tupleHandlePair.first)
		{
			states.insert(state);
		}
	}

	for (const StateType& state : states)
	{
		soughtState = state;
		StateType translState = transl(state);

		for (auto tupleHandlePair : transTable_->GetTupleMap())
		{
			checkedTuple.clear();
			for (const StateType& tupleState : tupleHandlePair.first)
			{
				checkedTuple.push_back(transl(tupleState));
			}
			assert(checkedTuple.size() == tupleHandlePair.first.size());

			result.SetMtbdd(translState, invertFunc(
				tupleHandlePair.second, result.GetMtbdd(translState)));
		}
	}

	return result;
}
