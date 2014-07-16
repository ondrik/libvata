/*****************************************************************************
 *  VATA Tree Automata Library
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
#include	<vata/util/convert.hh>

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

	bool operator<(const Triple& rhs) const
	{
		// lexicographic comparison
		if (first < rhs.first)
		{
			return true;
		}
		else if (first > rhs.first)
		{
			return false;
		}

		assert(first == rhs.first);
		if (second < rhs.second)
		{
			return true;
		}
		else if (second > rhs.second)
		{
			return false;
		}

		assert(second == rhs.second);
		return third < rhs.third;
	}


	friend std::ostream& operator<<(std::ostream& os, const Triple& trip)
	{
		os << "(" <<
			VATA::Util::Convert::ToString(trip.first)  << ", " <<
			VATA::Util::Convert::ToString(trip.second) << ", " <<
			VATA::Util::Convert::ToString(trip.third)  << ")";

		return os;
	}
};

namespace VATA
{
	namespace Util
	{
		template <typename T1, typename T2, typename T3>
		inline size_t hash_value(const Triple<T1, T2, T3>& key)
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, key.first);
			boost::hash_combine(seed, key.second);
			boost::hash_combine(seed, key.third);
			return seed;
		}
	}
}

#endif
