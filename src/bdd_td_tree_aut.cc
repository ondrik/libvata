/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Source file for a BDD-based top-down tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/mtbdd/void_apply2func.hh>

using VATA::BDDTopDownTreeAut;
using VATA::Parsing::AbstrParser;
using VATA::Util::AutDescription;
using VATA::Util::Convert;


// static class data member
BDDTopDownTreeAut::StringToSymbolDict* BDDTopDownTreeAut::pSymbolDict_ =
	nullptr;

BDDTopDownTreeAut::SymbolType* BDDTopDownTreeAut::pNextBaseSymbol_ =
	nullptr;

bool BDDTopDownTreeAut::isValid() const
{
	if (transTable_.get() == nullptr)
	{	// in case the transition table pointer is bad
		return false;
	}

	// check that final states are a subset of states
	for (auto fst : finalStates_)
	{
		if (states_.find(fst) == states_.end())
		{	// in case the final state is not in the set of states
			return false;
		}
	}

	return true;
}


void BDDTopDownTreeAut::copyStates(const BDDTopDownTreeAut& src)
{
	// Assertions
	assert(isValid());
	assert(src.isValid());
	assert(transTable_ == src.transTable_);

	// copy states and increment their reference counters
	for (auto itSt = src.states_.cbegin(); itSt != src.states_.cend(); ++itSt)
	{
		transTable_->IncrementStateRefCnt(*itSt);
		states_.insert(*itSt);
	}

	// simply copy final states
	finalStates_.insert(src.finalStates_.begin(), src.finalStates_.end());

	assert(isValid());
}

bool BDDTopDownTreeAut::isStandAlone() const
{
	// Assertions
	assert(isValid());

	// TODO: couldn't this be done only on final states? Are there such
	// operations that change reference counters for inner states without
	// changing reference counters of final states?

	// check whether the automaton has some shared states
	for (auto itSt = states_.cbegin(); itSt != states_.cend(); ++itSt)
	{	// iterate through all states
		assert(transTable_->GetStateRefCnt(*itSt) > 0);

		if (transTable_->GetStateRefCnt(*itSt) > 1)
		{	// in case there is some state which is shared
			return false;
		}
	}

	return true;
}


BDDTopDownTreeAut::BDDTopDownTreeAut(const BDDTopDownTreeAut& aut) :
	states_(),
	finalStates_(),
	transTable_(aut.transTable_)
{
	copyStates(aut);

	// Assertions
	assert(isValid());
}


BDDTopDownTreeAut& BDDTopDownTreeAut::operator=(const BDDTopDownTreeAut& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	deallocateStates();
	states_.clear();
	finalStates_.clear();

	// NB: need to copy the transition table before copying states!
	transTable_ = rhs.transTable_;
	copyStates(rhs);

	// Assertions
	assert(isValid());

	return *this;
}


void BDDTopDownTreeAut::AddSimplyTransition(const StateTuple& children,
	SymbolType symbol, const StateType& parent)
{
	// Assertions
	assert(isValid());
	assert(isStandAlone());
	assert(symbol.length() == SYMBOL_VALUE_LENGTH);

	addArityToSymbol(symbol, children.size());
	assert(symbol.length() == SYMBOL_SIZE);

	UnionApplyFunctor unioner;

	const TransMTBDD& oldMtbdd = getMtbdd(parent);
	TransMTBDD addedMtbdd(symbol, StateTupleSet(children), StateTupleSet());
	setMtbdd(parent, unioner(oldMtbdd, addedMtbdd));

	assert(isValid());
}


void BDDTopDownTreeAut::AddTransition(const StateTuple& children,
	const SymbolType& symbol, const StateType& state)
{
	// Assertions
	assert(isValid());
	assert(symbol.length() == SYMBOL_VALUE_LENGTH);

	if (isStandAlone())
	{	// in case the automaton has no shared states
		AddSimplyTransition(children, symbol, state);
	}
	else
	{
		// TODO
		assert(false);
	}

	assert(isValid());
}


BDDTopDownTreeAut::~BDDTopDownTreeAut()
{
	// Assertions
	assert(isValid());

	deallocateStates();
}
