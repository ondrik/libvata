/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of inclusion on BDD bottom-up tree automata
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

#include "bdd_bu_tree_aut_core.hh"
#include "bdd_bu_tree_aut_incl.hh"

#include "bdd_td_tree_aut_core.hh"

#include "tree_incl_up.hh"
#include "up_tree_incl_fctor.hh"

using VATA::AutBase;
using VATA::BDDBUTreeAutCore;
using VATA::BDDTDTreeAutCore;
using VATA::Util::Convert;


bool BDDBUTreeAutCore::CheckInclusion(
	const BDDBUTreeAutCore&     smaller,
	const BDDBUTreeAutCore&     bigger,
	const VATA::InclParam&      params)
{
	BDDBUTreeAutCore newSmaller;
	BDDBUTreeAutCore newBigger;
	typename AutBase::StateType states = static_cast<typename AutBase::StateType>(-1);

	if (InclParam::e_direction::downward == params.GetDirection())
	{	// for the other direction translate the automaton to the top-down encoding
		return BDDTDTreeAutCore::CheckInclusion(smaller.GetTopDownAut(), bigger.GetTopDownAut(), params);
	}

	if (!params.GetUseSimulation())
	{
		newSmaller = smaller;
		newBigger = bigger;

		states = AutBase::SanitizeAutsForInclusion(newSmaller, newBigger);
	}

	switch (params.GetOptions())
	{
		case InclParam::ANTICHAINS_UP_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			return CheckUpwardTreeInclusion<BDDBUTreeAutCore,
				VATA::UpwardInclusionFunctor>(newSmaller, newBigger,
					Util::Identity(states));
		}

		case InclParam::ANTICHAINS_UP_SIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) == states);

			return CheckUpwardTreeInclusion<BDDBUTreeAutCore,
				VATA::UpwardInclusionFunctor>(smaller, bigger,
					params.GetSimulation());
		}

		default:
		{
			throw NotImplementedException("Inclusion:\n" +
				params.toString());
		}
	}
}
