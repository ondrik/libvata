/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Source file of computation of downward inclusion on top-down represented
 *    tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/bdd_td_tree_aut_op.hh>

using VATA::BDDTopDownTreeAut;
using VATA::Util::Convert;

typedef VATA::AutBase::StateBinaryRelation StateBinaryRelation;


StateBinaryRelation VATA::ComputeDownwardSimulation(
	const BDDTopDownTreeAut& aut, const size_t& size)
{
	if ((&aut == nullptr) || (&size == nullptr)) { }

	throw std::runtime_error("Unimplemented");
}

StateBinaryRelation VATA::ComputeUpwardSimulation(
	const BDDTopDownTreeAut& aut, const size_t& size)
{
	if ((&aut == nullptr) || (&size == nullptr)) { }

	throw std::runtime_error("Unimplemented");
}
