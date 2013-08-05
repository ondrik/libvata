/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of inclusion checking functions of ExplicitTreeAut.
 *
 *****************************************************************************/

// VATA headers
#include <vata/ta_expl/explicit_tree_aut.hh>

#include "explicit_tree_incl_up.hh"
#include "explicit_tree_incl_down.hh"
#include "tree_incl_down.hh"
#include "down_tree_incl_fctor.hh"
#include "down_tree_opt_incl_fctor.hh"

using VATA::ExplicitTreeAut;
using VATA::ExplicitUpwardInclusion;

bool ExplicitTreeAut::CheckInclusion(
	const ExplicitTreeAut&                 smaller,
	const ExplicitTreeAut&                 bigger,
	const VATA::InclParam&                 params)
{
	ExplicitTreeAut newSmaller;
	ExplicitTreeAut newBigger;
	typename AutBase::StateType states = static_cast<typename AutBase::StateType>(-1);

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

			return ExplicitUpwardInclusion::Check(newSmaller, newBigger,
				Util::Identity(states));
		}

		case InclParam::ANTICHAINS_UP_SIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) == states);

			return ExplicitUpwardInclusion::Check(smaller, bigger,
				params.GetSimulation());
		}

		case InclParam::ANTICHAINS_DOWN_NONREC_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			return ExplicitDownwardInclusion::Check(newSmaller, newBigger,
				Util::Identity(states));
		}

		case InclParam::ANTICHAINS_DOWN_NONREC_SIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) == states);

			return ExplicitDownwardInclusion::Check(smaller, bigger,
				params.GetSimulation());
		}

		case InclParam::ANTICHAINS_DOWN_REC_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			return CheckDownwardTreeInclusion<ExplicitTreeAut,
				VATA::DownwardInclusionFunctor>(newSmaller, newBigger,
					Util::Identity(states));
		}

		case InclParam::ANTICHAINS_DOWN_REC_OPT_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			return CheckDownwardTreeInclusion<ExplicitTreeAut,
				VATA::OptDownwardInclusionFunctor>(newSmaller, newBigger,
					Util::Identity(states));
		}

		case InclParam::ANTICHAINS_DOWN_REC_SIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) == states);

			return CheckDownwardTreeInclusion<ExplicitTreeAut,
				VATA::DownwardInclusionFunctor>(smaller, bigger,
					params.GetSimulation());
		}

		case InclParam::ANTICHAINS_DOWN_REC_OPT_SIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) == states);

			return CheckDownwardTreeInclusion<ExplicitTreeAut,
				VATA::OptDownwardInclusionFunctor>(smaller, bigger,
					params.GetSimulation());
		}

		default:
		{
			throw NotImplementedException("Inclusion:\n" +
				params.toString());
		}
	}
}
