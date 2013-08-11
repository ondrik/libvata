/*****************************************************************************
 *  VATA Tree Automata Library
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
#include <vata/notimpl_except.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>

// Utilities
#include <vata/util/binary_relation.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/transl_weak.hh>
#include <vata/util/transl_strict.hh>


namespace VATA
{
	class AutBase;

	template <class Automaton>
	Automaton Reduce(const Automaton&)
	{
		throw NotImplementedException(__func__);
	}

	template <class Automaton>
	Automaton GetCandidateTree(const Automaton&)
	{
		throw NotImplementedException(__func__);
	}

	template <class Automaton, class Dict>
	Automaton Complement(const Automaton& aut, const Dict& alphabet)
	{
		if ((nullptr == &aut) && (nullptr == &alphabet))
		{ }

		throw NotImplementedException(__func__);
	}

	/**
	 * @brief  Generic procedure for checking equivalence of automata
	 *
	 * To be used when a specific implementation is not available
	 */
	template <class Automaton>
	bool CheckEquivalence(const Automaton& lhs, const Automaton& rhs)
	{
		return CheckInclusion(lhs, rhs) && CheckInclusion(rhs, lhs);
	}
}


/**
 * @brief  Base class for automata
 *
 * This is the base class for automata. Contains  mostly static definitions, no
 * virtual methods are present (so that the overhead of virtual method table
 * would not propagate.
 */
class VATA::AutBase
{
public:   // data types

	typedef size_t StateType;

	typedef VATA::Util::TwoWayDict<std::string, StateType> StringToStateDict;
	typedef VATA::Util::TranslatorStrict<AutBase::StringToStateDict::MapBwdType>
		StateBackTranslatorStrict;

	typedef std::unordered_map<StateType, StateType> StateToStateMap;
	typedef VATA::Util::TranslatorWeak<StateToStateMap> StateToStateTranslator;

	typedef std::pair<StateType, StateType> StatePair;
	typedef std::unordered_map<StatePair, StateType, boost::hash<StatePair>>
		ProductTranslMap;

	typedef VATA::Util::BinaryRelation StateBinaryRelation;

private:  // data members

#if 0
	static StateType* pNextState_;
#endif

protected:// methods

	AutBase() { }

public:   // methods

#if 0
	inline static void SetNextStatePtr(StateType* pNextState)
	{
		// Assertions
		assert(pNextState != nullptr);

		pNextState_ = pNextState;
	}
#endif

	template <class Automaton>
	static StateType SanitizeAutsForInclusion(
		Automaton&     smaller,
		Automaton&     bigger)
	{
		StateType stateCnt = 0;
		StateToStateMap stateMap;
		StateToStateTranslator stateTrans(stateMap,
			[&stateCnt](const StateType&){return stateCnt++;});

		Automaton tmpAut = smaller.RemoveUselessStates();
		Automaton newSmaller;
		tmpAut.ReindexStates(newSmaller, stateTrans);

		tmpAut = bigger.RemoveUselessStates();
		stateMap.clear();
		Automaton newBigger;
		tmpAut.ReindexStates(newBigger, stateTrans);

		smaller = newSmaller;
		bigger = newBigger;

		return stateCnt;
	}

	/*
	template <class Automaton>
	static StateType SanitizeAutForSimulation(Automaton& aut)
	{
		StateType stateCnt = 0;
		StateToStateMap stateMap;
		StateToStateTranslator stateTrans(stateMap,
			[&stateCnt](const StateType&){return stateCnt++;});

		Automaton newAut = aut.RemoveUselessStates();
		Automaton reindexedAut;
		newAut.ReindexStates(reindexedAut, stateTrans);

		aut = reindexedAut;

		return stateCnt;
	}
	*/

	template <class Automaton, class Index>
	static StateType SanitizeAutForSimulation(Automaton& aut,
			StateType& stateCnt, Index& index)
	{
		Automaton newAut = aut.RemoveUselessStates();
		Automaton reindexedAut;
		newAut.ReindexStates(reindexedAut, index);

		aut = reindexedAut;

		return stateCnt;
	}
};

#endif
