/*****************************************************************************
 *  VATA Tree Automata Library
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


BDDTopDownTreeAut BDDBottomUpTreeAut::GetTopDownAut() const
{
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

	BDDTopDownTreeAut result;

	StateType soughtState;
	StateTuple checkedTuple;
	InverterApplyFunctor invertFunc(soughtState, checkedTuple);

	StateHT states;
	for (const StateType& fst : GetFinalStates())
	{
		result.finalStates_.insert(fst);
		states.insert(fst);
	}

	for (auto tupleBddPair : transTable_)
	{	// collect states
		for (const StateType& state : tupleBddPair.first)
		{
			states.insert(state);
		}
	}

	for (const StateType& state : states)
	{
		soughtState = state;

		for (auto tupleBddPair : transTable_)
		{
			checkedTuple = tupleBddPair.first;

			// TODO: it is necessary to somehow process arity
			SymbolType prefix(BDDTopDownTreeAut::SYMBOL_ARITY_LENGTH,
				checkedTuple.size());
			TransMTBDD extendedBdd = tupleBddPair.second.ExtendWith(prefix,
				VATA::SymbolicAutBase::SYMBOL_SIZE);

			result.SetMtbdd(state, invertFunc(
				extendedBdd, result.GetMtbdd(state)));
		}
	}

	return result;
}
