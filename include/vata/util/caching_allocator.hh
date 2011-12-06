/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Caching allocator template header file.
 *
 *****************************************************************************/

#ifndef _VATA_CACHING_ALLOCATOR_HH_
#define _VATA_CACHING_ALLOCATOR_HH_

#include <cstdlib>

#include <vector>
#include <functional>

#include <unordered_set>

// insert class to proper namespace
namespace VATA {
	namespace Util {
		template <class T, class Initializer = std::function<void(T*)>> class CachingAllocator;
		template <class T, class Initializer = std::function<void(T*)>> class CachingArrayAllocator;
	}
}

template <class T, class Initializer>
class VATA::Util::CachingAllocator {

public:

	typedef T* Ptr;

private:

	std::vector<T*> store_;
	Initializer initializer_;

public:

	CachingAllocator() : store_(), initializer_([](T*){}) {}

	CachingAllocator(Initializer initializer) : store_(), initializer_(initializer) {}

	~CachingAllocator() {

		for (auto& element : this->store_)
			delete element;

	}

	Ptr operator()() {

		Ptr ptr;

		if (!this->store_.empty()) {

			ptr = this->store_.back();
			this->store_.pop_back();

		} else {

			ptr = new T();

		}

		this->initializer_(ptr);

		return ptr;

	}

	void reclaim(Ptr ptr) {

		this->store_.push_back(ptr);

	}

};

template <class T, class Initializer>
class VATA::Util::CachingArrayAllocator {

public:

	typedef T* Ptr;

private:

	std::vector<T*> store_;
	Initializer initializer_;

public:

	CachingArrayAllocator() : store_(), initializer_([](T*){}) {}

	CachingArrayAllocator(Initializer initializer) : store_(), initializer_(initializer) {}

	~CachingArrayAllocator() {

		for (auto& element : this->store_)
			std::free(element);

	}

	Ptr operator()(size_t count) {

		Ptr ptr;

		if (!this->store_.empty()) {

			ptr = this->store_.back();
			this->store_.pop_back();

		} else {

			ptr = reinterpret_cast<T*>(std::malloc(count*sizeof(T)));

			if (!ptr)
				throw std::bad_alloc();

		}

		this->initializer_(ptr);

		return ptr;

	}

	void reclaim(Ptr ptr) {

		this->store_.push_back(ptr);

	}

};

#endif
