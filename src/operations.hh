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
using VATA::AutBase;

extern timespec startTime;

template <class Automaton>
bool CheckInclusion(Automaton smaller, Automaton bigger, const Arguments& args)
{
	// insert default values
	Options options = args.options;
	options.insert(std::make_pair("sim", "no"));
	options.insert(std::make_pair("dir", "up"));
	options.insert(std::make_pair("optC", "no"));
	options.insert(std::make_pair("timeS", "yes"));

	std::runtime_error optErrorEx("Invalid options for inclusion: " +
			Convert::ToString(options));

	AutBase::StateType states = AutBase::SanitizeAutsForInclusion(smaller, bigger);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startTime);     // set the timer

	if (options["sim"] == "no")
	{
		VATA::Util::Identity ident(states);
		if (options["dir"] == "up")
		{
			return VATA::CheckUpwardInclusionWithPreorder(smaller, bigger, ident);
		}
		else if (options["dir"] == "down")
		{
			if (options["optC"] == "no")
			{
				return VATA::CheckDownwardInclusionWithPreorder(smaller, bigger, ident);
			}
			else if (options["optC"] == "yes")
			{
				return VATA::CheckOptDownwardInclusionWithPreorder(
					smaller, bigger, ident);
			}
			else
			{
				throw optErrorEx;
			}
		}
		else
		{
			throw optErrorEx;
		}
	}
	else if (options["sim"] == "yes")
	{
		Automaton unionAut = VATA::UnionDisjunctStates(smaller, bigger);

		if (options["dir"] == "up")
		{
			AutBase::StateBinaryRelation sim = ComputeUpwardSimulation(unionAut, states);

			if (options["timeS"] == "no")
			{
				clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startTime);     // set the timer
			}
			else if (options["timeS"] == "yes")
			{ }
			else
			{
				throw optErrorEx;
			}

			return VATA::CheckUpwardInclusionWithPreorder(smaller, bigger, sim);
		}
		else if (options["dir"] == "down")
		{
			AutBase::StateBinaryRelation sim = ComputeDownwardSimulation(unionAut, states);

			if (options["timeS"] == "no")
			{
				clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startTime);     // set the timer
			}
			else if (options["timeS"] == "yes")
			{ }
			else
			{
				throw optErrorEx;
			}

			return VATA::CheckDownwardInclusionWithPreorder(smaller, bigger, sim);
		}
		else
		{
			throw optErrorEx;
		}
	}
	else
	{
		throw optErrorEx;
	}
}

template <class Automaton>
VATA::AutBase::StateBinaryRelation ComputeSimulation(Automaton aut,
	const Arguments& args)
{
	if (!args.pruneUseless)
	{
		throw std::runtime_error("Simulation can only be computed for "
			"automata with pruned useless states!");
	}

	// insert default values
	Options options = args.options;
	options.insert(std::make_pair("dir", "down"));

	AutBase::StateType states = AutBase::SanitizeAutForSimulation(aut);

	if (options["dir"] == "up")
	{
		return VATA::ComputeUpwardSimulation(aut, states);
	}
	else if (options["dir"] == "down")
	{
		return VATA::ComputeDownwardSimulation(aut, states);
	}
	else
	{
		throw std::runtime_error("Invalid options for simulation: " +
			Convert::ToString(options));
	}
}

#endif
