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

using VATA::BDDTreeAut;
using VATA::Parsing::AbstrParser;
using VATA::Util::AutDescription;
using VATA::Util::Convert;


// static class data member
BDDTreeAut::StringToSymbolDict BDDTreeAut::symbolDict_;


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
	assert(haveDisjointStateSets(*this, src));

	states_.insert(states_.end(), src.states_.begin(), src.states_.end());
	finalStates_.insert(finalStates_.end(), src.finalStates_.begin(),
		src.finalStates_.end());

	assert(isValid());
}


void BDDTreeAut::addSimplyTransition(const StateTuple& children, const SymbolType& symbol,
	const StateType& parent)
{
	// Assertions
	assert(isValid());

	UnionApplyFunctor unioner;

	const MTBDD& oldMtbdd = getMtbdd(parent);
	MTBDD addedMtbdd(symbol, StateTupleSet(children), StateTupleSet());
	setMtbdd(parent, unioner(oldMtbdd, addedMtbdd));

	assert(isValid());
}


void BDDTreeAut::loadFromAutDescExplicit(const AutDescription& desc,
	StringToStateDict* pStateDict)
{
	// Assertions
	assert(hasEmptyStateSet());

	for (AutDescription::StateSet::const_iterator itFst =
		desc.finalStates.begin(); itFst != desc.finalStates.end(); ++itFst)
	{	// traverse final states
		if (pStateDict->FindFwd(*itFst) == pStateDict->EndFwd())
		{	// in case the state name is not known
			StateType state = AddState();
			states_.push_back(state);
			finalStates_.push_back(state);
			pStateDict->Insert(std::make_pair(*itFst, state));
		}
	}

	for (AutDescription::TransitionSet::const_iterator itTr =
		desc.transitions.begin(); itTr != desc.transitions.end(); ++itTr)
	{	// traverse the transitions
		const AutDescription::StateTuple& childrenStr = itTr->first;
		const std::string& symbolStr = itTr->second;
		const AutDescription::State& parentStr = itTr->third;

		// translate the parent state
		StateType parent;
		StringToStateDict::ConstIteratorFwd itSt;
		if ((itSt = pStateDict->FindFwd(parentStr)) != pStateDict->EndFwd())
		{	// in case the state name is known
			parent = itSt->second;
		}
		else
		{	// in case there is no translation for the state name
			parent = AddState();
			states_.push_back(parent);
			pStateDict->Insert(std::make_pair(parentStr, parent));
		}

		// translate children
		StateTuple children;
		for (AutDescription::StateTuple::const_iterator itTup = childrenStr.begin();
			itTup != childrenStr.end(); ++itTup)
		{	// for all children states
			StateType child;
			StringToStateDict::ConstIteratorFwd itSt;
			if ((itSt = pStateDict->FindFwd(*itTup)) != pStateDict->EndFwd())
			{	// in case the state name is known
				parent = itSt->second;
			}
			else
			{	// in case there is no translation for the state name
				child = AddState();
				states_.push_back(parent);
				pStateDict->Insert(std::make_pair(*itTup, child));
			}

			children.push_back(child);
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

		addSimplyTransition(children, symbol, parent);
	}

	assert(isValid());
}


void BDDTreeAut::loadFromAutDescSymbolic(const AutDescription& desc,
	StringToStateDict* pStateDict)
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
	if (pStateDict == static_cast<StringToStateDict*>(0))
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


std::string BDDTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer& serializer,
	StringToStateDict* pStateDict)
{
	bool translateStates = false;
	if (pStateDict != static_cast<StringToStateDict*>(0))
	{	// in case there is provided dictionary
		translateStates = true;
	}

	AutDescription desc;

	for (StateVector::const_iterator itSt = states_.begin();
		itSt != states_.end(); ++itSt)
	{	// copy states
		if (translateStates)
		{	// if there is a dictionary, use it
			desc.states.insert(pStateDict->TranslateBwd(*itSt));
		}
		else
		{	// if there is not a dictionary, generate strings
			desc.states.insert(Convert::ToString(*itSt));
		}
	}

	for (StateVector::const_iterator itSt = finalStates_.begin();
		itSt != finalStates_.end(); ++itSt)
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

	return serializer.Serialize(desc);
}


bool BDDTreeAut::haveDisjointStateSets(const BDDTreeAut& lhs,
	const BDDTreeAut& rhs)
{
	// Assertions
	assert(lhs.isValid());
	assert(lhs.isValid());

	assert(false);
}


BDDTreeAut::~BDDTreeAut()
{
	// Assertions
	assert(isValid());

	for (StateVector::iterator itSt = states_.begin();
		itSt != states_.end(); ++itSt)
	{	// release all states
		transTable_->DecrementStateRefCnt(*itSt);
	}
}
