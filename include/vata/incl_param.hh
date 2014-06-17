/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the InclParam structure
 *
 *****************************************************************************/

#ifndef _VATA_INCL_PARAM_
#define _VATA_INCL_PARAM_

// VATA headers
#include <vata/aut_base.hh>

namespace VATA
{
	/**
	 * @brief  Structure containing parameters for inclusion checking functions
	 *
	 * @note  Uses bit fields so take care!
	 */
	class InclParam
	{
	public:  // data types

		enum class e_algorithm
		{
			antichains,
			congruences
		};

		enum class e_direction
		{
			downward,
			upward
		};

		enum class e_search_order
		{
			breadth,
			depth
		};

		typedef unsigned TOptions;

	private: // constants

		/// 0 ... antichains (default),       1 ... bisimulation up-to congruence
		static const unsigned FLAG_MASK_ALGORITHM              = 1 << 0;
		/// 0 ... upward algorithm (default), 1 ... downward algorithm
		static const unsigned FLAG_MASK_DIRECTION              = 1 << 1;
		/// 0 ... do not use (default),       1 ... use caching of implications
		static const unsigned FLAG_MASK_DOWNWARD_CACHE_IMPL    = 1 << 2;
		/// 0 ... nonrecursive (default),     1 ... recursive implementation
		static const unsigned FLAG_MASK_RECURSIVE              = 1 << 3;
		/// 0 ... do not use (default),       1 ... use simulation
		static const unsigned FLAG_MASK_SIMULATION             = 1 << 4;
		/// 0 ... depth-first search (default), 1 ... breadth-first search
		static const unsigned FLAG_MASK_SEARCH_ORDER           = 1 << 5;
		/// 0 ... equivalence checking no (default), 1 ... yes
		static const unsigned FLAG_MASK_EQUIV                  = 1 << 6;

	public:  // constants

		static const unsigned ANTICHAINS_NOSIM = 0
			;

		static const unsigned ANTICHAINS_SIM = 0
			| FLAG_MASK_SIMULATION
			;

		static const unsigned ANTICHAINS_UP_NOSIM = 0
			;

		static const unsigned ANTICHAINS_UP_SIM = 0
			| FLAG_MASK_SIMULATION
			;

		static const unsigned ANTICHAINS_DOWN_NONREC_NOSIM = 0
			| FLAG_MASK_DIRECTION
			;

		static const unsigned ANTICHAINS_DOWN_NONREC_OPT_NOSIM = 0
			| FLAG_MASK_DIRECTION
			| FLAG_MASK_DOWNWARD_CACHE_IMPL
			;

		static const unsigned ANTICHAINS_DOWN_NONREC_SIM = 0
			| FLAG_MASK_DIRECTION
			| FLAG_MASK_SIMULATION
			;

		static const unsigned ANTICHAINS_DOWN_NONREC_OPT_SIM = 0
			| FLAG_MASK_DIRECTION
			| FLAG_MASK_DOWNWARD_CACHE_IMPL
			| FLAG_MASK_SIMULATION
			;

		static const unsigned ANTICHAINS_DOWN_REC_NOSIM = 0
			| FLAG_MASK_DIRECTION
			| FLAG_MASK_RECURSIVE
			;

		static const unsigned ANTICHAINS_DOWN_REC_OPT_NOSIM = 0
			| FLAG_MASK_DIRECTION
			| FLAG_MASK_DOWNWARD_CACHE_IMPL
			| FLAG_MASK_RECURSIVE
			;

		static const unsigned ANTICHAINS_DOWN_REC_SIM = 0
			| FLAG_MASK_DIRECTION
			| FLAG_MASK_RECURSIVE
			| FLAG_MASK_SIMULATION
			;

		static const unsigned ANTICHAINS_DOWN_REC_OPT_SIM = 0
			| FLAG_MASK_DIRECTION
			| FLAG_MASK_DOWNWARD_CACHE_IMPL
			| FLAG_MASK_RECURSIVE
			| FLAG_MASK_SIMULATION
			;

		static const unsigned CONGR_DEPTH_NOSIM = 0
			| FLAG_MASK_ALGORITHM
			;

		static const unsigned CONGR_DEPTH_SIM = 0
			| FLAG_MASK_ALGORITHM
			| FLAG_MASK_SIMULATION
			;

		static const unsigned CONGR_BREADTH_NOSIM = 0
			| FLAG_MASK_ALGORITHM
			| FLAG_MASK_SEARCH_ORDER
			;

		static const unsigned CONGR_DEPTH_EQUIV_NOSIM = 0
			| FLAG_MASK_ALGORITHM
			| FLAG_MASK_EQUIV
			;

		static const unsigned CONGR_BREADTH_EQUIV_NOSIM = 0
			| FLAG_MASK_ALGORITHM
			| FLAG_MASK_EQUIV
			| FLAG_MASK_SEARCH_ORDER
			;

	private: // data members

		/**
		 * @brief  Bit field for the options
		 */
		TOptions flags_;

		/**
		 * @brief  The simulation relation (if present)
		 */
		const AutBase::StateDiscontBinaryRelation* simulation_;

	public:   // methods

		InclParam() :
			flags_(0),
			simulation_(nullptr)
		{ }

		void SetAlgorithm(e_algorithm alg)
		{
			switch (alg)
			{
				case e_algorithm::antichains:   flags_ &= ~FLAG_MASK_ALGORITHM; break;
				case e_algorithm::congruences:  flags_ |=  FLAG_MASK_ALGORITHM; break;
				default: assert(false);
			}
		}

		e_algorithm GetAlgorithm() const
		{
			if (flags_ & FLAG_MASK_ALGORITHM)
			{
				return e_algorithm::congruences;
			}
			else
			{
				return e_algorithm::antichains;
			}
		}

		void SetDirection(e_direction dir)
		{
			switch (dir)
			{
				case e_direction::upward:    flags_ &= ~FLAG_MASK_DIRECTION; break;
				case e_direction::downward:  flags_ |=  FLAG_MASK_DIRECTION; break;
				default: assert(false);
			}
		}

		e_direction GetDirection() const
		{
			if (flags_ & FLAG_MASK_DIRECTION)
			{
				return e_direction::downward;
			}
			else
			{
				return e_direction::upward;
			}
		}

		void SetUseRecursion(bool useRec)
		{
			if (useRec)
			{
				flags_ |=  FLAG_MASK_RECURSIVE;
			}
			else
			{
				flags_ &= ~FLAG_MASK_RECURSIVE;
			}
		}

		bool GetUseRecursion() const
		{
			return flags_ & FLAG_MASK_RECURSIVE;
		}

		void SetUseDownwardCacheImpl(bool useCacheImpl)
		{
			if (useCacheImpl)
			{
				flags_ |=  FLAG_MASK_DOWNWARD_CACHE_IMPL;
			}
			else
			{
				flags_ &= ~FLAG_MASK_DOWNWARD_CACHE_IMPL;
			}
		}

		bool GetUseDownwardCacheImpl() const
		{
			return flags_ & FLAG_MASK_DOWNWARD_CACHE_IMPL;
		}

		void SetUseSimulation(bool useSimulation)
		{
			if (useSimulation)
			{
				flags_ |=  FLAG_MASK_SIMULATION;
			}
			else
			{
				flags_ &= ~FLAG_MASK_SIMULATION;
			}
		}

		bool GetUseSimulation() const
		{
			return flags_ & FLAG_MASK_SIMULATION;
		}

		TOptions GetOptions() const
		{
			return flags_;
		}

		const AutBase::StateDiscontBinaryRelation& GetSimulation() const
		{
			assert(nullptr != simulation_);
			return *simulation_;
		}

		void SetSimulation(const AutBase::StateDiscontBinaryRelation* sim)
		{
			assert(nullptr != sim);
			simulation_ = sim;
		}

		void SetSearchOrder(e_search_order order)
		{
			switch (order)
			{
				case e_search_order::depth:   flags_ &= ~FLAG_MASK_SEARCH_ORDER; break;
				case e_search_order::breadth: flags_ |=  FLAG_MASK_SEARCH_ORDER; break;
				default: assert(false);
			}
		}

		e_search_order GetSearchOrder() const
		{
			if (flags_ & FLAG_MASK_SEARCH_ORDER)
			{
				return e_search_order::breadth;
			}
			else
			{
				return e_search_order::depth;
			}
		}

		void SetEquivalence(bool equiv)
		{
			if (equiv)
			{
				flags_ |= FLAG_MASK_EQUIV;
			}
			else
			{
				flags_ &= ~FLAG_MASK_EQUIV;
			}
		}

		bool GetEquivalence() const
		{
			if (flags_ & FLAG_MASK_EQUIV)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		std::string toString() const;
	};
}

#endif
