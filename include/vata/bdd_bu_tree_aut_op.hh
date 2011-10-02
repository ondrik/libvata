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

	BDDBottomUpTreeAut UnionDisjunctStates(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs);

	BDDBottomUpTreeAut Intersection(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr);

	BDDBottomUpTreeAut RemoveUnreachableStates(const BDDBottomUpTreeAut& aut);

	BDDBottomUpTreeAut RemoveUselessStates(const BDDBottomUpTreeAut& aut);

	bool CheckUpwardInclusion(const BDDBottomUpTreeAut& smaller,
		const BDDBottomUpTreeAut& bigger);

	bool CheckUpwardInclusionWithoutUseless(const BDDBottomUpTreeAut& smaller,
		const BDDBottomUpTreeAut& bigger);

	bool CheckUpwardInclusionWithSimulation(const BDDBottomUpTreeAut& smaller,
		const BDDBottomUpTreeAut& bigger);

	bool CheckDownwardInclusion(const BDDBottomUpTreeAut& smaller,
		const BDDBottomUpTreeAut& bigger);

	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const BDDBottomUpTreeAut& aut, const size_t& states);

	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const BDDBottomUpTreeAut& aut);

	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const BDDBottomUpTreeAut& aut);

	bool CheckDownwardInclusionWithSimulation(
		const BDDBottomUpTreeAut& smaller, const BDDBottomUpTreeAut& bigger);

	template <class Rel>
	bool CheckDownwardInclusionWithPreorder(
		const BDDBottomUpTreeAut& smaller, const BDDBottomUpTreeAut& bigger,
		const Rel& preorder);
}

#endif
