/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for inclusion checking functions of ExplicitTreeAut.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_HH_
#define _VATA_EXPLICIT_TREE_HH_

namespace VATA
{
	/**
	 * @brief  Dispatcher for calling correct inclusion checking function
	 *
	 * This function is a dispatcher that calls a proper inclusion checking
	 * function between @p smaller and @p bigger according to the parameters in @p
	 * params.
	 *
	 * @param[in]  smaller  The smaller automaton
	 * @param[in]  bigger   The bigger automaton
	 * @param[in]  params   Parameters for the inclusion (can be @p nullptr for
	 *                      the default parameters)
	 *
	 * @returns  @p true if the language of @p smaller is a subset of the language
	 *           of @p bigger, @p false otherwise
	 */
	template <class SymbolType>
	bool CheckInclusion(
		const ExplicitTreeAut<SymbolType>&     smaller,
		const ExplicitTreeAut<SymbolType>&     bigger,
		const VATA::InclParam&                 params)
	{
		ExplicitTreeAut<SymbolType> newSmaller;
		ExplicitTreeAut<SymbolType> newBigger;
		typename AutBase::StateType states;

			newSmaller = smaller;
			newBigger = bigger;

			states = AutBase::SanitizeAutsForInclusion(newSmaller, newBigger);
			ExplicitTreeAut<SymbolType> unionAut = VATA::UnionDisjointStates(newSmaller, newBigger);
			AutBase::StateBinaryRelation sim = ComputeUpwardSimulation(unionAut, states);

		switch (params.GetOptions())
		{
			case InclParam::ANTICHAINS_UP_NOSIM:
			{
				return ExplicitUpwardInclusion::Check(newSmaller, newBigger,
					Util::Identity(states));
			}

			case InclParam::ANTICHAINS_UP_SIM:
			{
				return ExplicitUpwardInclusion::Check(newSmaller, newBigger,
					sim);
			}

			case InclParam::ANTICHAINS_DOWN_NONREC_NOSIM:
			{
				return ExplicitDownwardInclusion::Check(newSmaller, newBigger,
					Util::Identity(states));
			}

			case InclParam::ANTICHAINS_DOWN_NONREC_SIM:
			{
				return ExplicitDownwardInclusion::Check(smaller, bigger,
					params.GetSimulation());
			}

			case InclParam::ANTICHAINS_DOWN_REC_NOSIM:
			{
				return CheckDownwardTreeInclusion<ExplicitTreeAut<SymbolType>,
					VATA::DownwardInclusionFunctor>(newSmaller, newBigger,
						Util::Identity(states));
			}

			case InclParam::ANTICHAINS_DOWN_REC_OPT_NOSIM:
			{
				return CheckDownwardTreeInclusion<ExplicitTreeAut<SymbolType>,
					VATA::OptDownwardInclusionFunctor>(newSmaller, newBigger,
						Util::Identity(states));
			}

			case InclParam::ANTICHAINS_DOWN_REC_SIM:
			{
				return CheckDownwardTreeInclusion<ExplicitTreeAut<SymbolType>,
					VATA::DownwardInclusionFunctor>(smaller, bigger,
						params.GetSimulation());
			}

			case InclParam::ANTICHAINS_DOWN_REC_OPT_SIM:
			{
				return CheckDownwardTreeInclusion<ExplicitTreeAut<SymbolType>,
					VATA::OptDownwardInclusionFunctor>(smaller, bigger,
						params.GetSimulation());
			}

			default:
			{
				throw std::runtime_error("Unimplemented inclusion:\n" +
					params.toString());
			}
		}
	}
}

#endif
