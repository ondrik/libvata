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
#include <vata/ta_expl/explicit_tree_aut.hh>
#include <vata/ta_expl/explicit_tree_transl.hh>

using VATA::AutBase;
using VATA::ExplicitTreeAut;

AutBase::StateBinaryRelation ExplicitTreeAut::ComputeDownwardSimulation(
	size_t            size) const
{
	return this->TranslateDownward().computeSimulation(size);
}

