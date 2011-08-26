/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of miscellaneous utility gadgets.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/util/util.hh>

// Standard library headers
#include <fstream>

std::string VATA::Util::ReadFile(const std::string& fileName)
{
	std::ifstream t(fileName);
	if (!t)
	{	// in case the file could not be open
		throw std::runtime_error("Error opening file " + fileName);
	}

	std::string str;

	t.seekg(0, std::ios::end);
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	return str;
}


VATA::AutBase::StringToStateDict VATA::Util::CreateProductStringToStateMap(
	const VATA::AutBase::StringToStateDict& lhsCont,
	const VATA::AutBase::StringToStateDict& rhsCont,
	const AutBase::ProductTranslMap& translMap)
{
	typedef VATA::AutBase::StringToStateDict StringToStateDict;

	StringToStateDict result;

	for (auto mapElem : translMap)
	{
		const auto& key = mapElem.first;

		StringToStateDict::ConstIteratorBwd itLhs;
		if ((itLhs = lhsCont.FindBwd(key.first)) == lhsCont.EndBwd())
		{
			assert(false);  // fail gracefully
		}

		StringToStateDict::ConstIteratorBwd itRhs;
		if ((itRhs = rhsCont.FindBwd(key.second)) == rhsCont.EndBwd())
		{
			assert(false);  // fail gracefully
		}

		std::string prodStateStr = '[' + itLhs->second + "_1|" +
			itRhs->second + "_2]";
		result.insert(std::make_pair(prodStateStr, mapElem.second));
	}

	return result;
}


VATA::AutBase::StringToStateDict VATA::Util::CreateUnionStringToStateMap(
	const VATA::AutBase::StringToStateDict& lhsCont,
	const VATA::AutBase::StringToStateDict& rhsCont,
	const AutBase::StateToStateMap* translMap)
{
	typedef VATA::AutBase::StateType StateType;
	typedef VATA::AutBase::StateToStateMap StateToStateMap;
	typedef VATA::AutBase::StringToStateDict StringToStateDict;

	StringToStateDict result;

	for (auto dictElem : lhsCont)
	{
		result.insert(std::make_pair(dictElem.first + "_1", dictElem.second));
	}

	for (auto dictElem : rhsCont)
	{
		StateType state = dictElem.second;
		if (translMap != nullptr)
		{	// in case there should be translation
			StateToStateMap::const_iterator itTransl;
			if ((itTransl = translMap->find(state)) == translMap->end())
			{
				assert(false);    // fail gracefully
			}

			state = itTransl->second;
		}

		result.insert(std::make_pair(dictElem.first + "_2", state));
	}

	return result;
}
