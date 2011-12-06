/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for smart set.
 *
 *****************************************************************************/

#ifndef _VATA_SMART_SET_HH_
#define _VATA_SMART_SET_HH_

#include <ostream>
#include <vector>

namespace VATA {
		namespace Util {
				class SmartSet;
		}
}

class VATA::Util::SmartSet {

public:

	typedef size_t Key;

private:

	struct Element {

		Element* next_;
		Key key_;
		size_t count_;

		Element(const Key& key, size_t count = 0) : next_(nullptr), key_(key), count_(count) {}

	};

	GCC_DIAG_OFF(effc++)
	struct Iterator : public std::iterator<std::input_iterator_tag, Key> {
	GCC_DIAG_ON(effc++)

		const Element* element_;

		Iterator(const Element* element) : element_(element) {}

		Iterator& operator++() {

			assert(this->element_);

			this->element_ = this->element_->next_;

			return *this;

		}

		Iterator operator++(int) const {

			return ++Iterator(this->element_);

		}

		const Key& operator*() {

			assert(this->element_);

			return this->element_->key_;

		}

		bool operator==(const Iterator& rhs) const {

			return this->element_ == rhs.element_;

		}

		bool operator!=(const Iterator& rhs) const {

			return this->element_ != rhs.element_;

		}

	};

public:

	typedef Iterator iterator;

private:

	Element head_;
	Element* last_;
	size_t size_;

	std::vector<Element*> index_;

protected:

	size_t& insert(const Key& key) {

		assert(key < this->index_.size());

		auto& prev = this->index_[key];

		if (!prev) {

			prev = this->last_;
			prev->next_ = new Element(key);
			this->last_ = prev->next_;

			++this->size_;

		}

		assert(key == prev->next_->key_);

		return prev->next_->count_;

	}

	void erase(Element*& prev) {

		assert(prev);

		--this->size_;

		auto el = prev->next_;

		assert(el);

		prev->next_ = el->next_;

		if (prev->next_) {

			assert(prev->next_->key_ < this->index_.size());
			assert(this->index_[prev->next_->key_] == el);

			this->index_[prev->next_->key_] = prev;

		}

		delete el;

		prev = nullptr;

	}

public:

	SmartSet(size_t range = 0) : head_(Key(), 0), last_(&head_), size_(0), index_(range, nullptr) {}

	SmartSet(const SmartSet& s) : head_(Key(), 0), last_(&head_), size_(s.size_),
		index_(s.index_.size(), nullptr) {

		for (auto el = s.head_.next_ ; el; el = el->next_) {

			this->index_[el->key_] = this->last_;
			this->last_->next_ = new Element(el->key_, el->count_);
			this->last_ = this->last_->next_;

		}

	}

	SmartSet& operator=(const SmartSet& s) {

		std::fill(this->index_.begin(), this->index_.end(), nullptr);

		this->index_.resize(s.index_.size(), nullptr);

		this->last_ = &this->head_;

		for (auto el = s.head_.next_ ; el; el = el->next_) {

			this->index_[el->key_] = this->last_;
			this->last_->next_ = new Element(el->key_, el->count_);
			this->last_ = this->last_->next_;

		}

		this->size_ = s.size();

		return *this;

	}

	void assignFlat(const SmartSet& s) {

		std::fill(this->index_.begin(), this->index_.end(), nullptr);

		this->index_.resize(s.index_.size(), nullptr);

		this->last_ = &this->head_;

		for (auto el = s.head_.next_ ; el; el = el->next_) {

			this->index_[el->key_] = this->last_;
			this->last_->next_ = new Element(el->key_, 1);
			this->last_ = this->last_->next_;

		}

		this->size_ = s.size();

	}

	SmartSet::iterator begin() const { return SmartSet::Iterator(this->head_.next_); }
	SmartSet::iterator end() const { return SmartSet::Iterator(nullptr); }

	bool contains(const Key& key) const {

		assert(key < this->index_.size());

		if (!this->index_[key])
			return false;

		assert(this->index_[key]->next_);
		assert(this->index_[key]->next_->key_ == key);

		return true;

	}

	size_t count(const Key& key) const {

		assert(key < this->index_.size());

		if (!this->index_[key])
			return 0;

		assert(this->index_[key]->next_);
		assert(this->index_[key]->next_->key_ == key);

		return this->index_[key]->next_->count_;

	}

	void init(const Key& key, size_t count) {

		if (count) {

			this->insert(key) = count;

			return;

		}

		assert(key < this->index_.size());

		auto& prev = this->index_[key];

		if (prev)
			this->erase(prev);

	}

	void add(const Key& key) {

		++this->insert(key);

	}

	void remove(const Key& key) {

		assert(key < this->index_.size());

		auto& prev = this->index_[key];

		if (!prev)
			return;

		auto el = prev->next_;

		assert(el);
		assert(key == el->key_);

		if (el->count_ == 1)

			this->erase(prev);

		else

			--el->count_;

	}

	void removeStrict(const Key& key) {

		assert(key < this->index_.size());

		auto& prev = this->index_[key];

		assert(prev);

		auto el = prev->next_;

		assert(key == el->key_);

		if (el->count_ == 1)

			this->erase(prev);

		else

			--el->count_;

	}

	bool empty() const { return this->head_.next_ == nullptr; }

	size_t size() const { return this->size_; }

	void clear() {

		for (auto el = this->head_.next_; el; ) {

			auto tmp = el;

			el = el->next_;

			assert(tmp->key_ < this->index_.size());

			this->index_[tmp->key_] = nullptr;

			delete tmp;

		}

		this->last_ = &this->head_;
		this->size_ = 0;

	}

	friend std::ostream& operator<<(std::ostream& os, const SmartSet& s) {

		os << '{';

		for (auto el = s.head_.next_; el; el = el->next_)
			os << ' ' << el->key_ << ':' << el->count_;

		return os << " }";

	}

};

#endif
