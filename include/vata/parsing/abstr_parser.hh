/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for abstract class of automaton format parser.
 *
 *****************************************************************************/

#ifndef _VATA_ABSTR_PARSER_HH_
#define _VATA_ABSTR_PARSER_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/aut_description.hh>
#include <vata/util/triple.hh>


namespace VATA
{
	namespace Parsing
	{
		class AbstrParser;
	}
}


class VATA::Parsing::AbstrParser
{
public:   // data types

	typedef VATA::Util::AutDescription AutDescription;

public:   // methods

	virtual AutDescription ParseString(const std::string& str) = 0;

	virtual ~AbstrParser()
	{ }
};

#endif
