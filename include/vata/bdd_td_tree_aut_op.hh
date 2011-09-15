/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for operations on BDD top-down tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TD_TREE_AUT_OP_HH_
#define _VATA_BDD_TD_TREE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_op.hh>
#include <vata/bdd_td_tree_aut.hh>


namespace VATA
{
	template <>
	BDDTopDownTreeAut Union(const BDDTopDownTreeAut& lhs,
		const BDDTopDownTreeAut& rhs, AutBase::StateToStateMap* pTranslMap);

	template <>
	BDDTopDownTreeAut Intersection(const BDDTopDownTreeAut& lhs,
		const BDDTopDownTreeAut& rhs, AutBase::ProductTranslMap* pTranslMap);

	template <>
	BDDTopDownTreeAut RemoveUnreachableStates(const BDDTopDownTreeAut& aut,
		AutBase::StateToStateMap* pTranslMap);

	template <>
	BDDTopDownTreeAut RemoveUselessStates(const BDDTopDownTreeAut& aut,
		AutBase::StateToStateMap* pTranslMap);

	template <>
	bool CheckInclusion(const BDDTopDownTreeAut& smaller,
		const BDDTopDownTreeAut& bigger);

	template <>
	bool CheckInclusionNoUseless(const BDDTopDownTreeAut& smaller,
		const BDDTopDownTreeAut& bigger);
}

#endif
