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

using VATA::AutBase;
using VATA::BDDBottomUpTreeAut;
using VATA::BDDTopDownTreeAut;
using VATA::Util::Convert;

bool VATA::CheckDownwardInclusion(const BDDBottomUpTreeAut& smaller,
	const BDDBottomUpTreeAut& bigger)
{
	BDDTopDownTreeAut invSmaller = smaller.GetTopDownAut();
	BDDTopDownTreeAut invBigger = bigger.GetTopDownAut();

	return CheckDownwardInclusion(invSmaller, invBigger);
}
