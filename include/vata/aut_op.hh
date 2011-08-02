/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for operations on automata.
 *
 *****************************************************************************/

#ifndef _VATA_AUT_OP_HH_
#define _VATA_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>

namespace VATA
{
	template <class Aut>
	Aut Union(const Aut& lhs, const Aut& rhs);
}

#endif
