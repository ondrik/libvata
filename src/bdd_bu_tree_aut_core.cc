/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of the core of an BDD-based bottom-up tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/bdd_td_tree_aut.hh>

#include "bdd_bu_tree_aut_core.hh"

using VATA::BDDBUTreeAutCore;
using VATA::BDDTopDownTreeAut;


BDDBUTreeAutCore::BDDBUTreeAutCore(
	const BDDBUTreeAutCore&           aut) :
	transTable_(aut.transTable_),
	finalStates_(aut.finalStates_)
{ }


BDDBUTreeAutCore::BDDBUTreeAutCore(
	BDDBUTreeAutCore&&                aut) :
	transTable_(std::move(aut.transTable_)),
	finalStates_(std::move(aut.finalStates_))
{ }


BDDBUTreeAutCore& BDDBUTreeAutCore::operator=(
	const BDDBUTreeAutCore&         rhs)
{
	if (this != &rhs)
	{
		finalStates_ = rhs.finalStates_;
		transTable_ = rhs.transTable_;
	}

	return *this;
}


BDDBUTreeAutCore& BDDBUTreeAutCore::operator=(
	BDDBUTreeAutCore&&               rhs)
{
	if (this != &rhs)
	{
		finalStates_ = std::move(rhs.finalStates_);
		transTable_ = std::move(rhs.transTable_);
	}

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
		this->SetMtbdd(children, unioner(oldMtbdd, addedMtbdd));
	}
	else
	{	// copy on write
		assert(false);
	}

	UnionApplyFunctor unioner;

	const TransMTBDD& oldMtbdd = this->GetMtbdd(children);
	TransMTBDD addedMtbdd(symbol, StateSet(parent), StateSet());

	// TODO: this could be done better
	this->SetMtbdd(children, unioner(oldMtbdd, addedMtbdd));
}


std::string BDDBUTreeAutCore::DumpToString(
	VATA::Serialization::AbstrSerializer&      serializer) const
{
	return this->DumpToString(serializer,
		[](const StateType& state){return Convert::ToString(state);},
		SymbolBackTranslatorStrict(this->GetSymbolDict().GetReverseMap()));
}


std::string BDDBUTreeAutCore::DumpToString(
	VATA::Serialization::AbstrSerializer&      serializer,
	const StringToStateDict&                   stateDict) const
{
	return this->DumpToString(serializer,
		StateBackTranslatorStrict(stateDict.GetReverseMap()),
		SymbolBackTranslatorStrict(this->GetSymbolDict().GetReverseMap()));
}


BDDBUTreeAutCore::TransMTBDD BDDBUTreeAutCore::ReindexStates(
	BDDBUTreeAutCore&          dstAut,
	StateToStateTranslator&    stateTrans) const
{
	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class RewriteApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<RewriteApplyFunctor,
		StateSet, StateSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // data members

		StateToStateTranslator& trans_;

	public:   // methods

		RewriteApplyFunctor(StateToStateTranslator& trans) :
			trans_(trans)
		{ }

		inline StateSet ApplyOperation(const StateSet& value)
		{
			StateSet result;

			for (const StateType& state : value)
			{ // for every state
				result.insert(trans_(state));
			}

			return result;
		}
	};

	RewriteApplyFunctor rewriter(stateTrans);
	for (auto tupleBddPair : transTable_)
	{
		const StateTuple& oldTuple = tupleBddPair.first;
		StateTuple newTuple;
		for (const StateType& state : oldTuple)
		{
			newTuple.push_back(stateTrans(state));
		}
		assert(newTuple.size() == oldTuple.size());

		dstAut.SetMtbdd(newTuple, rewriter(tupleBddPair.second));
	}

	for (const StateType& fst : this->GetFinalStates())
	{
		dstAut.SetStateFinal(stateTrans(fst));
	}

	TransMTBDD nullaryBdd = rewriter(this->GetMtbdd(StateTuple()));
	dstAut.SetMtbdd(StateTuple(), nullaryBdd);

	return nullaryBdd;
}


BDDBUTreeAutCore BDDBUTreeAutCore::ReindexStates(
	StateToStateTranslator&    stateTrans) const
{
	BDDBUTreeAutCore res;
	this->ReindexStates(res, stateTrans);

	return res;
}


BDDTopDownTreeAut BDDBUTreeAutCore::GetTopDownAut() const
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

	BDDTopDownTreeAut result;

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


std::string BDDBUTreeAutCore::DumpToDot() const
{
	std::vector<const TransMTBDD*> tupleVec;
	for (auto tupleHandlePair : transTable_)
	{
		tupleVec.push_back(&tupleHandlePair.second);
	}

	return TransMTBDD::DumpToDot(tupleVec);
}


void BDDBUTreeAutCore::LoadFromAutDesc(
	const AutDescription&         desc,
	StringToStateDict&            stateDict)
{
	typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
		StateTranslator;
	typedef VATA::Util::TranslatorWeak<StringToSymbolDict>
		SymbolTranslator;

	StateType stateCnt = 0;

	this->LoadFromAutDesc(desc,
		StateTranslator(stateDict,
			[&stateCnt](const std::string&){return stateCnt++;}),
		SymbolTranslator(GetSymbolDict(),
			[this](const std::string&){return AddSymbol();}));
}
