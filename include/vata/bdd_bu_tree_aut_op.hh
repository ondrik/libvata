/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for operations on BDD bottom-up tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_BU_TREE_AUT_OP_HH_
#define _VATA_BDD_BU_TREE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_op.hh>
#include <vata/bdd_bu_tree_aut.hh>


namespace VATA
{
	template <>
	BDDBottomUpTreeAut Union(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs, AutBase::StateToStateMap* pTranslMap);

	template <>
	BDDBottomUpTreeAut Intersection(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs, AutBase::ProductTranslMap* pTranslMap);

	template <>
	BDDBottomUpTreeAut RemoveUnreachableStates(const BDDBottomUpTreeAut& aut,
		AutBase::StateToStateMap* pTranslMap);

	template <>
	BDDBottomUpTreeAut RemoveUselessStates(const BDDBottomUpTreeAut& aut,
		AutBase::StateToStateMap* pTranslMap);

	template <>
	bool CheckInclusion(const BDDBottomUpTreeAut& smaller,
		const BDDBottomUpTreeAut& bigger);

	template <>
	bool CheckInclusionNoUseless(const BDDBottomUpTreeAut& smaller,
		const BDDBottomUpTreeAut& bigger);
}

#endif
