/*****************************************************************************
 *  VATA Tree Automata Library
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
#include <vata/tree_incl_up.hh>
#include <vata/up_tree_incl_fctor.hh>


namespace VATA
{
	BDDBottomUpTreeAut Union(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr);

	BDDBottomUpTreeAut UnionDisjointStates(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs);

	BDDBottomUpTreeAut Intersection(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr);

	BDDBottomUpTreeAut RemoveUnreachableStates(const BDDBottomUpTreeAut& aut);

	BDDBottomUpTreeAut RemoveUselessStates(const BDDBottomUpTreeAut& aut);
}

#endif
