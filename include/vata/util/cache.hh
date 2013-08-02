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

#include <unordered_map>
#include <memory>

#include <boost/functional/hash.hpp>

// insert class to proper namespace
namespace VATA {
	namespace Util {
		template <class T, class Deleter> class Cache;
	}
}

template <class T, class Deleter = std::function<void(const T*)>>
class VATA::Util::Cache {

public:

	typedef typename std::shared_ptr<T> TPtr;
	typedef typename std::weak_ptr<T> WeakTPtr;
	typedef typename std::unordered_map<T, WeakTPtr, boost::hash<T>> TToWeakTPtrMap;

protected:

	TToWeakTPtrMap store_;
	Deleter deleter_;

	struct DeleteElementF {

		Cache& cache_;

		DeleteElementF(Cache& cache) : cache_(cache) {}

		void operator()(const T* v) {

			this->cache_.deleter_(v);
			this->cache_.store_.erase(*v);

		}

	};

public:

	Cache() : store_(), deleter_([](const T*) {}) {}

	Cache(const Deleter& deleter) : store_(), deleter_(deleter) {}

	~Cache() { assert(this->store_.empty()); }
	
	TPtr find(const T& x) {

		auto i = this->store_.find(x);

		return (i == this->store_.end())?(TPtr(nullptr)):(TPtr(i->second));

	}

	TPtr lookup(const T& x) {

		auto p = this->store_.insert(std::make_pair(x, WeakTPtr()));

		if (!p.second)
			return TPtr(p.first->second);

		auto ptr = TPtr(const_cast<T*>(&p.first->first), DeleteElementF(*this));

		p.first->second = WeakTPtr(ptr);

		return ptr;

	}

	bool empty() const { return this->store_.empty(); }

};

#endif
