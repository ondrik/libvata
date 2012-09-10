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

	BDDTopDownTreeAut UnionDisjunctStates(const BDDTopDownTreeAut& lhs,
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

	template <class Rel>
	bool CheckDownwardInclusionWithPreorder(
		const BDDTopDownTreeAut& smaller, const BDDTopDownTreeAut& bigger,
		const Rel& preorder)
	{
		return CheckDownwardTreeInclusion<BDDTopDownTreeAut,
			DownwardInclusionFunctor>(smaller, bigger, preorder);
	}

	template <class Rel>
	bool CheckDownwardInclusionNonRecWithPreorder(
		const BDDTopDownTreeAut& smaller, const BDDTopDownTreeAut& bigger,
		const Rel& preorder)
	{
		if ((&smaller == nullptr) || (&bigger == nullptr) ||
			(&preorder == nullptr)) { }

		throw std::runtime_error("Unimplemented");
	}

	template <class Rel>
	bool CheckOptDownwardInclusionWithPreorder(
		const BDDTopDownTreeAut& smaller, const BDDTopDownTreeAut& bigger,
		const Rel& preorder)
	{
		return CheckDownwardTreeInclusion<BDDTopDownTreeAut,
			OptDownwardInclusionFunctor>(smaller, bigger, preorder);
	}

	template <class Rel>
	bool CheckUpwardInclusionWithPreorder(
		const BDDTopDownTreeAut& smaller, const BDDTopDownTreeAut& bigger,
		const Rel& preorder)
	{
		if ((&smaller == nullptr) || (&bigger == nullptr) ||
			(&preorder == nullptr)) { }

		throw std::runtime_error("Unimplemented");
	}

	inline bool CheckInclusion(const BDDTopDownTreeAut& smaller,
		const BDDTopDownTreeAut& bigger)
	{
		return CheckDownwardInclusion(smaller, bigger);
	}
}

#endif
