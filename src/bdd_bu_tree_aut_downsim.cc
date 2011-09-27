/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Source file of computation of downward inclusion on bottom-up
 *    represented tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

using VATA::BDDBottomUpTreeAut;
using VATA::BDDTopDownTreeAut;
using VATA::Util::Convert;

typedef VATA::AutBase::StateBinaryRelation StateBinaryRelation;
typedef VATA::AutBase::StateType StateType;
typedef VATA::AutBase::StateToStateMap StateToStateMap;
typedef VATA::AutBase::StateToStateTranslator StateToStateTranslator;


StateBinaryRelation VATA::ComputeDownwardSimulation(
	const BDDBottomUpTreeAut& aut)
{
	BDDBottomUpTreeAut newAut;

	StateType stateCnt = 0;
	StateToStateMap stateMap;
	StateToStateTranslator stateTrans(stateMap,
		[&stateCnt](const StateType&){return stateCnt++;});

	aut.ReindexStates(newAut, stateTrans);

	return ComputeDownwardSimulation(newAut, stateCnt);
}

StateBinaryRelation VATA::ComputeDownwardSimulation(
	const BDDBottomUpTreeAut& aut, const size_t& states)
{
	assert(&aut != nullptr);
	StateBinaryRelation result(states);

	return result;
}

