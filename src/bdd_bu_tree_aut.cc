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
using VATA::Parsing::AbstrParser;
using VATA::Util::AutDescription;
using VATA::Util::Convert;


// static class data member
BDDBottomUpTreeAut::StringToSymbolDict* BDDBottomUpTreeAut::pSymbolDict_ =
	nullptr;

BDDBottomUpTreeAut::SymbolType* BDDBottomUpTreeAut::pNextSymbol_ =
	nullptr;

bool BDDBottomUpTreeAut::isValid() const
{
	if (transTable_.get() == nullptr)
	{	// in case the transition table pointer is bad
		return false;
	}

	return true;
}

bool BDDBottomUpTreeAut::isStandAlone() const
{
	// Assertions
	assert(isValid());

	// TODO: couldn't this be done only on the empty tuple?

	// check whether the automaton has some shared states
	for (auto tupleHandlePair : mtbddMap_)
	{	// iterate through all states
		assert(transTable_->GetHandleRefCnt(tupleHandlePair.second) > 0);

		if (transTable_->GetHandleRefCnt(tupleHandlePair.second) > 1)
		{	// in case there is some state which is shared
			return false;
		}
	}

	return true;
}


void BDDBottomUpTreeAut::copyStates(const BDDBottomUpTreeAut& src)
{
	// Assertions
	assert(isValid());
	assert(src.isValid());
	assert(transTable_ == src.transTable_);

	// copy tuples and increment their reference counters
	for (auto childrenHandlePair : src.mtbddMap_)
	{
		typename TupleToMTBDDMap::iterator it;
		if ((it = mtbddMap_.find(childrenHandlePair.first)) != mtbddMap_.end())
		{
			if ((*it).second != childrenHandlePair.second)
			{	// in case they are not the same
				transTable_->IncrementHandleRefCnt(childrenHandlePair.second);
				transTable_->DecrementHandleRefCnt((*it).second);
				(*it).second = childrenHandlePair.second;
			}
		}
		else
		{
			transTable_->IncrementHandleRefCnt(childrenHandlePair.second);
			mtbddMap_.insert(childrenHandlePair);
		}
	}

	StateTuple tuple;

	// simply copy final states
	finalStates_.insert(src.finalStates_.begin(), src.finalStates_.end());

	assert(isValid());
}


BDDBottomUpTreeAut& BDDBottomUpTreeAut::operator=(const BDDBottomUpTreeAut& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	deallocateTuples();
	mtbddMap_.clear();
	finalStates_.clear();

	// NB: need to copy the transition table before copying states!
	transTable_ = rhs.transTable_;
	copyStates(rhs);

	// Assertions
	assert(isValid());

	return *this;
}

void BDDBottomUpTreeAut::AddSimplyTransition(const StateTuple& children,
	SymbolType symbol, const StateType& parent)
{
	// Assertions
	assert(isValid());
	assert(isStandAlone());
	assert(symbol.length() == SYMBOL_SIZE);

	UnionApplyFunctor unioner;

	MTBDDHandle handle;

	TupleToMTBDDMap::iterator it;
	if ((it = mtbddMap_.find(children)) == mtbddMap_.end())
	{
		handle = transTable_->AddHandle();
		mtbddMap_.insert(std::make_pair(children, handle));
	}
	else
	{
		handle = getMtbddHandle(children);
	}

	const TransMTBDD& oldMtbdd = getMtbdd(handle);
	TransMTBDD addedMtbdd(symbol, StateSet(parent), StateSet());
	setMtbdd(handle, unioner(oldMtbdd, addedMtbdd));

	assert(isValid());
}


BDDBottomUpTreeAut::~BDDBottomUpTreeAut()
{
	// Assertions
	assert(isValid() || (transTable_ == nullptr));

	if (transTable_ != nullptr)
	{
		deallocateTuples();
		assert(mtbddMap_.empty());
	}
}
