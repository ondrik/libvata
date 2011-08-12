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

namespace VATA
{
	/**
	 * @brief  Computes union of two automata
	 *
	 * This function computes an automaton such that its language is the union
	 * of languages of the input automata.
	 *
	 * @param[in]  lhs  Left-hand side automaton
	 * @param[in]  rhs  Right-hand side automaton
	 *
	 * @return  Union automaton
	 *
	 * @tparam  Aut  Automaton type
	 */
	template <class Aut>
	Aut Union(const Aut& lhs, const Aut& rhs);

	/**
	 * @brief  Computes intersection of two automata
	 *
	 * This function computes an automaton such that its language is the
	 * intersection of languages of the input automata.
	 *
	 * @param[in]  lhs  Left-hand side automaton
	 * @param[in]  rhs  Right-hand side automaton
	 *
	 * @return  Intersection automaton
	 *
	 * @tparam  Aut  Automaton type
	 */
	template <class Aut>
	Aut Intersection(const Aut& lhs, const Aut& rhs);

	/**
	 * @brief  Removes unreachable states
	 *
	 * The function removes unreachable states from given automaton.
	 *
	 * @param[in]  aut  Input automaton
	 *
	 * @return  Automaton without unreachable states
	 *
	 * @tparam  Aut  Automaton type
	 */
	template <class Aut>
	Aut RemoveUnreachableStates(const Aut& aut);

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
}

#endif
