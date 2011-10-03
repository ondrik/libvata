/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of inclusion on BDD top-down tree automata
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/bdd_td_tree_aut_op.hh>
#include <vata/down_tree_incl_fctor.hh>
#include <vata/tree_incl_down.hh>

using VATA::BDDTopDownTreeAut;

bool VATA::CheckDownwardInclusion(const BDDTopDownTreeAut& smaller,
	const BDDTopDownTreeAut& bigger)
{
	BDDTopDownTreeAut newSmaller = RemoveUselessStates(smaller);
	BDDTopDownTreeAut newBigger = RemoveUselessStates(bigger);

	return CheckDownwardInclusionWithoutUseless(newSmaller, newBigger);
}

bool VATA::CheckDownwardInclusionWithoutUseless(
	const BDDTopDownTreeAut& smaller, const BDDTopDownTreeAut& bigger)
{
	typedef AutBase::StateType StateType;
	typedef AutBase::StateToStateMap StateToStateMap;
	typedef AutBase::StateToStateTranslator StateToStateTranslator;

	typedef VATA::Util::Convert Convert;

	StateType stateCnt = 0;
	StateToStateMap stateMap;
	StateToStateTranslator stateTrans(stateMap,
		[&stateCnt](const StateType&){return stateCnt++;});

	BDDTopDownTreeAut newSmaller;
	smaller.ReindexStates(newSmaller, stateTrans);
	for (const StateType& fst : smaller.GetFinalStates())
	{
		newSmaller.SetStateFinal(stateTrans(fst));
	}

	stateMap.clear();
	BDDTopDownTreeAut newBigger;
	bigger.ReindexStates(newBigger, stateTrans);
	for (const StateType& fst : bigger.GetFinalStates())
	{
		newBigger.SetStateFinal(stateTrans(fst));
	}

	VATA::Util::Identity ident(stateCnt);

	return CheckDownwardTreeInclusion<BDDTopDownTreeAut,
		VATA::DownwardInclusionFunctor>(newSmaller, newBigger, ident);
}

bool VATA::CheckUpwardInclusion(const BDDTopDownTreeAut& smaller,
	const BDDTopDownTreeAut& bigger)
{
	assert(&smaller != nullptr);
	assert(&bigger != nullptr);

	throw std::runtime_error("Unimplemented");
}

bool VATA::CheckUpwardInclusionWithSimulation(const BDDTopDownTreeAut& smaller,
	const BDDTopDownTreeAut& bigger)
{
	assert(&smaller != nullptr);
	assert(&bigger != nullptr);

	throw std::runtime_error("Unimplemented");
}

bool VATA::CheckDownwardInclusionWithSimulation(const BDDTopDownTreeAut& smaller,
	const BDDTopDownTreeAut& bigger)
{
	assert(&smaller != nullptr);
	assert(&bigger != nullptr);

	throw std::runtime_error("Unimplemented");
}

