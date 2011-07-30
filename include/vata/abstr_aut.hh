/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for an abstract class representing some automaton.
 *
 *****************************************************************************/

#ifndef _VATA_ABSTR_AUT_HH_
#define _VATA_ABSTR_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/util/two_way_dict.hh>

namespace VATA
{
	class AbstrAut;
}


class VATA::AbstrAut
{
public:   // data types

	typedef uint32_t StateType;

	typedef VATA::Util::TwoWayDict<std::string, StateType> StringToStateDict;

public:   // methods

	virtual void LoadFromString(VATA::Parsing::AbstrParser& parser,
		const std::string& str,
		StringToStateDict* pStateDict = static_cast<StringToStateDict*>(0),
		const std::string& params = "") = 0;

	virtual std::string DumpToString(VATA::Serialization::AbstrSerializer& serializer,
		StringToStateDict* pStateDict = static_cast<StringToStateDict*>(0)) = 0;

	virtual ~AbstrAut()
	{	}
};

#endif
