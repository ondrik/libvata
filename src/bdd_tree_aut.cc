/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Source file for a BDD-based tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_tree_aut.hh>
#include <vata/mtbdd/void_apply2func.hh>

using VATA::BDDTreeAut;
using VATA::Parsing::AbstrParser;
using VATA::Util::AutDescription;
using VATA::Util::Convert;


// static class data member
BDDTreeAut::StringToSymbolDict BDDTreeAut::symbolDict_;


bool BDDTreeAut::isValid() const
{
	if (transTable_.get() == nullptr)
	{	// in case the transition table pointer is bad
		return false;
	}

	// check that final states are a subset of states
	for (auto itFst = finalStates_.cbegin();
		itFst != finalStates_.cend(); ++itFst)
	{
		if (states_.find(*itFst) == states_.end())
		{	// in case the final state is not in the set of states
			return false;
		}
	}

	return true;
}


BDDTreeAut::SymbolType BDDTreeAut::addSymbol()
{
	const size_t MAX_SYMBOL_SIZE = 64;

	static SymbolType nextSymbol(MAX_SYMBOL_SIZE, 0);

	return nextSymbol++;
}


void BDDTreeAut::copyStates(const BDDTreeAut& src)
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

bool BDDTreeAut::isStandAlone() const
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


BDDTreeAut::BDDTreeAut(const BDDTreeAut& aut) :
	states_(),
	finalStates_(),
	transTable_(aut.transTable_)
{
	copyStates(aut);

	// Assertions
	assert(isValid());
}


BDDTreeAut& BDDTreeAut::operator=(const BDDTreeAut& rhs)
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


void BDDTreeAut::AddSimplyTransition(const StateTuple& children,
	const SymbolType& symbol, const StateType& parent)
{
	// Assertions
	assert(isValid());
	assert(isStandAlone());

	UnionApplyFunctor unioner;

	const TransMTBDD& oldMtbdd = getMtbdd(parent);
	TransMTBDD addedMtbdd(symbol, StateTupleSet(children), StateTupleSet());
	setMtbdd(parent, unioner(oldMtbdd, addedMtbdd));

	assert(isValid());
}


void BDDTreeAut::loadFromAutDescExplicit(const AutDescription& desc,
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


void BDDTreeAut::loadFromAutDescSymbolic(const AutDescription& /* desc */,
	StringToStateDict* /* pStateDict */)
{
	// Assertions
	assert(hasEmptyStateSet());

	assert(false);

	assert(isValid());
}


void BDDTreeAut::LoadFromString(AbstrParser& parser, const std::string& str,
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


AutDescription BDDTreeAut::dumpToAutDescExplicit(
	const StringToStateDict* pStateDict) const
{
	GCC_DIAG_OFF(effc++)
	class CondColApplyFunctor :
		public VATA::MTBDDPkg::VoidApply2Functor<CondColApplyFunctor,
		StateTupleSet, bool>
	{
	GCC_DIAG_ON(effc++)

	public:   // data types

		typedef std::list<StateTuple> AccumulatorType;

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

		inline void ApplyOperation(const StateTupleSet& lhs, const bool& rhs)
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
	for (auto itSt = finalStates_.cbegin(); itSt != finalStates_.cend(); ++itSt)
	{	// copy final states
		if (translateStates)
		{	// if there is a dictionary, use it
			desc.finalStates.insert(pStateDict->TranslateBwd(*itSt));
		}
		else
		{	// if there is not a dictionary, generate strings
			desc.finalStates.insert(Convert::ToString(*itSt));
		}
	}

	CondColApplyFunctor collector;

	// copy states, transitions and symbols
	for (auto itSt = states_.cbegin(); itSt != states_.cend(); ++itSt)
	{	// for all states
		const StateType& state = *itSt;
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

		desc.states.insert(stateStr);

		const TransMTBDD& transMtbdd = getMtbdd(state);

		for (auto itSym = symbolDict_.BeginFwd();
			itSym != symbolDict_.EndFwd(); ++itSym)
		{	// iterate over all known symbols
			const std::string& symbol = itSym->first;
			BDD symbolBdd(itSym->second, true, false);

			collector.Clear();
			collector(transMtbdd, symbolBdd);

			for (auto itCol = collector.GetAccumulator().cbegin();
				itCol != collector.GetAccumulator().cend(); ++itCol)
			{	// for each state tuple for which there is a transition
				const StateTuple& tuple = *itCol;

				std::vector<std::string> tupleStr;
				for (auto itTup = tuple.cbegin(); itTup != tuple.cend(); ++itTup)
				{	// for each element in the tuple
					if (translateStates)
					{	// if there is a dictionary, use it
						tupleStr.push_back(pStateDict->TranslateBwd(*itTup));
					}
					else
					{	// if there is not a dictionary, generate strings
						tupleStr.push_back(Convert::ToString(*itTup));
					}
				}

				desc.transitions.insert(AutDescription::Transition(tupleStr, symbol,
					stateStr));
			}
		}
	}

	return desc;
}

AutDescription BDDTreeAut::dumpToAutDescSymbolic(
	const StringToStateDict* /* pStateDict */) const
{
	assert(false);
}

std::string BDDTreeAut::DumpToString(
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


void BDDTreeAut::AddTransition(const StateTuple& children,
	const SymbolType& symbol, const StateType& state)
{
	// Assertions
	assert(isValid());

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


BDDTreeAut::~BDDTreeAut()
{
	// Assertions
	assert(isValid());

	deallocateStates();
}
