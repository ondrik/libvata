/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for miscellaneous utility gadgets.
 *
 *****************************************************************************/

#ifndef _VATA_UTIL_HH_
#define _VATA_UTIL_HH_

// VATA headers
#include <vata/vata.hh>

namespace VATA
{
	namespace Util
	{
		std::string ReadFile(const std::string& fileName);

		template <class Container, class Translator>
		Container RebindMap(const Container& container, const Translator& transl)
		{
			Container result;

			for (auto contElem : container)
			{	// for each element in the container
				const auto& key = contElem.first;
				const auto& value = contElem.second;

				typename Translator::const_iterator itTrans;
				if ((itTrans = transl.find(value)) != transl.end())
				{	// in case the value maps to something
					result.insert(std::make_pair(key, itTrans->second));
				}
			}

			return result;
		}
	}
}

#endif
