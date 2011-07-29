/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation file for a serializer of automata to Timbuk format.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/serialization/timbuk_serializer.hh>

using VATA::Serialization::TimbukSerializer;


std::string TimbukSerializer::Serialize(const AutDescription& desc)
{
	std::string result;

	result += "Ops ";
	for (AutDescription::SymbolSet::const_iterator itSymb = desc.symbols.begin();
		itSymb != desc.symbols.end(); ++itSymb)
	{
		result += itSymb->first + ":" + VATA::Util::Convert::ToString(itSymb->second) + " ";
	}

	result += "\n";
	result += "Automaton " + (desc.name.empty()? "anonymous" : desc.name);

	result += "\n";
	result += "States ";
	for (AutDescription::StateSet::const_iterator itSt = desc.states.begin();
		itSt != desc.states.end(); ++itSt)
	{
		result += *itSt + " ";
	}

	result += "\n";
	result += "Final States ";
	for (AutDescription::StateSet::const_iterator itFst = desc.finalStates.begin();
		itFst != desc.finalStates.end(); ++itFst)
	{
		result += *itFst + " ";
	}

	result += "\n";
	result += "Transitions\n";
	for (AutDescription::TransitionSet::const_iterator itTran = desc.transitions.begin();
		itTran != desc.transitions.end(); ++itTran)
	{
		result += itTran->second;
		if (!(itTran->first.empty()))
		{
			result += "(";
			result += itTran->first[0];
			for (size_t i = 1; i < itTran->first.size(); ++i)
			{
				result += ", ";
				result += itTran->first[i];
			}

			result += ")";
		}

		result += " -> ";
		result += itTran->third;

		result += "\n";
	}

	return result;
}
