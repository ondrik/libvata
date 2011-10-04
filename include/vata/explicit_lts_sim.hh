/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for explicit LTS simulation algorithm.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_LTS_SIM_HH_
#define _VATA_EXPLICIT_LTS_SIM_HH_

#include <vector>

// VATA headers
#include <vata/vata.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/lts.hh>

namespace VATA {

	void computeSimulation(Util::BinaryRelation& result, size_t outputSize, const Util::LTS& lts);

	void computeSimulation(Util::BinaryRelation& result, size_t outputSize, const Util::LTS& lts,
		const std::vector<std::vector<size_t>>& part, const std::vector<size_t>& finalStates);

}

#endif
