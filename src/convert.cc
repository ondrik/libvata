/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Convert class implementation.
 *
 *****************************************************************************/

// VATA headers
#include <vata/util/convert.hh>


namespace VATA
{
	namespace Util
	{
		template <>
		std::string Convert::ToString<unsigned char>(const unsigned char& n)
		{
			// the output stream for the string
			std::ostringstream oss;
			// insert the object into the stream
			oss << static_cast<unsigned>(n);
			// return the string
			return oss.str();
		}
	}
}
