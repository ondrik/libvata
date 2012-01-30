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


/**
 * @brief  Class for a parser of automata in the Timbuk format
 *
 * This class is a parser for automata in the Timbuk format.
 */
class VATA::Parsing::TimbukParser :
	public VATA::Parsing::AbstrParser
{
public:   // methods

	/**
	 * @copydoc  VATA::Parsing::AbstrParser::ParseString
	 */
	virtual AutDescription ParseString(const std::string& str);

	/**
	 * @copydoc  VATA::Parsing::AbstrParser::~AbstrParser
	 */
	virtual ~TimbukParser()
	{ }
};

#endif
