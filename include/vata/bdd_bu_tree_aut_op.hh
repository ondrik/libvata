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
#include <vata/bdd_bu_tree_aut.hh>


namespace VATA
{
	BDDBottomUpTreeAut Union(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr);

	BDDBottomUpTreeAut Intersection(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr);

	BDDBottomUpTreeAut RemoveUnreachableStates(const BDDBottomUpTreeAut& aut);

	BDDBottomUpTreeAut RemoveUselessStates(const BDDBottomUpTreeAut& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr);

	bool CheckInclusion(const BDDBottomUpTreeAut& smaller,
		const BDDBottomUpTreeAut& bigger);

	bool CheckInclusionNoUseless(const BDDBottomUpTreeAut& smaller,
		const BDDBottomUpTreeAut& bigger);

	inline bool CheckEquivalence(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs)
	{
		return CheckInclusion(lhs, rhs) && CheckInclusion(rhs, lhs);
	}

	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const BDDBottomUpTreeAut& aut, const size_t& states);

	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const BDDBottomUpTreeAut& aut);

	inline AutBase::StateBinaryRelation ComputeSimulation(
		const BDDBottomUpTreeAut& aut)
	{
		return ComputeDownwardSimulation(aut);
	}
}

#endif
