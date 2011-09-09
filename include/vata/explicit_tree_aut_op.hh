/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <ilengal@fit.vutbr.cz>
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
#include <vata/tree_incl_down.hh>

namespace VATA
{
	template <class SymbolType>
	ExplicitTreeAut<SymbolType> Union(const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs, AutBase::StateToStateMap* pTranslMap);

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> Intersection(const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs, AutBase::ProductTranslMap* pTranslMap);

	template <class SymbolType>
	ExplicitTreeAut<SymbolType> RemoveUnreachableStates(const ExplicitTreeAut<SymbolType>& aut,
		AutBase::StateToStateMap* pTranslMap);

	template <class SymbolType>
	bool CheckInclusion(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger) {

		return CheckDownwardTreeInclusion(smaller, bigger);

	}

}

#endif
