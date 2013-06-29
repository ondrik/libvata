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
		const VATA::InclParam&      params);
}

#endif
