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
#include <vata/bdd_td_tree_aut.hh>


namespace VATA
{
	class BDDTopDownTreeAut;

	BDDTopDownTreeAut Union(const BDDTopDownTreeAut& lhs,
		const BDDTopDownTreeAut& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr);

	BDDTopDownTreeAut Intersection(const BDDTopDownTreeAut& lhs,
		const BDDTopDownTreeAut& rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr);

	BDDTopDownTreeAut RemoveUnreachableStates(const BDDTopDownTreeAut& aut);

	BDDTopDownTreeAut RemoveUselessStates(const BDDTopDownTreeAut& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr);

	bool CheckInclusion(const BDDTopDownTreeAut& smaller,
		const BDDTopDownTreeAut& bigger);

	bool CheckInclusionNoUseless(const BDDTopDownTreeAut& smaller,
		const BDDTopDownTreeAut& bigger);

	inline bool CheckEquivalence(const BDDTopDownTreeAut& lhs,
		const BDDTopDownTreeAut& rhs)
	{
		return CheckInclusion(lhs, rhs) && CheckInclusion(rhs, lhs);
	}

	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const BDDTopDownTreeAut& aut);
}

#endif
