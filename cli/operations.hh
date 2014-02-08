/*****************************************************************************
 *  VATA Tree Automata Library
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
using VATA::InclParam;
using VATA::SimParam;

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
	options.insert(std::make_pair("rec", "no"));
	options.insert(std::make_pair("alg", "antichains"));
	options.insert(std::make_pair("order", "depth"));

	std::runtime_error optErrorEx("Invalid options for inclusion: " +
			Convert::ToString(options));

	AutBase::StateType states = AutBase::SanitizeAutsForInclusion(smaller, bigger);

	/****************************************************************************
	 *                        Parsing of input parameters
	 ****************************************************************************/

	// parameters for inclusion
	InclParam ip;

	// the algorithm used
	if (options["alg"] == "antichains")
	{
		ip.SetAlgorithm(InclParam::e_algorithm::antichains);
	}
	else if (options["alg"] == "congr")
	{
		ip.SetAlgorithm(InclParam::e_algorithm::congruences);
	}
	else { throw optErrorEx; }

	// direction of the algorithm
	if (options["dir"] == "up")
	{
		ip.SetDirection(InclParam::e_direction::upward);
	}
	else if (options["dir"] == "down")
	{
		ip.SetDirection(InclParam::e_direction::downward);
	}
	else { throw optErrorEx; }

	// recursive/nonrecursive version
	if (options["rec"] == "yes")
	{
		ip.SetUseRecursion(true);
	}
	else if (options["rec"] == "no")
	{
		ip.SetUseRecursion(false);
	}
	else { throw optErrorEx; }

	// caching of implications
	if (options["optC"] == "no")
	{
		ip.SetUseDownwardCacheImpl(false);
	}
	else if (options["optC"] == "yes")
	{
		ip.SetUseDownwardCacheImpl(true);
	}
	else { throw optErrorEx; }

	// use simulation?
	if (options["sim"] == "no")
	{
		ip.SetUseSimulation(false);
	}
	else if (options["sim"] == "yes")
	{
		ip.SetUseSimulation(true);
	}
	else { throw optErrorEx; }

	if (options["order"] == "depth")
	{
		ip.SetSearchOrder(InclParam::e_search_order::depth);
	}
	else if (options["order"] == "breadth")
	{
		ip.SetSearchOrder(InclParam::e_search_order::breadth);
	}
	else {throw optErrorEx; }

	bool incl_sim_time = false;
	if (options["timeS"] == "no")
	{
		incl_sim_time = false;
	}
	else if (options["timeS"] == "yes")
	{
		incl_sim_time = true;
	}
	else { throw optErrorEx; }

	/****************************************************************************
	 *                            Additional handling
	 ****************************************************************************/

	// set the timer
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startTime);

	AutBase::StateBinaryRelation sim;

	if (ip.GetUseSimulation())
	{	// if simulation is desired, then compute it here!
		//Automaton unionAut = Automaton::UnionDisjointStates(smaller, bigger);
		Automaton unionAut;

		if (ip.GetAlgorithm() == InclParam::e_algorithm::congruences)
		{
			smaller = Automaton::UnionDisjointStates(smaller, bigger);
		}
		else
		{
			unionAut = Automaton::UnionDisjointStates(smaller, bigger);
		}

		if (InclParam::e_direction::upward == ip.GetDirection())
		{	// for upward algorithm compute the upward simulation
			SimParam sp;
			sp.SetRelation(VATA::SimParam::e_sim_relation::TA_UPWARD);
			sp.SetNumStates(states);
			sim = unionAut.ComputeSimulation(sp);
			ip.SetSimulation(&sim);
		}
		else if (InclParam::e_direction::downward == ip.GetDirection())
		{	// for downward algorithm, compute the downward simulation
			SimParam sp;
			sp.SetRelation(VATA::SimParam::e_sim_relation::TA_DOWNWARD);
			sp.SetNumStates(states);
			sim = unionAut.ComputeSimulation(sp);
			ip.SetSimulation(&sim);
		}
		else
		{
			assert(false);         // fail gracefully
		}
	}

	if (!incl_sim_time)
	{	// if the simulation time is not to be included in the total time
		// reset the timer
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startTime);
	}

	return Automaton::CheckInclusion(smaller, bigger, ip);
}

template <
	class Automaton,
	class StringToStateMap,
	class StateToStateMap>
VATA::AutBase::StateBinaryRelation ComputeSimulation(
	Automaton                aut,
	const Arguments&         args,
	const StringToStateMap   /* index */,    // TODO: why is this here?
	StateToStateMap&         translMap)
{
// TODO: Why was this here?
//	if (!args.pruneUseless)
//	{
//		throw std::runtime_error("Simulation can only be computed for "
//			"automata with pruned useless states!");
//	}

	using StateType = AutBase::StateType;

	using StateToStateTranslator = AutBase::StateToStateTranslWeak;

	// insert default values
	Options options = args.options;
	options.insert(std::make_pair("dir", "down"));

	StateType stateCnt = 0;
	StateToStateTranslator stateTransl(translMap,
		[&stateCnt](const StateType&){return stateCnt++;});

	aut = aut.ReindexStates(stateTransl);

	// AutBase::StateType states = AutBase::SanitizeAutForSimulation(aut,
	// 	stateCnt, stateTransl);

	SimParam sp;
	sp.SetNumStates(stateCnt);
	if (options["dir"] == "up")
	{
		sp.SetRelation(VATA::SimParam::e_sim_relation::TA_UPWARD);
	}
	else if (options["dir"] == "down")
	{
		sp.SetRelation(VATA::SimParam::e_sim_relation::TA_DOWNWARD);
	}
	else
	{
		throw std::runtime_error("Invalid options for simulation: " +
			Convert::ToString(options));
	}

	return aut.ComputeSimulation(sp);
}

template <class Automaton>
Automaton ComputeReduction(
	Automaton           aut,
	const Arguments&    args)
{
	// insert default values
	Options options = args.options;
	options.insert(std::make_pair("dir", "down"));

	if (options["dir"] == "up")
	{
		throw std::runtime_error("Unimplemented.");
	}
	else if (options["dir"] == "down")
	{
		return aut.Reduce();
	}
	else
	{
		throw std::runtime_error("Invalid options for simulation: " +
			Convert::ToString(options));
	}
}

template <class Automaton>
bool CheckEquiv(Automaton smaller, Automaton bigger, const Arguments& args)
{
	// insert default values
	Options options = args.options;
	options.insert(std::make_pair("order", "depth"));

	// parameters for inclusion
	InclParam ip;

	std::runtime_error optErrorEx("Invalid options for equivalence: " +
			Convert::ToString(options));

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startTime); // set the timer

	ip.SetEquivalence(true);
	ip.SetAlgorithm(InclParam::e_algorithm::congruences);

	if (options["order"] == "depth")
	{
		ip.SetSearchOrder(InclParam::e_search_order::depth);
	}
	else if (options["order"] == "breadth")
	{
		ip.SetSearchOrder(InclParam::e_search_order::breadth);
	}
	else { throw optErrorEx; }

	// TODO: change
	assert(false);
	return Automaton::CheckInclusion(smaller, bigger, ip);
}
#endif
