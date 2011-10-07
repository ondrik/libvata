/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Shared list template header file.
 *
 *****************************************************************************/

#ifndef _VATA_SHARED_LIST_HH_
#define _VATA_SHARED_LIST_HH_

// insert class to proper namespace
namespace VATA {
	namespace Util {
		template <class T> class SharedList;
	}
}

template <class T>
class VATA::Util::SharedList {

public:

	struct Iterator {

		typedef std::input_iterator_tag iterator_category;
		typedef size_t value_type;
		typedef size_t* pointer;
		typedef size_t& reference;

		const SharedList* pos_;
		typename T::const_iterator iter_;

		Iterator() : pos_(nullptr), iter_() {}

		Iterator(const SharedList* pos) : pos_(pos), iter_(pos->subList_->begin()) {}

		Iterator& operator++() {

			assert(this->pos_);
			assert(this->pos_->subList_);

			if (++this->iter_ != this->pos_->subList_->end())
				return *this;

			if ((this->pos_ = this->pos_->next_) == nullptr)
				return *this;

			assert(this->pos_->subList_);

			this->iter_ = this->pos_->subList_->begin();

			return *this;

		}

		Iterator operator++(int) const {

			return ++Iterator(this->pos_);

		}

		bool operator==(const Iterator& rhs) const {

			if (this->pos_ != rhs.pos_)
				return false;

			if (!this->pos_)
				return true;
				
			return this->iter_ == rhs.iter_;

		}

		bool operator!=(const Iterator& rhs) const {

			if (this->pos_ != rhs.pos_)
				return true;

			if (!this->pos_)
				return false;

			return this->iter_ != rhs.iter_;

		}

		const size_t& operator*() const {

			return *this->iter_;
			
		}

	};

	Iterator begin() const { return Iterator(this); }
	Iterator end() const { return Iterator(); }

	typedef Iterator const_iterator;

private:

	SharedList* next_;
	T* subList_;
	size_t counter_;

public:

	SharedList(T* subList = nullptr) : next_(nullptr), subList_(subList), counter_(1) {}

	void init(T* subList) {

		assert(subList);

		this->subList_ = subList;

	}

	T* subList() {

		return this->subList_;

	}

	template <class Deleter>
	void release(const Deleter& deleter) {

		SharedList* elem = this, * tmp;

		while (elem && elem->counter_ == 1) {
			tmp = elem;
			elem = elem->next_;
			deleter(tmp);
		}

		if (elem)
			--elem->counter_;

	}

	template <class Deleter>
	void unsafeRelease(const Deleter& deleter) {

		SharedList* elem = this;

		while (elem && elem->counter_ == 1) {
			deleter(elem);
			elem = elem->next_;
		}

		if (elem)
			--elem->counter_;

	}

	SharedList* copy() {

		++this->counter_;

		return this;

	}

	template <class Allocator>
	static bool append(SharedList*& list, const typename T::value_type& v,
		Allocator& allocator) {

		if (!list) {

			list = allocator();
			list->next_ = nullptr;
			list->subList_->push_back(v);

			return true;

		}

		if (list->counter_ > 1) {

			SharedList* tmp = allocator();
			tmp->next_ = list;
			list = tmp;

		}

		list->subList_->push_back(v);

		return false;

	}
/*
	friend std::ostream& operator<<(std::ostream& os, const SharedList& list) {

		os << '(';

		for (auto& v : list)
			os << ' ' << v;

		return os << " )";

	}
*/
};

#endif
