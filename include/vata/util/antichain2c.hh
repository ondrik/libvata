/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a class representing antichain.
 *
 *****************************************************************************/

#ifndef _VATA_ANTICHAIN_HH_
#define _VATA_ANTICHAIN_HH_

// VATA headers
#include <vata/vata.hh>

namespace VATA
{
	namespace Util
	{
		template
		<
			typename Key,
			typename T,
			class Cmp,
			class CmpStrict,
			class Cont
		>
		class Antichain2C;
	}
}


template
<
	typename Key,
	typename T,
	class Cmp,
	class CmpStrict,
	class Cont = std::unordered_multimap<Key, T>
>
class VATA::Util::Antichain2C
{
private:  // data types

	typedef Cont Container;
	typedef Cmp Compare;
	typedef CmpStrict CompareStrict;

public:   // data types

	typedef Key key_type;
	typedef T mapped_type;
	typedef std::pair<key_type, mapped_type> value_type;

	typedef typename Container::iterator iterator;
	typedef typename Container::const_iterator const_iterator;

private:  // data types

	typedef std::pair<typename value_type::second_type,
		typename value_type::second_type> SecondComponentCacheElement;
	typedef std::unordered_map<SecondComponentCacheElement, bool,
		boost::hash<SecondComponentCacheElement>> SecondComponentCache;

private:  // data members

	Container container_;
	Compare comparer_;
	CompareStrict comparerStrict_;

	SecondComponentCache secCompCache_;

public:

	Antichain2C() :
		container_(),
		comparer_(),
		comparerStrict_(),
		secCompCache_()
	{ }

	inline iterator find(const key_type& key)
	{
		return container_.find(key);
	}

	inline iterator find(const value_type& value)
	{
		for (auto keyRange = container_.equal_range(value.first);
			keyRange.first != keyRange.second; ++(keyRange.first))
		{	// for all items with proper key
			typename SecondComponentCache::const_iterator itSecCache;
			if ((itSecCache = secCompCache_.find(std::make_pair(
				(keyRange.first)->second, value.second))) != secCompCache_.end())
			{
				if (itSecCache->second)
				{
					return keyRange.first;
				}
				else
				{
					return end();
				}
			}

			if (comparer_(*(keyRange.first), value))
			{	// if there is a bigger set in the cache
				secCompCache_.insert(std::make_pair(std::make_pair(
					(keyRange.first)->second, value.second), true));
				return keyRange.first;
			}
			else
			{
				secCompCache_.insert(std::make_pair(std::make_pair(
					(keyRange.first)->second, value.second), false));
			}
		}

		return end();
	}

	inline const_iterator find(const value_type& value) const
	{
		return const_cast<Antichain2C*>(this)->find(value);
	}

	inline std::pair<iterator, bool> insert(const value_type& value)
	{
		auto keyRange = container_.equal_range(value.first);
		for (auto itRange = keyRange.first; itRange != keyRange.second; ++itRange)
		{	// for all items with proper key
			if (comparer_(*itRange, value))
			{	// if there is a bigger set in the cache
				return std::make_pair(itRange, false);
			}
		}

		while (keyRange.first != keyRange.second)
		{	// until we process all elements for key
			if (comparerStrict_(value, *(keyRange.first)))
			{	// if there is a _strictly_ smaller element in the workset
				auto nextPtr = keyRange.first;
				++nextPtr;
				container_.erase(keyRange.first);
				keyRange.first = nextPtr;
			}
			else
			{
				++(keyRange.first);
			}
		}

		return std::make_pair(container_.insert(value), true);
	}

	inline iterator begin()
	{
		return container_.begin();
	}

	inline const_iterator begin() const
	{
		return container_.begin();
	}

	inline iterator end()
	{
		return container_.end();
	}

	inline const_iterator end() const
	{
		return container_.end();
	}

	inline bool empty() const
	{
		return container_.empty();
	}

	inline void erase(iterator position)
	{
		container_.erase(position);
	}

	inline std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		return container_.equal_range(key);
	}

	friend std::ostream& operator<<(std::ostream& os, const Antichain2C& ac)
	{
		os << Convert::ToString(ac.container_);
		return os;
	}
};

#endif

