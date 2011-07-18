/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    A data structure for a triple of values
 *
 *****************************************************************************/

#ifndef _VATA_TRIPLE_HH_
#define _VATA_TRIPLE_HH_

// VATA headers
#include	<vata/vata.hh>

// Boost library headers
#include <boost/functional/hash.hpp>


namespace VATA
{
	namespace Util
	{
		template <
			typename T1,
			typename T2,
			typename T3
		>
		struct Triple;
	}
}


template <
	typename T1,
	typename T2,
	typename T3
>
struct VATA::Util::Triple
{
	T1 first;
	T2 second;
	T3 third;

	Triple(const T1& parFirst, const T2& parSecond, const T3& parThird)
		: first(parFirst),
			second(parSecond),
			third(parThird)
	{ }

	inline bool operator==(const Triple& rhs) const
	{
		return ((first == rhs.first) && (second == rhs.second) &&
			(third == rhs.third));
	}

	/**
	 * @brief  Hasher structure for a triple of keys
	 *
	 * This class is a hasher for a triple of keys.
	 */
	struct Hasher
	{
		inline size_t operator()(const Triple& key) const
		{
			size_t seed  = 0;
			boost::hash_combine(seed, key.first);
			boost::hash_combine(seed, key.second);
			boost::hash_combine(seed, key.third);
			return seed;
		}
	};
};


#endif
