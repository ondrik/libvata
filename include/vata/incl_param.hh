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

#include <vata/util/convert.hh>

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

		typedef unsigned TOptions;

		typedef VATA::Util::Convert Convert;

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

	public:  // constants

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

	private: // data members

		/**
		 * @brief  Bit field for the options
		 */
		TOptions flags_;

		/**
		 * @brief  The simulation relation (if present)
		 */
		const AutBase::StateBinaryRelation* simulation_;

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

		const AutBase::StateBinaryRelation& GetSimulation() const
		{
			assert(nullptr != simulation_);
			return *simulation_;
		}

		void SetSimulation(const AutBase::StateBinaryRelation* sim)
		{
			assert(nullptr != sim);
			simulation_ = sim;
		}

		std::string toString() const
		{
			std::string result;

			result += "Algorithm: ";
			switch (this->GetAlgorithm())
			{
				case e_algorithm::antichains:  result += "Antichains"; break;
				case e_algorithm::congruences: result += "Congruence"; break;
				default: assert(false);
			}

			result += "\n";
			result += "Direction: ";
			switch (this->GetDirection())
			{
				case e_direction::downward: result += "Downward"; break;
				case e_direction::upward:   result += "Upward";   break;
				default: assert(false);
			}

			result += "\n";
			result += "Downward inclusion caching implications: ";
			result += Convert::ToString(this->GetUseDownwardCacheImpl()) + "\n";

			result += "Recursive algorithm: ";
			result += Convert::ToString(this->GetUseRecursion()) + "\n";

			result += "Use simulation: ";
			result += Convert::ToString(this->GetUseSimulation()) + "\n";

			return result;
		}
	};
}

#endif
