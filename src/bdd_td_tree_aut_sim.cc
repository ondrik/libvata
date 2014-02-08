/*****************************************************************************
 *  VATA Tree Automata Library
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

using VATA::BDDTopDownTreeAut;
using VATA::Util::Convert;

typedef VATA::AutBase::StateBinaryRelation StateBinaryRelation;


StateBinaryRelation BDDTopDownTreeAut::ComputeSimulation(
	const SimParam&             /*  params */) const
{
	throw NotImplementedException(__func__);
}
