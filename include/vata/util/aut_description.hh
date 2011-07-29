/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for class with automaton description.
 *
 *****************************************************************************/

#ifndef _VATA_AUT_DESCRIPTION_HH_
#define _VATA_AUT_DESCRIPTION_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/triple.hh>

namespace VATA
{
	namespace Util
	{
		struct AutDescription;
	}
}

struct VATA::Util::AutDescription
{
public:   // data types

	typedef std::pair<std::string, unsigned> Symbol;
	typedef std::string State;
	typedef std::vector<State> StateTuple;
	typedef VATA::Util::Triple<StateTuple, std::string, State> Transition;

	typedef std::set<Symbol> SymbolSet;
	typedef std::set<State> StateSet;
	typedef std::set<Transition> TransitionSet;

public:   // data members

	std::string name;
	SymbolSet symbols;
	StateSet states;
	StateSet finalStates;
	TransitionSet transitions;

public:   // methods

	AutDescription()
		: name(),
			symbols(),
			states(),
			finalStates(),
			transitions()
	{ }

	bool operator==(const AutDescription& rhs) const;

	friend std::ostream& operator<<(std::ostream& os, const AutDescription& retType)
	{
		os << "Ops ";
		for (AutDescription::SymbolSet::const_iterator itSymb = retType.symbols.begin();
			itSymb != retType.symbols.end(); ++itSymb)
		{
			os << itSymb->first << ":" << VATA::Util::Convert::ToString(itSymb->second) << " ";
		}

		os << "\n";
		os << "Automaton " << retType.name;

		os << "\n";
		os << "States ";
		for (AutDescription::StateSet::const_iterator itSt = retType.states.begin();
			itSt != retType.states.end(); ++itSt)
		{
			os << *itSt << " ";
		}

		os << "\n";
		os << "Final States ";
		for (AutDescription::StateSet::const_iterator itFst = retType.finalStates.begin();
			itFst != retType.finalStates.end(); ++itFst)
		{
			os << *itFst << " ";
		}

		os << "\n";
		os << "Transitions\n";
		for (AutDescription::TransitionSet::const_iterator itTran = retType.transitions.begin();
			itTran != retType.transitions.end(); ++itTran)
		{
			os << itTran->second;
			if (!(itTran->first.empty()))
			{
				os << "(";
				os << itTran->first[0];
				for (size_t i = 1; i < itTran->first.size(); ++i)
				{
					os << ", ";
					os << itTran->first[i];
				}

				os << ")";
			}

			os << " -> ";
			os << itTran->third;

			os << "\n";
		}

		return os;
	}
};


#endif
