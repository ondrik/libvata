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
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_bu_tree_aut_op.hh>
#include <vata/bdd_bu_tree_aut_incl.hh>
#include <vata/bdd_td_tree_aut_incl.hh>
#include <vata/tree_incl_up.hh>

using VATA::AutBase;
using VATA::BDDBottomUpTreeAut;
using VATA::BDDTopDownTreeAut;
using VATA::Util::Convert;


bool VATA::CheckInclusion(
	const BDDBottomUpTreeAut&   smaller,
	const BDDBottomUpTreeAut&   bigger,
	const VATA::InclParam&      params)
{
	BDDBottomUpTreeAut newSmaller;
	BDDBottomUpTreeAut newBigger;
	typename AutBase::StateType states = static_cast<typename AutBase::StateType>(-1);

	if (InclParam::e_direction::downward == params.GetDirection())
	{	// for the other direction translate the automaton to the top-down encoding
		return CheckInclusion(smaller.GetTopDownAut(), bigger.GetTopDownAut(), params);
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

			return CheckUpwardTreeInclusion<BDDBottomUpTreeAut,
				VATA::UpwardInclusionFunctor>(newSmaller, newBigger,
					Util::Identity(states));
		}

		case InclParam::ANTICHAINS_UP_SIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) == states);

			return CheckUpwardTreeInclusion<BDDBottomUpTreeAut,
				VATA::UpwardInclusionFunctor>(smaller, bigger,
					params.GetSimulation());
		}

		default:
		{
			throw std::runtime_error("Unimplemented inclusion:\n" +
				params.toString());
		}
	}
}
