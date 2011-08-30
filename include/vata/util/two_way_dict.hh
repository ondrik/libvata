/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a two way dictionary.
 *
 *****************************************************************************/

#ifndef _VATA_TWO_WAY_DICT_
#define _VATA_TWO_WAY_DICT_

// VATA headers
#include <vata/vata.hh>


namespace VATA
{
	namespace Util
	{
		template
		<
			typename T1,
			typename T2,
			class Cont1,
			class Cont2
		>
		class TwoWayDict;
	}
}


/**
 * @brief   Two-way dictionary
 *
 * This class can be used as a two-way dictionary for two different types, @p
 * Type1 and @p Type2
 *
 * @tparam  Type1   First type
 * @tparam  Type2   Second type
 */
template
<
	typename T1,
	typename T2,
	class Cont1 = std::map<T1, T2>,
	class Cont2 = std::map<T2, T1>
>
class VATA::Util::TwoWayDict
{
public:   // Public data types

	typedef T1 Type1;
	typedef T2 Type2;

private:  // Private data types

	typedef Cont1 MapFwdType;
	typedef Cont2 MapBwdType;

public:   // Public data types

	typedef typename MapFwdType::const_iterator ConstIteratorFwd;
	typedef typename MapBwdType::const_iterator ConstIteratorBwd;

	typedef typename MapFwdType::const_iterator const_iterator;

private:  // Private data members

	MapFwdType fwdMap_;
	MapBwdType bwdMap_;

public:   // Public methods

	TwoWayDict() :
		fwdMap_(),
		bwdMap_()
	{ }

	inline const Type2& TranslateFwd(const Type1& t1) const
	{
		ConstIteratorFwd itFwd;
		if ((itFwd = fwdMap_.find(t1)) == EndFwd())
		{	// in case the value that should be stored there is not
			assert(false);      // fail gracefully
		}

		return itFwd->second;
	}

	inline const Type1& TranslateBwd(const Type2& t2) const
	{
		ConstIteratorBwd itBwd;
		if ((itBwd = bwdMap_.find(t2)) == EndBwd())
		{	// in case the value that should be stored there is not
			assert(false);      // fail gracefully
		}

		return itBwd->second;
	}

	inline const_iterator find(const Type1& t1) const
	{
		return FindFwd(t1);
	}

	inline ConstIteratorFwd FindFwd(const Type1& t1) const
	{
		return fwdMap_.find(t1);
	}

	inline ConstIteratorBwd FindBwd(const Type2& t2) const
	{
		return bwdMap_.find(t2);
	}

	inline const_iterator begin() const
	{
		return BeginFwd();
	}

	inline const_iterator end() const
	{
		return EndFwd();
	}

	inline ConstIteratorFwd BeginFwd() const
	{
		return fwdMap_.begin();
	}

	inline ConstIteratorBwd BeginBwd() const
	{
		return bwdMap_.begin();
	}

	inline ConstIteratorFwd EndFwd() const
	{
		return fwdMap_.end();
	}

	inline ConstIteratorBwd EndBwd() const
	{
		return bwdMap_.end();
	}

	inline std::pair<ConstIteratorFwd, bool> insert(
		const std::pair<Type1, Type2>& value)
	{
		return Insert(value);
	}

	inline std::pair<ConstIteratorFwd, bool> Insert(
		const std::pair<Type1, Type2>& value)
	{
		auto resPair = fwdMap_.insert(value);
		if (!(resPair.second))
		{	// in case there is already some forward mapping for given value
			assert(false);      // fail gracefully
		}

		if (!(bwdMap_.insert(std::make_pair(value.second, value.first)).second))
		{	// in case there is already some backward mapping for given value
			assert(false);      // fail gracefully
		}

		return resPair;
	}

	TwoWayDict Union(const TwoWayDict& rhs) const
	{
		TwoWayDict result = *this;

		// copy all pairs
		for (ConstIteratorFwd itRhs = rhs.BeginFwd(); itRhs != rhs.EndFwd(); ++itRhs)
		{
			if ((result.fwdMap_.find(itRhs->first) != result.fwdMap_.end()) ||
				(result.bwdMap_.find(itRhs->second) != result.bwdMap_.end()))
			{	// in case the first or the second component is already in the dictionary
				assert(false);    // fail gracefully
			}

			result.Insert(*itRhs);
		}

		return result;
	}

	friend std::ostream& operator<<(std::ostream& os,
		const TwoWayDict& dict)
	{
		return (os << Convert::ToString(dict.fwdMap_));
	}
};

#endif
