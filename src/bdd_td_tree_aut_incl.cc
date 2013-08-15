/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of inclusion on BDD top-down tree automata
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

#include "bdd_td_tree_aut_core.hh"

#include "down_tree_incl_fctor.hh"
#include "down_tree_opt_incl_fctor.hh"
#include "tree_incl_down.hh"

using VATA::BDDTDTreeAutCore;

typedef VATA::AutBase::StateType StateType;


bool BDDTDTreeAutCore::CheckInclusion(
	const BDDTDTreeAutCore&     smaller,
	const BDDTDTreeAutCore&     bigger,
	const VATA::InclParam&      params)
{
	BDDTDTreeAutCore newSmaller;
	BDDTDTreeAutCore newBigger;
	typename AutBase::StateType states = static_cast<typename AutBase::StateType>(-1);

	if (!params.GetUseSimulation())
	{
		newSmaller = smaller;
		newBigger = bigger;

		states = AutBase::SanitizeAutsForInclusion(newSmaller, newBigger);
	}

	switch (params.GetOptions())
	{
		case InclParam::ANTICHAINS_DOWN_REC_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			return CheckDownwardTreeInclusion<BDDTDTreeAutCore,
				VATA::DownwardInclusionFunctor>(newSmaller, newBigger,
					Util::Identity(states));
		}

		case InclParam::ANTICHAINS_DOWN_REC_OPT_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			return CheckDownwardTreeInclusion<BDDTDTreeAutCore,
				VATA::OptDownwardInclusionFunctor>(newSmaller, newBigger,
					Util::Identity(states));
		}

		case InclParam::ANTICHAINS_DOWN_REC_SIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) == states);

			return CheckDownwardTreeInclusion<BDDTDTreeAutCore,
				VATA::DownwardInclusionFunctor>(smaller, bigger,
					params.GetSimulation());
		}

		case InclParam::ANTICHAINS_DOWN_REC_OPT_SIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) == states);

			return CheckDownwardTreeInclusion<BDDTDTreeAutCore,
				VATA::OptDownwardInclusionFunctor>(smaller, bigger,
					params.GetSimulation());
		}

		default:
		{
			throw NotImplementedException("Inclusion:\n" + params.toString());
		}
	}
}
