/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Source file for an explicit tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/ta_expl/explicit_tree_aut.hh>

#include "explicit_tree_sim.hh"
#include "explicit_tree_unreach.hh"

// global tuple cache definition
VATA::Explicit::TupleCache VATA::Explicit::tupleCache;

// pointer to symbol dictionary
VATA::ExplicitTreeAut::StringToSymbolDict* VATA::ExplicitTreeAut::pSymbolDict_ = nullptr;

// pointer to next symbol counter
VATA::ExplicitTreeAut::SymbolType* VATA::ExplicitTreeAut::pNextSymbol_ = nullptr;


VATA::ExplicitTreeAut VATA::ExplicitTreeAut::Reduce() const
{
	typedef AutBase::StateType StateType;

	typedef Util::TwoWayDict<
		StateType,
		StateType,
		std::unordered_map<StateType, StateType>,
		std::unordered_map<StateType, StateType>
	> StateDict;

	size_t stateCnt = 0;

	StateDict stateDict;
	Util::TranslatorWeak<StateDict> stateTranslator(
		stateDict, [&stateCnt](const StateType&){ return stateCnt++; }
	);

	this->BuildStateIndex(stateTranslator);

	AutBase::StateBinaryRelation sim = this->ComputeDownwardSimulation(
		stateDict.size(), Util::TranslatorStrict<StateDict>(stateDict)
	);

	return this->CollapseStates(
			sim, Util::TranslatorStrict<StateDict::MapBwdType>(stateDict.GetReverseMap())
		).RemoveUnreachableStates(sim, Util::TranslatorStrict<StateDict>(stateDict)
	);
}
