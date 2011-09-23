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

	// simply copy states and final states
	states_.insert(src.finalStates_.begin(), src.finalStates_.end());
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
	states_.clear();
	finalStates_.clear();

	// NB: need to copy the transition table before copying states!
	transTable_ = rhs.transTable_;
	copyStates(rhs);

	defaultTrFuncHandle_ = rhs.defaultTrFuncHandle_;
	transTable_->IncrementHandleRefCnt(defaultTrFuncHandle_);

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

	for (const StateType& state : GetStates())
	{
		StateType newState = result.AddState();
		pTranslMap->insert(std::make_pair(state, newState));
	}

	StateTranslator transl(*pTranslMap);
	for (const StateType& fst : GetFinalStates())
	{
		result.finalStates_.insert(transl(fst));
	}

	StateType soughtState;
	StateTuple checkedTuple;
	InverterApplyFunctor invertFunc(soughtState, checkedTuple);


	for (const StateType& state : GetStates())
	{
		soughtState = state;
		StateType translState = transl(state);

		for (auto tupleHandlePair : GetTuples())
		{
			checkedTuple.clear();
			for (const StateType& tupleState : tupleHandlePair.first)
			{
				checkedTuple.push_back(transl(tupleState));
			}
			assert(checkedTuple.size() == tupleHandlePair.first.size());

			result.setMtbdd(translState, invertFunc(
				getMtbdd(tupleHandlePair.second), result.getMtbdd(translState)));
		}
	}

	return result;
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
