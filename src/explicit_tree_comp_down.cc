/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2014  Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Source file for explicit tree automaton complementation
 *
 *****************************************************************************/

#include "explicit_tree_comp_down.hh"

using VATA::ExplicitTreeAutCore;

ExplicitTreeAutCore ExplicitTreeAutCore::Complement() const
{
	typedef AutBase::StateType StateType;
	typedef std::unordered_map<StateType, StateType> StateDict;

	StateDict stateDict;

	size_t stateCnt = 0;
	Util::TranslatorWeak<StateDict> stateTranslator(
		stateDict, [&stateCnt](const StateType&){ return stateCnt++; }
	);

	this->BuildStateIndex(stateTranslator);

	return this->ComplementWithPreorder(
		Util::Identity(stateCnt)
		/* ComputeDownwardSimulation(
			aut, stateDict.size(), Util::TranslatorStrict<StateDict>(stateDict)
		)*/
	);
}
