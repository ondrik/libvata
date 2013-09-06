/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of the core of a BDD-based top-down tree automaton.
 *
 *****************************************************************************/


#include "bdd_td_tree_aut_core.hh"


using VATA::BDDTDTreeAutCore;
using VATA::Util::Convert;


BDDTDTreeAutCore::BDDTDTreeAutCore(AlphabetType& alphabet) :
	SymbolicTreeAutBaseCore(alphabet),
	finalStates_(),
	transTable_(new TransTable)
{ }


BDDTDTreeAutCore::BDDTDTreeAutCore(
	TransTablePtr                    transTable,
	AlphabetType&                    alphabet) :
	SymbolicTreeAutBaseCore(alphabet),
	finalStates_(),
	transTable_(transTable)
{ }


BDDTDTreeAutCore::BDDTDTreeAutCore(
	const BDDTDTreeAutCore&           aut) :
	SymbolicTreeAutBaseCore(aut),
	finalStates_(aut.finalStates_),
	transTable_(aut.transTable_)
{ }


BDDTDTreeAutCore::BDDTDTreeAutCore(
	BDDTDTreeAutCore&&                aut) :
	SymbolicTreeAutBaseCore(aut),
	finalStates_(std::move(aut.finalStates_)),
	transTable_(std::move(aut.transTable_))
{ }


BDDTDTreeAutCore& BDDTDTreeAutCore::operator=(
	const BDDTDTreeAutCore&           rhs)
{
	SymbolicTreeAutBaseCore::operator=(rhs);

	if (this != &rhs)
	{
		transTable_   = rhs.transTable_;
		finalStates_  = rhs.finalStates_;
	}

	return *this;
}


BDDTDTreeAutCore& BDDTDTreeAutCore::operator=(
	BDDTDTreeAutCore&&               rhs)
{
	assert(this != &rhs);

	SymbolicTreeAutBaseCore::operator=(std::move(rhs));

	transTable_   = std::move(rhs.transTable_);
	finalStates_  = std::move(rhs.finalStates_);

	return *this;
}


void BDDTDTreeAutCore::AddTransition(
	const StateTuple&       children,
	const SymbolType&       symbol,
	const StateType&        parent)
{
	// Assertions
	assert(symbol.length() == SYMBOL_SIZE);

	SymbolType newSymbol = symbol;
	addArityToSymbol(newSymbol, children.size());
	assert(newSymbol.length() == SYMBOL_TOTAL_SIZE);

	if (transTable_.unique())
	{
		UnionApplyFunctor unioner;

		const TransMTBDD& oldMtbdd = GetMtbdd(parent);
		TransMTBDD addedMtbdd(newSymbol, StateTupleSet(children), StateTupleSet());
		SetMtbdd(parent, unioner(oldMtbdd, addedMtbdd));
	}
	else
	{	// copy on write
		assert(false);              // fail gracefully
	}
}


std::string BDDTDTreeAutCore::DumpToDot() const
{
	std::vector<const TransMTBDD*> stateVec;
	for (auto stateBddPair : GetStates())
	{
		stateVec.push_back(&GetMtbdd(stateBddPair.first));
	}

	return TransMTBDD::DumpToDot(stateVec);
}


void BDDTDTreeAutCore::ReindexStates(
	BDDTDTreeAutCore&           dstAut,
	StateToStateTranslWeak&     stateTrans) const
{
	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class RewriteApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<RewriteApplyFunctor,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // data members

		StateToStateTranslWeak& trans_;

	public:   // methods

		RewriteApplyFunctor(StateToStateTranslWeak& trans) :
			trans_(trans)
		{ }

		StateTupleSet ApplyOperation(const StateTupleSet& value)
		{
			StateTupleSet result;

			for (const StateTuple& tuple : value)
			{ // for every tuple
				StateTuple resTuple;
				for (StateTuple::const_iterator itTup = tuple.begin();
					itTup != tuple.end(); ++itTup)
				{
					resTuple.push_back(trans_(*itTup));
				}

				result.insert(resTuple);
			}

			return result;
		}
	};

	RewriteApplyFunctor rewriter(stateTrans);
	for (auto stateBddPair : this->GetStates())
	{
		StateType newState = stateTrans(stateBddPair.first);
		dstAut.SetMtbdd(newState, rewriter(stateBddPair.second));
	}

	for (const StateType& fst : this->GetFinalStates())
	{
		dstAut.SetStateFinal(stateTrans(fst));
	}
}
