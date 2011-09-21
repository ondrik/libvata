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

template <class T>
class Cache {

public:

	typedef typename std::unordered_map<T, std::weak_ptr<T>> store_type;
	typedef typename store_type::value_type value_type;

protected:

	store_type store_;

public:

	Cache() {}
	
	std::shared_ptr<T> find(const T& x) {

		auto i = this->store_.find(x);

		return (i == this->store_.end())
			?(std::shared_ptr<T>(nullptr))
			:(std::shared_ptr<T>(i->second));

	}

	template <class F>
	std::shared_ptr<T> lookup(const T& x, F f = [](const T*) {}) {

		auto p = this->store_.insert(std::make_pair(x, std::weak_ptr<T>()));

		if (!p.second)
			return std::shared_ptr<T>(p.first->second);

		auto ptr = std::shared_ptr<T>(
			const_cast<T*>(&p.first->first),
			[this, f](const T* tuple) { f(tuple); this->store_.erase(*tuple); }
		);

		p.first->second = std::weak_ptr<StateTuple>(ptr);

		return ptr;

	}

	bool empty() const { return this->store_.empty(); }

};

template <class T, class V>
class CachedBinaryOp {

public:

	typedef std::unordered_map<std::pair<T, T>, V> store_type;
	typedef std::set<typename store_type::value_type*> value_type_set;
	typedef std::unordered_map<T, value_type_list> store_map_type;

protected:

	store_type store_;
	store_map_type storeMap_;

public:

	void clear() {

		this->storeMap_.clear();
		this->store_.clear();

	}

	void invalidateKey(const T& x) {

		auto i = this->storeMap_.find(x);

		if (i == this->storeMap_.end())
			return;

		for (auto item : i->second) {

			auto j =  this->storeMap_.find(
				(x == item->first.first)?(item->first.second):(item->first.first)
			);

			assert(j != this->storeMap_.end());

			j->second.erase(item);

			this->store_.erase(item->first);

		}

		this->storeMap_.erase(i);

	}

	template <class F>
	V lookup(const T& x, const T& y, F f) {

		assert(x != y);

		auto i = this->store.insert(std::make_pair(std::make_pair(x, y), V()));

		if (i.second) {

			f(x, y, &*i.first);

			this->storeMap_.insert(
				std::make_pair(x, value_type_list())
			).first->second.insert(&*i.first);
			
			this->storeMap_.insert(
				std::make_pair(y, value_type_list())
			).first->second.insert(&*i.first);

		}

		return i.first->second;

	}

};

#endif
