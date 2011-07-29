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


void BDDTreeAut::LoadFromString(AbstrParser& parser, const std::string& str,
	StringToStateDict* pStateDict)
{
	// Assertions
	assert(hasEmptyStateSet());

	bool delStateDict = false;
	if (pStateDict == static_cast<StringToStateDict*>(0))
	{	// in case we do not wish to retain the string-to-state dictionary
		delStateDict = true;
		pStateDict = new StringToStateDict();
	}

	AutDescription desc = parser.ParseString(str);

	for (AutDescription::TransitionSet::const_iterator itTr =
		desc.transitions.begin(); itTr != desc.transitions.end(); ++itTr)
	{	// traverse the transitions
		const std::vector<std::string>& children = itTr->first;
		const std::string& symbolStr = itTr->second;
		const std::string& parentStr = itTr->third;

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
