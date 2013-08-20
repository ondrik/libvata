/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Implemention of simulation computation for explicitly represented tree
 *    automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/aut_base.hh>

#include "explicit_tree_aut_core.hh"
#include "explicit_tree_transl.hh"

using VATA::AutBase;
using VATA::ExplicitTreeAutCore;

using StateBinaryRelation  = AutBase::StateBinaryRelation;

StateBinaryRelation ExplicitTreeAutCore::ComputeDownwardSimulation(
	size_t            size) const
{
	return this->TranslateDownward().computeSimulation(size);
}

