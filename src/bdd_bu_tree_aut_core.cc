/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of the core of an BDD-based bottom-up tree automaton.
 *
 *****************************************************************************/

#include "bdd_bu_tree_aut_core.hh"
#include "bdd_td_tree_aut_core.hh"


using VATA::BDDBUTreeAutCore;
using VATA::BDDTDTreeAutCore;

using VATA::Util::Convert;


BDDBUTreeAutCore& BDDBUTreeAutCore::operator=(
	const BDDBUTreeAutCore&         rhs)
{
	this->SymbolicTreeAutBaseCore::operator=(rhs);

	if (this != &rhs)
	{
		finalStates_  = rhs.finalStates_;
		transTable_   = rhs.transTable_;
	}

	return *this;
}


BDDBUTreeAutCore& BDDBUTreeAutCore::operator=(
	BDDBUTreeAutCore&&               rhs)
{
	assert(this != &rhs);

	this->SymbolicTreeAutBaseCore::operator=(std::move(rhs));

	finalStates_  = std::move(rhs.finalStates_);
	transTable_   = std::move(rhs.transTable_);

	return *this;
}


void BDDBUTreeAutCore::AddTransition(
	const StateTuple&      children,
	SymbolType             symbol,
	const StateType&       parent)
{
	// Assertions
	assert(symbol.length() == SYMBOL_SIZE);

	if (transTable_.unique())
	{
		UnionApplyFunctor unioner;

		const TransMTBDD& oldMtbdd = this->GetMtbdd(children);
		TransMTBDD addedMtbdd(symbol, StateSet(parent), StateSet());
		// TODO: this might maybe be done better
		this->SetMtbdd(children, unioner(oldMtbdd, addedMtbdd));
	}
	else
	{	// copy on write
		assert(false);
	}
}


BDDBUTreeAutCore::TransMTBDD BDDBUTreeAutCore::ReindexStates(
	BDDBUTreeAutCore&          dstAut,
	StateToStateTranslWeak&    stateTransl) const
{
	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class RewriteApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<RewriteApplyFunctor,
		StateSet, StateSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // data members

		StateToStateTranslWeak& transl_;

	public:   // methods

		RewriteApplyFunctor(StateToStateTranslWeak& transl) :
			transl_(transl)
		{ }

		StateSet ApplyOperation(const StateSet& value)
		{
			StateSet result;

			for (const StateType& state : value)
			{ // for every state
				result.insert(transl_(state));
			}

			return result;
		}
	};

	RewriteApplyFunctor rewriter(stateTransl);
	for (auto tupleBddPair : transTable_)
	{
		const StateTuple& oldTuple = tupleBddPair.first;
		StateTuple newTuple;
		for (const StateType& state : oldTuple)
		{
			newTuple.push_back(stateTransl(state));
		}
		assert(newTuple.size() == oldTuple.size());

		dstAut.SetMtbdd(newTuple, rewriter(tupleBddPair.second));
	}

	for (const StateType& fst : this->GetFinalStates())
	{
		dstAut.SetStateFinal(stateTransl(fst));
	}

	TransMTBDD nullaryBdd = rewriter(this->GetMtbdd(StateTuple()));
	dstAut.SetMtbdd(StateTuple(), nullaryBdd);

	return nullaryBdd;
}


BDDBUTreeAutCore BDDBUTreeAutCore::ReindexStates(
	StateToStateTranslWeak&    stateTransl) const
{
	BDDBUTreeAutCore res;
	this->ReindexStates(res, stateTransl);

	return res;
}


BDDTDTreeAutCore BDDBUTreeAutCore::GetTopDownAut() const
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

		InverterApplyFunctor(
			const StateType&        soughtState,
			const StateTuple&       checkedTuple) :
			soughtState_(soughtState),
			checkedTuple_(checkedTuple)
		{ }

		inline StateTupleSet ApplyOperation(
			const StateSet&         lhs,
			const StateTupleSet&    rhs)
		{
			StateTupleSet result = rhs;
			if (lhs.find(soughtState_) != lhs.end())
			{
				result.insert(checkedTuple_);
			}

			return result;
		}
	};

	BDDTDTreeAutCore result(this->GetAlphabet());

	StateType soughtState;
	StateTuple checkedTuple;
	InverterApplyFunctor invertFunc(soughtState, checkedTuple);

	StateHT states;
	for (const StateType& fst : this->GetFinalStates())
	{
		result.SetStateFinal(fst);
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
			SymbolType prefix(BDDTDTreeAutCore::SYMBOL_ARITY_LENGTH,
				checkedTuple.size());
			TransMTBDD extendedBdd = tupleBddPair.second.ExtendWith(prefix,
				Symbolic::SYMBOL_SIZE);

			result.SetMtbdd(state, invertFunc(
				extendedBdd, result.GetMtbdd(state)));
		}
	}

	return result;
}


std::string BDDBUTreeAutCore::DumpToDot() const
{
	std::vector<const TransMTBDD*> tupleVec;
	for (auto tupleHandlePair : transTable_)
	{
		tupleVec.push_back(&tupleHandlePair.second);
	}

	return TransMTBDD::DumpToDot(tupleVec);
}
