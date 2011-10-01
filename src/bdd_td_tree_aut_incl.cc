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
	VATA::Util::Identity ident;

	return CheckDownwardTreeInclusion<BDDTopDownTreeAut,
		VATA::DownwardInclusionFunctor>(smaller, bigger, ident);
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

