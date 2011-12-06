/*****************************************************************************
 *  VATA Tree Automata Library
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
#include <vata/parsing/abstr_parser.hh>
#include <vata/util/convert.hh>
#include <vata/util/triple.hh>

namespace VATA
{
	namespace Parsing
	{
		class TimbukParser;
	}
}


class VATA::Parsing::TimbukParser :
	public VATA::Parsing::AbstrParser
{
public:   // methods

	virtual AutDescription ParseString(const std::string& str);

	virtual ~TimbukParser()
	{ }
};

#endif
