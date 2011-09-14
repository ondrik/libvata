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
BDDBottomUpTreeAut::StringToSymbolDict BDDBottomUpTreeAut::symbolDict_;

BDDBottomUpTreeAut::SymbolType
	BDDBottomUpTreeAut::nextSymbol_(SYMBOL_SIZE, 0);


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


void BDDBottomUpTreeAut::loadFromAutDescExplicit(const AutDescription& desc,
	StringToStateDict* pStateDict)
{
	// Assertions
	assert(hasEmptyStateSet());
	assert(pStateDict != nullptr);

	for (auto itFst = desc.finalStates.cbegin();
		itFst != desc.finalStates.cend(); ++itFst)
	{	// traverse final states
		finalStates_.insert(safelyTranslateToState(*itFst, *pStateDict));
	}

	assert(isValid());

	for (auto itTr = desc.transitions.cbegin();
		itTr != desc.transitions.cend(); ++itTr)
	{	// traverse the transitions
		const AutDescription::StateTuple& childrenStr = itTr->first;
		const std::string& symbolStr = itTr->second;
		const AutDescription::State& parentStr = itTr->third;

		// translate the parent state
		StateType parent = safelyTranslateToState(parentStr, *pStateDict);

		// translate children
		StateTuple children;
		for (auto itTup = childrenStr.cbegin();
			itTup != childrenStr.cend(); ++itTup)
		{	// for all children states
			children.push_back(safelyTranslateToState(*itTup, *pStateDict));
		}

		// translate the symbol
		SymbolType symbol(0);
		StringToSymbolDict::ConstIteratorFwd itSym;
		if ((itSym = symbolDict_.FindFwd(symbolStr)) != symbolDict_.EndFwd())
		{	// in case the state name is known
			symbol = itSym->second;
		}
		else
		{	// in case there is no translation for the state name
			symbol = addSymbol();
			symbolDict_.Insert(std::make_pair(symbolStr, symbol));
		}

		AddSimplyTransition(children, symbol, parent);
		assert(isValid());
	}

	assert(isValid());
}


void BDDBottomUpTreeAut::loadFromAutDescSymbolic(const AutDescription&/* desc */,
	StringToStateDict* /* pStateDict */)
{
	// Assertions
	assert(hasEmptyStateSet());

	assert(false);

	assert(isValid());
}


void BDDBottomUpTreeAut::LoadFromString(AbstrParser& parser, const std::string& str,
	StringToStateDict* pStateDict, const std::string& params)
{
	// Assertions
	assert(hasEmptyStateSet());

	bool delStateDict = false;
	if (pStateDict == nullptr)
	{	// in case we do not wish to retain the string-to-state dictionary
		delStateDict = true;
		pStateDict = new StringToStateDict();
	}

	if (params == "symbolic")
	{
		loadFromAutDescSymbolic(parser.ParseString(str), pStateDict);
	}
	else
	{
		loadFromAutDescExplicit(parser.ParseString(str), pStateDict);
	}

	if (delStateDict)
	{	// in case we do not need the dictionary
		delete pStateDict;
	}

	assert(isValid());
}


AutDescription BDDBottomUpTreeAut::dumpToAutDescExplicit(
	const StringToStateDict* pStateDict) const
{
	GCC_DIAG_OFF(effc++)
	class CondColApplyFunctor :
		public VATA::MTBDDPkg::VoidApply2Functor<CondColApplyFunctor,
		StateSet, bool>
	{
	GCC_DIAG_ON(effc++)

	public:   // data types

		typedef std::list<StateType> AccumulatorType;

	private:  // data members

		AccumulatorType accumulator_;

	public:

		CondColApplyFunctor() :
			accumulator_()
		{ }

		inline const AccumulatorType& GetAccumulator() const
		{
			return accumulator_;
		}

		inline void Clear()
		{
			accumulator_.clear();
		}

		inline void ApplyOperation(const StateSet& lhs, const bool& rhs)
		{
			if (rhs)
			{
				accumulator_.insert(accumulator_.end(), lhs.begin(), lhs.end());
			}
		}
	};

	bool translateStates = false;
	if (pStateDict != nullptr)
	{	// in case there is provided dictionary
		translateStates = true;
	}

	AutDescription desc;

	// copy final states
	for (auto fst : finalStates_)
	{	// copy final states
		if (translateStates)
		{	// if there is a dictionary, use it
			desc.finalStates.insert(pStateDict->TranslateBwd(fst));
		}
		else
		{	// if there is not a dictionary, generate strings
			desc.finalStates.insert(Convert::ToString(fst));
		}
	}

	CondColApplyFunctor collector;

	// copy states, transitions and symbols
	for (auto tupleHandlePair : mtbddMap_)
	{	// for all states
		const StateTuple& children = tupleHandlePair.first;

		std::vector<std::string> tupleStr;

		for (auto state : children)
		{
			std::string stateStr;

			// copy the state
			if (translateStates)
			{	// if there is a dictionary, use it
				stateStr = pStateDict->TranslateBwd(state);
			}
			else
			{	// if there is not a dictionary, generate strings
				stateStr = Convert::ToString(state);
			}

			tupleStr.push_back(stateStr);
			desc.states.insert(stateStr);
		}

		const TransMTBDD& transMtbdd = getMtbdd(tupleHandlePair.second);

		for (auto itSym = symbolDict_.BeginFwd();
			itSym != symbolDict_.EndFwd(); ++itSym)
		{	// iterate over all known symbols
			const std::string& symbol = itSym->first;
			BDD symbolBdd(itSym->second, true, false);

			collector.Clear();
			collector(transMtbdd, symbolBdd);

			for (auto state : collector.GetAccumulator())
			{	// for each state tuple for which there is a transition
				std::string stateStr;

				// copy the state
				if (translateStates)
				{	// if there is a dictionary, use it
					stateStr = pStateDict->TranslateBwd(state);
				}
				else
				{	// if there is not a dictionary, generate strings
					stateStr = Convert::ToString(state);
				}

				desc.transitions.insert(AutDescription::Transition(tupleStr, symbol,
					stateStr));
			}
		}
	}

	return desc;
}


AutDescription BDDBottomUpTreeAut::dumpToAutDescSymbolic(
	const StringToStateDict* /* pStateDict */) const
{
	assert(false);
}


std::string BDDBottomUpTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer& serializer,
	const StringToStateDict* pStateDict, const std::string& params) const
{
	AutDescription desc;
	if (params == "symbolic")
	{
		desc = dumpToAutDescSymbolic(pStateDict);
	}
	else
	{
		desc = dumpToAutDescExplicit(pStateDict);
	}

	return serializer.Serialize(desc);
}


BDDBottomUpTreeAut::~BDDBottomUpTreeAut()
{
	// Assertions
	assert(isValid() || (transTable_ == nullptr));

	if (transTable_ != nullptr)
	{
		deallocateTuples();
		assert(mtbddMap_.empty());
		transTable_->DecrementHandleRefCnt(defaultTrFuncHandle_);
	}
}
