/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for an abstract class representing some automaton.
 *
 *****************************************************************************/

#ifndef _VATA_ABSTR_AUT_HH_
#define _VATA_ABSTR_AUT_HH_

// VATA headers
#include <vata/vata.hh>

namespace VATA
{
	class AbstrAut;
}


class VATA::AbstrAut
{

public:   // public methods

	virtual ~AbstrAut()
	{	}
};

#endif
