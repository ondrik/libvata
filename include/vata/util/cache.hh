/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Caching stuff.
 *
 *****************************************************************************/

#ifndef _VATA_CACHE_HH
#define _VATA_CACHE_HH

#include <set>
#include <unordered_map>
#include <memory>

// insert class to proper namespace
namespace VATA {
	namespace Util {
		template <class T, class F> class Cache;
		template <class T1, class T2, class V> class CachedBinaryOp;
	}
}

template <class T, class F = std::function<void(const T*)>>
class VATA::Util::Cache {

public:

	typedef typename std::shared_ptr<T> TPtr;
	typedef typename std::weak_ptr<T> WeakTPtr;
	typedef typename std::unordered_map<T, WeakTPtr> TToWeakTPtrMap;

protected:

	TToWeakTPtrMap store_;
	F f_;

public:

	Cache() : store_(), f_([](const T*) {}) {}

	Cache(const F& f) : store_(), f_(f) {}
	
	TPtr find(const T& x) {

		auto i = this->store_.find(x);

		return (i == this->store_.end())?(TPtr(nullptr)):(TPtr(i->second));

	}

	TPtr lookup(const T& x) {

		auto p = this->store_.insert(std::make_pair(x, WeakTPtr()));

		if (!p.second)
			return TPtr(p.first->second);

		auto ptr = TPtr(
			const_cast<T*>(&p.first->first),
			[this](const T* v) { this->f_(v); this->store_.erase(*v); }
		);

		p.first->second = WeakTPtr(ptr);

		return ptr;

	}

	bool empty() const { return this->store_.empty(); }

};

template <class T1, class T2, class V>
class VATA::Util::CachedBinaryOp {

public:

	typedef std::pair<T1, T2> Key;
	typedef std::unordered_map<Key, V> KeyToVMap;
	typedef std::set<typename KeyToVMap::value_type*> KeyToVMapValueTypeSet;
	typedef std::unordered_map<T1, KeyToVMapValueTypeSet> T1ToKeyToVMapValueTypeSetMap;
	typedef std::unordered_map<T2, KeyToVMapValueTypeSet> T2ToKeyToVMapValueTypeSetMap;

protected:

	KeyToVMap store_;
	T1ToKeyToVMapValueTypeSetMap storeMap1_;
	T2ToKeyToVMapValueTypeSetMap storeMap2_;

public:

	CachedBinaryOp() : store_(), storeMap1_(), storeMap2_() {}

	void clear() {

		this->storeMap1_.clear();
		this->storeMap2_.clear();
		this->store_.clear();

	}

	void invalidateFirst(const T1& x) {

		auto i = this->storeMap1_.find(x);

		if (i == this->storeMap1_.end())
			return;

		for (auto& item : i->second) {

			auto j = this->storeMap2_.find(item->first.second);

			assert(j != this->storeMap2_.end());

			j->second.erase(item);

			this->store_.erase(item->first);

		}

		this->storeMap1_.erase(i);

	}

	void invalidateSecond(const T2& x) {

		auto i = this->storeMap2_.find(x);

		if (i == this->storeMap2_.end())
			return;

		for (auto& item : i->second) {

			auto j = this->storeMap1_.find(item->first.first);

			assert(j != this->storeMap1_.end());

			j->second.erase(item);

			this->store_.erase(item->first);

		}

		this->storeMap2_.erase(i);

	}

	template <class F>
	V lookup(const T1& x, const T2& y, F f) {

		auto p = this->store_.insert(std::make_pair(std::make_pair(x, y), V()));

		if (p.second) {

			p.first->second = f(x, y);

			this->storeMap1_.insert(
				std::make_pair(x, KeyToVMapValueTypeSet())
			).first->second.insert(&*p.first);
			
			this->storeMap2_.insert(
				std::make_pair(y, KeyToVMapValueTypeSet())
			).first->second.insert(&*p.first);

		}

		return p.first->second;

	}

};

#endif
