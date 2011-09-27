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
#include <vata/tree_incl_up.hh>
#include <vata/up_tree_incl_fctor.hh>

using VATA::BDDBottomUpTreeAut;

bool VATA::CheckInclusionNoUseless(const BDDBottomUpTreeAut& smaller,
	const BDDBottomUpTreeAut& bigger)
{
	assert(&smaller != nullptr);
	assert(&bigger != nullptr);

	throw std::runtime_error("Unimplemented");
}

bool VATA::CheckInclusion(const BDDBottomUpTreeAut& smaller,
	const BDDBottomUpTreeAut& bigger)
{
	return CheckUpwardTreeInclusion<BDDBottomUpTreeAut,
		VATA::UpwardInclusionFunctor>(smaller, bigger);
}
