/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of inclusion on BDD bottom-up tree automata
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_bu_tree_aut_op.hh>
#include <vata/bdd_td_tree_aut_op.hh>
#include <vata/tree_incl_up.hh>
#include <vata/up_tree_incl_fctor.hh>

using VATA::AutBase;
using VATA::BDDBottomUpTreeAut;
using VATA::BDDTopDownTreeAut;

bool VATA::CheckDownwardInclusion(const BDDBottomUpTreeAut& smaller,
	const BDDBottomUpTreeAut& bigger)
{
	BDDTopDownTreeAut invSmaller = smaller.GetTopDownAut();
	BDDTopDownTreeAut invBigger = bigger.GetTopDownAut();

	return CheckDownwardInclusion(invSmaller, invBigger);
}

bool VATA::CheckUpwardInclusionWithoutUseless(const BDDBottomUpTreeAut& smaller,
	const BDDBottomUpTreeAut& bigger)
{
	assert(&smaller != nullptr);
	assert(&bigger != nullptr);

	throw std::runtime_error("Unimplemented");
}

bool VATA::CheckUpwardInclusion(const BDDBottomUpTreeAut& smaller,
	const BDDBottomUpTreeAut& bigger)
{
	VATA_LOGGER_INFO("This function should be rewritten.");

	return CheckUpwardTreeInclusion<BDDBottomUpTreeAut,
		VATA::UpwardInclusionFunctor>(smaller, bigger);
}

bool VATA::CheckUpwardInclusionWithSimulation(const BDDBottomUpTreeAut& smaller,
	const BDDBottomUpTreeAut& bigger)
{
	assert(&smaller != nullptr);
	assert(&bigger != nullptr);

	throw std::runtime_error("Unimplemented");
}

bool VATA::CheckDownwardInclusionWithSimulation(
	const BDDBottomUpTreeAut& smaller, const BDDBottomUpTreeAut& bigger)
{
	typedef AutBase::StateType StateType;
	typedef AutBase::StateToStateMap StateToStateMap;
	typedef AutBase::StateToStateTranslator StateToStateTranslator;

	StateType stateCnt = 0;
	StateToStateMap stateMap;
	StateToStateTranslator stateTrans(stateMap,
		[&stateCnt](const StateType&){return stateCnt++;});

	BDDBottomUpTreeAut tmpAut = RemoveUselessStates(smaller);
	BDDBottomUpTreeAut newSmaller;
	tmpAut.ReindexStates(newSmaller, stateTrans);

	tmpAut = RemoveUselessStates(bigger);
	BDDBottomUpTreeAut newBigger;
	tmpAut.ReindexStates(newBigger, stateTrans);

	BDDBottomUpTreeAut unionAut = UnionDisjunctStates(newSmaller, newBigger);

	AutBase::StateBinaryRelation sim =
		ComputeDownwardSimulation(unionAut, stateCnt);

	BDDTopDownTreeAut invertSmaller = newSmaller.GetTopDownAut();
	BDDTopDownTreeAut invertBigger = newBigger.GetTopDownAut();

	return CheckDownwardInclusionWithPreorder(invertSmaller, invertBigger, sim);
}
