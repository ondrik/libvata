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
#include <vata/util/transl_weak.hh>


namespace VATA
{
	class AutBase;
}


class VATA::AutBase
{
public:   // data types

	typedef size_t StateType;

	typedef VATA::Util::TwoWayDict<std::string, StateType> StringToStateDict;

	typedef std::unordered_map<StateType, StateType> StateToStateMap;
	typedef VATA::Util::TranslatorWeak<StateToStateMap> StateToStateTranslator;

	typedef std::pair<StateType, StateType> StatePair;
	typedef std::unordered_map<StatePair, StateType, boost::hash<StatePair>>
		ProductTranslMap;

	typedef std::vector<std::vector<bool>> StateBinaryRelation;

private:  // data members

	static StateType* pNextState_;

protected:// methods

	AutBase() { }

public:   // methods

	inline static void SetNextStatePtr(StateType* pNextState)
	{
		// Assertions
		assert(pNextState != nullptr);

		pNextState_ = pNextState;
	}

};

#endif
