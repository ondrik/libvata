/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for functions checking inclusion on BDD top-down tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TD_TREE_AUT_INCL_
#define _VATA_BDD_TD_TREE_AUT_INCL_

#include <vata/down_tree_incl_fctor.hh>
#include <vata/down_tree_opt_incl_fctor.hh>
#include <vata/incl_param.hh>
#include <vata/tree_incl_down.hh>
#include <vata/tree_incl_up.hh>

namespace VATA
{
	bool CheckInclusion(
		const BDDTopDownTreeAut&    smaller,
		const BDDTopDownTreeAut&    bigger,
		const VATA::InclParam&      params);
//
//
//	template <class Rel>
//	bool CheckDownwardInclusionWithPreorder(
//		const BDDTopDownTreeAut&    smaller,
//		const BDDTopDownTreeAut&    bigger,
//		const Rel&                  preorder)
//	{
//		return CheckDownwardTreeInclusion<BDDTopDownTreeAut,
//			DownwardInclusionFunctor>(smaller, bigger, preorder);
//	}
//
//	template <class Rel>
//	bool CheckOptDownwardInclusionWithPreorder(
//		const BDDTopDownTreeAut&    smaller,
//		const BDDTopDownTreeAut&    bigger,
//		const Rel&                  preorder)
//	{
//		return CheckDownwardTreeInclusion<BDDTopDownTreeAut,
//			OptDownwardInclusionFunctor>(smaller, bigger, preorder);
//	}
}

#endif
