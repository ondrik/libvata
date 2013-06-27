/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for functions checking inclusion on BDD bottom-up tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_BU_TREE_AUT_INCL_
#define _VATA_BDD_BU_TREE_AUT_INCL_

#include <vata/incl_param.hh>

namespace VATA
{
	bool CheckInclusion(
		const BDDBottomUpTreeAut&   smaller,
		const BDDBottomUpTreeAut&   bigger,
		const VATA::InclParam*      params = nullptr);

	bool CheckUpwardInclusion(
		const BDDBottomUpTreeAut&   smaller,
		const BDDBottomUpTreeAut&   bigger);

	bool CheckDownwardInclusion(
		const BDDBottomUpTreeAut&   smaller,
		const BDDBottomUpTreeAut&   bigger);

	template <class Rel>
	bool CheckDownwardInclusionWithPreorder(
		const BDDBottomUpTreeAut&   smaller,
		const BDDBottomUpTreeAut&   bigger,
		const Rel&                  preorder)
	{
		BDDTopDownTreeAut invertSmaller = smaller.GetTopDownAut();
		BDDTopDownTreeAut invertBigger = bigger.GetTopDownAut();

		return CheckDownwardInclusionWithPreorder(invertSmaller, invertBigger,
			preorder);
	}

	template <class Rel>
	bool CheckOptDownwardInclusionWithPreorder(
		const BDDBottomUpTreeAut& smaller, const BDDBottomUpTreeAut& bigger,
		const Rel& preorder)
	{
		BDDTopDownTreeAut invertSmaller = smaller.GetTopDownAut();
		BDDTopDownTreeAut invertBigger = bigger.GetTopDownAut();

		return CheckOptDownwardInclusionWithPreorder(invertSmaller, invertBigger,
			preorder);
	}

	template <class Rel>
	bool CheckUpwardInclusionWithPreorder(
		const BDDBottomUpTreeAut& smaller, const BDDBottomUpTreeAut& bigger,
		const Rel& preorder)
	{
		return CheckUpwardTreeInclusion<BDDBottomUpTreeAut,
			VATA::UpwardInclusionFunctor>(smaller, bigger, preorder);
	}

	// Added due to FA extension
	template <class Rel>
	bool CheckUpwardInclusionWithSim(
		const BDDBottomUpTreeAut& smaller, const BDDBottomUpTreeAut& bigger,
		const Rel& preorder)
	{
		return CheckUpwardTreeInclusion<BDDBottomUpTreeAut,
			VATA::UpwardInclusionFunctor>(smaller, bigger, preorder);
	}
}

#endif
