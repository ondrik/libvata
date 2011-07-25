/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Parser of Timbuk format.
 *
 *****************************************************************************/

#ifndef _VATA_TIMBUK_PARSER_HH_
#define _VATA_TIMBUK_PARSER_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/triple.hh>

namespace VATA
{
	namespace Parsing
	{
		class TimbukParser;
	}
}





class VATA::Parsing::TimbukParser
{
public:   // data types

	struct ReturnType
	{
	public:   // data types

		typedef std::pair<std::string, unsigned> Symbol;
		typedef std::string State;
		typedef std::vector<State> StateTuple;
		typedef VATA::Util::Triple<StateTuple, Symbol, State> Transition;

		typedef std::vector<Symbol> SymbolList;
		typedef std::vector<State> StateList;
		typedef std::vector<Transition> TransitionList;

	public:   // data members

		std::string name;
		SymbolList symbols;
		StateList states;
		StateList finalStates;
		TransitionList transitions;

	public:   // methods

		ReturnType()
			: name(),
				symbols(),
				states(),
				finalStates(),
				transitions()
		{ }
	};

public:   // methods

	ReturnType ParseString(const std::string& str);

};

#endif
