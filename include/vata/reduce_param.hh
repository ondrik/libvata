/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2014  Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the ReduceParam structure
 *
 *****************************************************************************/

#ifndef _REDUCE_PARAM_HH_
#define _REDUCE_PARAM_HH_

#include <cassert>
#include <string>

namespace VATA
{
	class ReduceParam
	{
	public:   // data types

		/**
		 * @brief  The relation according to which the reduction will be done
		 */
		enum class e_reduce_relation
		{
			TA_DOWNWARD
		};

	private:  // data members

		/// the relation to be computed
		e_reduce_relation relation_ = static_cast<e_reduce_relation>(-1);

	public:   // methods

		void SetRelation(e_reduce_relation rel)
		{
			relation_ = rel;
		}

		e_reduce_relation GetRelation() const
		{
			return relation_;
		}

		std::string toString() const
		{
			std::string result = "ReduceParam relation: ";
			switch (this->GetRelation())
			{
				case e_reduce_relation::TA_DOWNWARD:
				{
					result += "TA_DOWNWARD";
					break;
				}
				default:
				{
					assert(false);     // fail gracefully
				}
			}

			return result;
		}
	};
}

#endif /* _REDUCE_PARAM_HH_ */
