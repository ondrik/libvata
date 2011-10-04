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
#include <vata/util/binary_relation.hh>
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

	typedef VATA::Util::BinaryRelation StateBinaryRelation;

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

	template <class Automaton>
	static StateType SanitizeAutsForInclusion(Automaton& smaller, Automaton& bigger)
	{
		StateType stateCnt = 0;
		StateToStateMap stateMap;
		StateToStateTranslator stateTrans(stateMap,
			[&stateCnt](const StateType&){return stateCnt++;});

		Automaton tmpAut = RemoveUselessStates(smaller);
		Automaton newSmaller;
		tmpAut.ReindexStates(newSmaller, stateTrans);
		for (const StateType& fst : smaller.GetFinalStates())
		{
			newSmaller.SetStateFinal(stateTrans(fst));
		}

		tmpAut = RemoveUselessStates(bigger);
		stateMap.clear();
		Automaton newBigger;
		bigger.ReindexStates(newBigger, stateTrans);
		for (const StateType& fst : bigger.GetFinalStates())
		{
			newBigger.SetStateFinal(stateTrans(fst));
		}

		smaller = newSmaller;
		bigger = newBigger;

		return stateCnt;
	}

	template <class Automaton>
	static StateType SanitizeAutForSimulation(Automaton& aut)
	{
		StateType stateCnt = 0;
		StateToStateMap stateMap;
		StateToStateTranslator stateTrans(stateMap,
			[&stateCnt](const StateType&){return stateCnt++;});

		Automaton newAut = RemoveUselessStates(aut);
		Automaton reindexedAut;
		newAut.ReindexStates(reindexedAut, stateTrans);

		aut = reindexedAut;

		return stateCnt;
	}
};

#endif
