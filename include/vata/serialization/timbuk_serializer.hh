/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a serializer of automata to Timbuk format.
 *
 *****************************************************************************/

#ifndef _VATA_TIMBUK_SERIALIZER_HH_
#define _VATA_TIMBUK_SERIALIZER_HH_

// VATA headers
#include <vata/vata.hh>

namespace VATA
{
	namespace Serialization
	{
		template <
			class Aut
		>
		class TimbukSerializer;
	}
}

template <
	class Aut
>
class VATA::Serialization::TimbukSerializer
{
public:   // data types

	typedef Aut AutType;

public:   // methods

	std::string Serialize(const AutType& aut)
	{
		std::string result;

		result += "Ops\n";
		result += "Automaton A\n";
		result += "States\n";
		result += "Final States\n";
		result += "Transitions\n";

		return result;
	}

};

#endif
