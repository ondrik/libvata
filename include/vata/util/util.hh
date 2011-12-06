/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for miscellaneous utility gadgets.
 *
 *****************************************************************************/

#ifndef _VATA_UTIL_HH_
#define _VATA_UTIL_HH_

#include <vector>

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>

namespace VATA
{
	namespace Util
	{
		std::string ReadFile(const std::string& fileName);

		template <class Container, class Translator>
		Container RebindMap(const Container& container, const Translator& transl);

		template <class Container1, class Container2, class Translator>
		void RebindMap2(Container1& dst, const Container2& src, const Translator& transl);

		template <class Container1, class T, class Translator>
		void RebindMap2(Container1& dst, const std::vector<T>& src, const Translator& transl);

		VATA::AutBase::StringToStateDict CreateProductStringToStateMap(
			const VATA::AutBase::StringToStateDict& lhsCont,
			const VATA::AutBase::StringToStateDict& rhsCont,
			const VATA::AutBase::ProductTranslMap& translMap);

		VATA::AutBase::StringToStateDict CreateUnionStringToStateMap(
			const VATA::AutBase::StringToStateDict& lhsCont,
			const VATA::AutBase::StringToStateDict& rhsCont,
			const VATA::AutBase::StateToStateMap* translMapLhs = nullptr,
			const VATA::AutBase::StateToStateMap* translMapRhs = nullptr);

		constexpr inline size_t IntExp2(size_t val)
		{
			return (val == 0)? 1 : 2 * IntExp2(val - 1);
		}
	}
}

template <class Container, class Translator>
Container VATA::Util::RebindMap(const Container& container,
	const Translator& transl)
{
	Container result;

	for (auto& contElem : container)
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

template <class Container1, class Container2, class Translator>
void VATA::Util::RebindMap2(Container1& dst, const Container2& src, const Translator& transl)
{
	for (auto& contElem : src)
	{	// for each element in the container
		dst[contElem.first] = transl[contElem.second];
	}
}

template <class Container1, class T, class Translator>
void VATA::Util::RebindMap2(Container1& dst, const std::vector<T>& src, const Translator& transl)
{
	size_t i = 0;
	for (auto& contElem : src)
	{	// for each element in the container
		dst[i++] = transl[contElem];
	}
}

#endif
