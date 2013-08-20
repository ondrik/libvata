/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Cache template header file.
 *
 *****************************************************************************/

#ifndef _VATA_CACHE_HH_
#define _VATA_CACHE_HH_


// standard library headers
#include <unordered_map>
#include <memory>


// Boost headers
#include <boost/functional/hash.hpp>


// insert class to proper namespace
namespace VATA { namespace Util {
	template <class T, class Deleter> class Cache;
}}


/**
 * @brief  A cache for objects
 *
 * This class serves as a cache for objects of the type @p T. It maps an object
 * of type @p T on a unique shared pointer. The cache also performs memory
 * management---in the case there is no shared pointer pointing on the object,
 * it is released. A custom @p Deleter function may be provided.
 */
template <
	class T,
	class Deleter = std::function<void(const T*)>>
class VATA::Util::Cache
{
public:   // data types

	using TPtr             = typename std::shared_ptr<T>;
	using WeakTPtr         = typename std::weak_ptr<T>;
	using TToWeakTPtrMap   = typename std::unordered_map<T, WeakTPtr, boost::hash<T>>;

protected:// data members

	TToWeakTPtrMap store_;
	Deleter deleter_;

	struct DeleteElementF
	{
		Cache& cache_;

		DeleteElementF(
			Cache&                  cache) :
			cache_(cache)
		{ }

		void operator()(const T* v)
		{
			cache_.deleter_(v);
			cache_.store_.erase(*v);
		}
	};

public:   // methods

	Cache() :
		store_(),
		deleter_([](const T*) {})
	{ }

	explicit Cache(
		const Deleter&            deleter) :
		store_(),
		deleter_(deleter)
	{ }

	~Cache()
	{
		assert(this->empty());
	}

	TPtr find(
		const T&                  x)
	{
		auto i = store_.find(x);

		return (i == store_.end())?(TPtr(nullptr)):(TPtr(i->second));
	}

	TPtr lookup(
		const T&                  x)
	{
		auto p = store_.insert(std::make_pair(x, WeakTPtr()));

		if (!p.second)
		{
			return TPtr(p.first->second);
		}

		auto ptr = TPtr(const_cast<T*>(&p.first->first), DeleteElementF(*this));

		p.first->second = WeakTPtr(ptr);

		return ptr;
	}


	bool empty() const
	{
		return store_.empty();
	}
};


#endif
