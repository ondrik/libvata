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
#include <vata/bdd_td_tree_aut.hh>

namespace VATA
{
	class BDDTopDownTreeAut;

	bool CheckDownwardInclusion(const BDDTopDownTreeAut& smaller,
		const BDDTopDownTreeAut& bigger);

	bool CheckUpwardInclusion(const BDDTopDownTreeAut& smaller,
		const BDDTopDownTreeAut& bigger);
}

#endif
