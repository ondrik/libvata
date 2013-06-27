/*****************************************************************************
 *  VATA Tree Automata Library
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
#include <vata/tree_incl_down.hh>
#include <vata/down_tree_incl_fctor.hh>
#include <vata/down_tree_opt_incl_fctor.hh>


namespace VATA
{
	class BDDTopDownTreeAut;

	BDDTopDownTreeAut Union(const BDDTopDownTreeAut& lhs,
		const BDDTopDownTreeAut& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr);

	BDDTopDownTreeAut UnionDisjointStates(const BDDTopDownTreeAut& lhs,
		const BDDTopDownTreeAut& rhs);

	BDDTopDownTreeAut Intersection(const BDDTopDownTreeAut& lhs,
		const BDDTopDownTreeAut& rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr);

	BDDTopDownTreeAut RemoveUnreachableStates(const BDDTopDownTreeAut& aut);

	BDDTopDownTreeAut RemoveUselessStates(const BDDTopDownTreeAut& aut);

	bool CheckDownwardInclusion(const BDDTopDownTreeAut& smaller,
		const BDDTopDownTreeAut& bigger);

	bool CheckUpwardInclusion(const BDDTopDownTreeAut& smaller,
		const BDDTopDownTreeAut& bigger);

	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const BDDTopDownTreeAut& aut);

	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const BDDTopDownTreeAut& aut, const size_t& size);

	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const BDDTopDownTreeAut& aut);

	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const BDDTopDownTreeAut& aut, const size_t& size);

}

#endif
