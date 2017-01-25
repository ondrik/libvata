/*****************************************************************************
 *  VATA Tree Automata Library
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

	typedef std::pair<std::string, int> Symbol;
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

	AutDescription() :
		name(),
		symbols(),
		states(),
		finalStates(),
		transitions()
	{ }

	/**
	 * @brief  Relaxed equivalence check
	 *
	 * Checks whether the final states and transitions of two automata descriptions match.
	 */
	bool operator==(const AutDescription& rhs) const
	{
		return (finalStates == rhs.finalStates) && (transitions == rhs.transitions);
	}

	/**
	 * @brief  Strict equivalence check
	 *
	 * Checks whether all components of two automata descriptions match.
	 */
	bool StrictlyEqual(const AutDescription& rhs) const
	{
		return
			(name == rhs.name) &&
			(symbols == rhs.symbols) &&
			(states == rhs.states) &&
			(finalStates == rhs.finalStates) &&
			(transitions == rhs.transitions);
	}

	std::string ToString() const
	{
		std::string result;
		result += "name: " + name + "\n";
		result += "symbols: " + Convert::ToString(symbols) + "\n";
		result += "states: " + Convert::ToString(states) + "\n";
		result += "final states: " + Convert::ToString(finalStates) + "\n";
		result += "transitions: \n";
		for (auto trans : transitions)
		{
			result += Convert::ToString(trans) + "\n";
		}

		return result;
	}
};

#endif
