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

namespace VATA
{
	namespace Parsing
	{
		template <
			class Aut
		>
		class TimbukParser;
	}
}


template <
	class Aut
>
class VATA::Parsing::TimbukParser
{
public:   // data types

	typedef Aut AutType;

public:   // methods

	AutType& LoadAut(AutType& aut, const std::string& str)
	{
		// Assertions
		assert(aut.HasEmptyStateSet());

		return aut;
	}

};

#endif
