/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the base class of automata.
 *
 *****************************************************************************/

#ifndef _VATA_AUT_BASE_HH_
#define _VATA_AUT_BASE_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/util/two_way_dict.hh>


namespace VATA
{
	class AutBase;
}


class VATA::AutBase
{
public:   // data types

	typedef uint32_t StateType;

	typedef VATA::Util::TwoWayDict<std::string, StateType> StringToStateDict;

	typedef std::unordered_map<StateType, StateType> StateToStateMap;

};

#endif
