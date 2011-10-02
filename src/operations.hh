/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Operations for the command-line interface to the VATA library.
 *
 *****************************************************************************/

#ifndef _OPERATIONS_HH_
#define _OPERATIONS_HH_

// VATA headers
#include <vata/vata.hh>

// local headers
#include "parse_args.hh"

using VATA::Util::Convert;

template <class Automaton>
bool CheckInclusion(const Automaton& smaller, const Automaton& bigger,
	const Arguments& args)
{
	// insert default values
	Options options = args.options;
	options.insert(std::make_pair("sim", "no"));
	options.insert(std::make_pair("dir", "up"));

	if ((options["sim"] == "no") && (options["dir"] == "up"))
	{
		return VATA::CheckUpwardInclusion(smaller, bigger);
	}
	else if ((options["sim"] == "no") && (options["dir"] == "down"))
	{
		return VATA::CheckDownwardInclusion(smaller, bigger);
	}
	else if ((options["sim"] == "yes") && (options["dir"] == "up"))
	{
		return VATA::CheckUpwardInclusionWithSimulation(smaller, bigger);
	}
	else if ((options["sim"] == "yes") && (options["dir"] == "down"))
	{
		return VATA::CheckDownwardInclusionWithSimulation(smaller, bigger);
	}
	else
	{
		throw std::runtime_error("Invalid options for inclusion: " +
			Convert::ToString(options));
	}
}

template <class Automaton>
VATA::AutBase::StateBinaryRelation ComputeSimulation(const Automaton& aut,
	const Arguments& args)
{
	if (!args.pruneUseless)
	{
		throw std::runtime_error("Simulation can only be computed for "
			"automata with pruned useless states!");
	}

	// insert default values
	Options options = args.options;
	options.insert(std::make_pair("dir", "up"));

	if (options["dir"] == "up")
	{
		return VATA::ComputeUpwardSimulation(aut);
	}
	else if (options["dir"] == "down")
	{
		return VATA::ComputeDownwardSimulation(aut);
	}
	else
	{
		throw std::runtime_error("Invalid options for simulation: " +
			Convert::ToString(options));
	}
}

#endif
