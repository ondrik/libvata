/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the InclParam structure
 *
 *****************************************************************************/

#ifndef _VATA_INCP_PARAM_
#define _VATA_INCP_PARAM_

namespace VATA
{
	/**
	 * @brief  Structure containing parameters for inclusion checking functions
	 *
	 * @note  Uses bit fields so take care!
	 */
	struct InclParam
	{
		/**
		 * @brief  Which algorithm to use?
		 *
		 * 0 ... antichains (default)
		 * 1 ... bisimulation up-to congruence
		 */
		unsigned opt_congruence                 :  1;

		/**
		 * @brief  Which direction of the algorithm?
		 *
		 * 0 ... upward algorithm (default)
		 * 1 ... downward algorithm
		 *
		 * @note  Only for tree automata
		 */
		unsigned opt_downward                   :  1;

		/**
		 * @brief  Use caching of implications for the downward algorithm?
		 *
		 * 0 ... do not use caching of implications (default)
		 * 1 ... use caching of implications
		 *
		 * @note  Only for tree automata and the downward algorithm
		 */
		unsigned opt_downward_cache_impl        :  1;

		/**
		 * @brief  Use recursive implementation of the downward algorithm?
		 *
		 * 0 ... do not use recursive implementation (default)
		 * 1 ... use recursive implementation
		 *
		 * @note  Only for tree automata and the downward algorithm
		 */
		unsigned opt_downward_rec               :  1;

		/**
		 * @brief  Use simulation?
		 *
		 * 0 ... do not use simulation (default)
		 * 1 ... use simulation
		 */
		unsigned opt_simulation                 :  1;


		/**
		 * @brief  The simulation relation (if present)
		 */
		const AutBase::StateBinaryRelation* simulation;

		InclParam() :
			opt_congruence(0),
			opt_downward(0),
			opt_downward_cache_impl(0),
			opt_downward_rec(0),
			opt_simulation(0),
			simulation(nullptr)
		{ }
	};
}

#endif
