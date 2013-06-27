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
		const VATA::InclParam*                 params = nullptr)
	{
		if (nullptr == params)
		{
			return CheckUpwardInclusion(smaller, bigger);
		}
		else if (!params->opt_downward && !params->opt_simulation)
		{	// upward inclusion (plain)
			return CheckUpwardInclusion(smaller, bigger);
		}
		else
		{	// unsupported stuff
			throw std::runtime_error("Unimplemented");
		}
	}

	template <class SymbolType, class Rel>
	bool CheckUpwardInclusionWithPreorder(
		const ExplicitTreeAut<SymbolType>&     smaller,
		const ExplicitTreeAut<SymbolType>&     bigger,
		const Rel&                             preorder)
	{
		return ExplicitUpwardInclusion::Check(smaller, bigger, preorder);
	}

	// Added due to FA extension
	template <class SymbolType, class Rel>
	bool CheckUpwardInclusionWithSim(const ExplicitTreeAut<SymbolType>& smaller,
		const ExplicitTreeAut<SymbolType>& bigger, const Rel& preorder) {

		return ExplicitUpwardInclusion::Check(smaller, bigger, preorder);
	}

	template <class SymbolType, class Rel>
	bool CheckOptDownwardInclusionWithPreorder(
		const ExplicitTreeAut<SymbolType>&     smaller,
		const ExplicitTreeAut<SymbolType>&     bigger,
		const Rel&                             preorder)
	{
		return CheckDownwardTreeInclusion<ExplicitTreeAut<SymbolType>,
			VATA::OptDownwardInclusionFunctor>(smaller, bigger, preorder);
	}


	template <class SymbolType, class Rel>
	bool CheckDownwardInclusionWithPreorder(
		const ExplicitTreeAut<SymbolType>&     smaller,
		const ExplicitTreeAut<SymbolType>&     bigger,
		const Rel&                             preorder)
	{
		return CheckDownwardTreeInclusion<ExplicitTreeAut<SymbolType>,
			VATA::DownwardInclusionFunctor>(smaller, bigger, preorder);
	}


	template <class SymbolType, class Rel>
	bool CheckDownwardInclusionNonRecWithPreorder(
		const ExplicitTreeAut<SymbolType>&     smaller,
		const ExplicitTreeAut<SymbolType>&     bigger,
		const Rel&                             preorder)
	{
		return ExplicitDownwardInclusion::Check(smaller, bigger, preorder);
	}


	template <class SymbolType>
	bool CheckUpwardInclusion(
		const ExplicitTreeAut<SymbolType>&     smaller,
		const ExplicitTreeAut<SymbolType>&     bigger)
	{
		ExplicitTreeAut<SymbolType> newSmaller = smaller;
		ExplicitTreeAut<SymbolType> newBigger = bigger;
		typename AutBase::StateType states =
			AutBase::SanitizeAutsForInclusion(newSmaller, newBigger);

		return CheckUpwardInclusionWithPreorder(newSmaller, newBigger,
			Util::Identity(states));
	}


	template <class SymbolType>
	bool CheckDownwardInclusion(
		const ExplicitTreeAut<SymbolType>&     smaller,
		const ExplicitTreeAut<SymbolType>&     bigger)
	{
		ExplicitTreeAut<SymbolType> newSmaller = smaller;
		ExplicitTreeAut<SymbolType> newBigger = bigger;
		typename AutBase::StateType states =
			AutBase::SanitizeAutsForInclusion(newSmaller, newBigger);

		return CheckDownwardInclusionWithPreorder(newSmaller, newBigger,
			ComputeDownwardSimulation(
				UnionDisjointStates(newSmaller, newBigger), states)
			);
	}
}

#endif
