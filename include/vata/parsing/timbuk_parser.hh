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
#include <vata/util/convert.hh>
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

		ReturnType()
			: name(),
				symbols(),
				states(),
				finalStates(),
				transitions()
		{ }

		bool operator==(const ReturnType& rhs) const;

		friend std::ostream& operator<<(std::ostream& os, const ReturnType& retType)
		{
			typedef TimbukParser::ReturnType ReturnType;

			os << "Ops ";
			for (ReturnType::SymbolSet::const_iterator itSymb = retType.symbols.begin();
				itSymb != retType.symbols.end(); ++itSymb)
			{
				os << itSymb->first << ":" << VATA::Util::Convert::ToString(itSymb->second) << " ";
			}

			os << "\n";
			os << "Automaton " << retType.name;

			os << "\n";
			os << "States ";
			for (ReturnType::StateSet::const_iterator itSt = retType.states.begin();
				itSt != retType.states.end(); ++itSt)
			{
				os << *itSt << " ";
			}

			os << "\n";
			os << "Final States ";
			for (ReturnType::StateSet::const_iterator itFst = retType.finalStates.begin();
				itFst != retType.finalStates.end(); ++itFst)
			{
				os << *itFst << " ";
			}

			os << "\n";
			os << "Transitions\n";
			for (ReturnType::TransitionSet::const_iterator itTran = retType.transitions.begin();
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

public:   // methods

	ReturnType ParseString(const std::string& str);

};

#endif
