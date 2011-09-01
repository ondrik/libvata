/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for operations on BDD tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TREE_AUT_OP_HH_
#define _VATA_BDD_TREE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_op.hh>
#include <vata/bdd_tree_aut.hh>


namespace VATA
{
	template <>
	BDDTreeAut Union(const BDDTreeAut& lhs, const BDDTreeAut& rhs,
		AutBase::StateToStateMap* pTranslMap);

	template <>
	BDDTreeAut Intersection(const BDDTreeAut& lhs, const BDDTreeAut& rhs,
		AutBase::ProductTranslMap* pTranslMap);

	template <>
	BDDTreeAut RemoveUnreachableStates(const BDDTreeAut& aut,
		AutBase::StateToStateMap* pTranslMap);

	template <>
	BDDTreeAut RemoveUselessStates(const BDDTreeAut& aut,
		AutBase::StateToStateMap* pTranslMap);

	template <>
	bool CheckInclusion(const BDDTreeAut& smaller, const BDDTreeAut& bigger);

	template <>
	bool CheckInclusionNoUseless(const BDDTreeAut& smaller,
		const BDDTreeAut& bigger);
}

#endif
