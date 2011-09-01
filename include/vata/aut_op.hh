/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for operations on automata.
 *
 *****************************************************************************/

#ifndef _VATA_AUT_OP_HH_
#define _VATA_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>

namespace VATA
{
	/**
	 * @brief  Computes union of two automata
	 *
	 * This function computes an automaton such that its language is the union
	 * of languages of the input automata.
	 *
	 * @param[in]         lhs  Left-hand side automaton
	 * @param[in]         rhs  Right-hand side automaton
	 * @param[out] pTranslMap  Translation map of states of the union automaton.
	 *                         In case @p nullptr is given, the parameter is
	 *                         ignored.
	 *
	 * @return  Union automaton
	 *
	 * @tparam  Aut  Automaton type
	 */
	template <class Aut>
	Aut Union(const Aut& lhs, const Aut& rhs,
		AutBase::StateToStateMap* pTranslMap = nullptr);

	/**
	 * @brief  Computes intersection of two automata
	 *
	 * This function computes an automaton such that its language is the
	 * intersection of languages of the input automata.
	 *
	 * @param[in]  lhs         Left-hand side automaton
	 * @param[in]  rhs         Right-hand side automaton
	 * @param[out] pTranslMap  Translation map of states of the product automaton.
	 *                         In case @p nullptr is given, the parameter is
	 *                         ignored.
	 *
	 * @return  Intersection automaton
	 *
	 * @tparam  Aut  Automaton type
	 */
	template <class Aut>
	Aut Intersection(const Aut& lhs, const Aut& rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr);

	/**
	 * @brief  Removes unreachable states
	 *
	 * The function removes unreachable states from given automaton.
	 *
	 * @param[in]   aut         Input automaton
	 * @param[out]  pTranslMap  The state-to-state translation map that gives
	 *                          which state is translated to which. In case @p
	 *                          nullptr is given, the parameter is ignored.
	 *
	 * @return  Automaton without unreachable states
	 *
	 * @tparam  Aut  Automaton type
	 */
	template <class Aut>
	Aut RemoveUnreachableStates(const Aut& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr);

	/**
	 * @brief  Removes useless states
	 *
	 * The function removes useless states, which are either unreachable states,
	 * or states from which it is not possible to terminate.
	 *
	 * @param[in]   aut         Input automaton
	 * @param[out]  pTranslMap  The state-to-state translation map that gives
	 *                          which state is translated to which. In case @p
	 *                          nullptr is given, the parameter is ignored.
	 *
	 * @return  Automaton without useless states
	 *
	 * @see VATA::RemoveUnreachableStates
	 *
	 * @tparam  Aut  Automaton type
	 */
	template <class Aut>
	Aut RemoveUselessStates(const Aut& aut,
		AutBase::StateToStateMap* pTranslMap = nullptr);

	/**
	 * @brief  Checks language inclusion of two automata
	 *
	 * This function checks language inclusion of two automata, i.e., whether
	 * L(@p smaller) is a subset of L(@p bigger).
	 *
	 * @param[in]  smaller  The automaton with the @e smaller language
	 * @param[in]  bigger   The automaton with the @e bigger language
	 *
	 * @return  @p true if L(@p smaller) is a subset of L(@p bigger), @p false
	 *          otherwise
	 *
	 * @tparam  Aut  Automaton type
	 */
	template <class Aut>
	bool CheckInclusion(const Aut& smaller, const Aut& bigger);

	/**
	 * @brief  Checks language inclusion of two automata w/o useless states
	 *
	 * This function checks language inclusion of two automata, i.e., whether
	 * L(@p smaller) is a subset of L(@p bigger). It is necessary that the
	 * automata do not contain useless states.
	 *
	 * @param[in]  smaller  The automaton with the @e smaller language
	 * @param[in]  bigger   The automaton with the @e bigger language
	 *
	 * @return  @p true if L(@p smaller) is a subset of L(@p bigger), @p false
	 *          otherwise
	 *
	 * @tparam  Aut  Automaton type
	 */
	template <class Aut>
	bool CheckInclusionNoUseless(const Aut& smaller, const Aut& bigger);
}

#endif
