/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
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
#include <list>

namespace VATA { namespace Util { class SmartSet; } }

class VATA::Util::SmartSet {

public:

	typedef std::pair<size_t, size_t> KeyCountPair;
	typedef std::list<KeyCountPair> KeyCountPairList;
	typedef std::vector<KeyCountPairList::iterator> KeyCountPairListElementIndex;

private:

	class Iterator {
		
		KeyCountPairList::const_iterator i_;

	public:

		typedef std::input_iterator_tag iterator_category;
		typedef size_t value_type;
		typedef size_t difference_type;
		typedef size_t* pointer;
		typedef size_t& reference;

		Iterator(KeyCountPairList::const_iterator i) : i_(i) {}

		Iterator& operator++() {
			return ++this->i_, *this;
		}

		Iterator operator++(int) {
			return Iterator(this->i_++);
		}
		
		size_t operator*() {
			return this->i_->first;
		}
		
		bool operator==(const Iterator& rhs) {
			return this->i_ == rhs.i_;
		}

		bool operator!=(const Iterator& rhs) {
			return this->i_ != rhs.i_;
		}

	};

public:

	typedef Iterator iterator;

private:

	KeyCountPairList elements_;
	KeyCountPairList::iterator sentinel_;
	KeyCountPairListElementIndex index_;

protected:

	size_t& insert(size_t key) {

		auto& iter = this->index_[key];
		
		if (iter == this->sentinel_)
			iter = this->elements_.insert(this->elements_.begin(), std::make_pair(key, 0));

		assert(key == iter->first);

		return iter->second;

	}

public:

	SmartSet(size_t size = 0) : elements_(), sentinel_(), index_(size, this->sentinel_) {}
	
	SmartSet(const SmartSet& s) : elements_(s.elements_), sentinel_(),
		index_(s.index_.size(), this->sentinel_) {

		for (auto i = this->elements_.begin(); i != this->elements_.end(); ++i)
			this->index_[i->first] = i;

	}
	
	SmartSet& operator=(const SmartSet& s) {

		this->elements_ = s.elements_;

		std::fill(this->index_.begin(), this->index_.end(), this->sentinel_);

		this->index_.resize(s.index_.size(), this->sentinel_);

		for (auto i = this->elements_.begin(); i != this->elements_.end(); ++i)
			this->index_[i->first] = i;

		return *this;

	}

	void assignFlat(const SmartSet& s) {

		this->elements_ = s.elements_;

		std::fill(this->index_.begin(), this->index_.end(), this->sentinel_);

		this->index_.resize(s.index_.size(), this->sentinel_);

		for (auto i = this->elements_.begin(); i != this->elements_.end(); ++i) {
			this->index_[i->first] = i;
			i->second = 1;
		}

	}

	SmartSet::iterator begin() const { return SmartSet::Iterator(this->elements_.begin()); }
	SmartSet::iterator end() const { return SmartSet::Iterator(this->elements_.end()); }

	bool contains(size_t key) const {

		assert(key < this->index_.size());

		auto& iter = this->index_[key];

		assert((iter == this->sentinel_) || (key == iter->first));

		return (iter != this->sentinel_);

	}
	
	size_t count(size_t key) const {

		assert(key < this->index_.size());

		auto& iter = this->index_[key];

		assert((iter == this->sentinel_) || (key == iter->first));

		return (iter == this->sentinel_)?(0):(iter->second);

	}

	void init(size_t key, size_t count) {

		assert(key < this->index_.size());

		if (count) {

			this->insert(key) = count;

			return;

		}

		auto& iter = this->index_[key];

		if (iter != this->sentinel_) {

			assert(key == iter->first);

			this->elements_.erase(iter);

			iter = this->sentinel_;

		}

	}

	void add(size_t key) {

		assert(key < this->index_.size());

		++this->insert(key);

	}

	void remove(size_t key) {

		assert(key < this->index_.size());

		auto& iter = this->index_[key];

		if (iter == this->sentinel_)
			return;

		assert(key == iter->first);

		if (iter->second == 1) {

			this->elements_.erase(iter);
			iter = this->sentinel_;

		} else
			--iter->second;

	}

	bool empty() const { return this->elements_.empty(); }

	size_t size() const { return this->elements_.size(); }

	void clear() {

		for (auto i = this->elements_.begin(); i != this->elements_.end(); ++i)
			this->index_[i->first] = this->sentinel_;

		this->elements_.clear();

	}
	
	void buildVector(std::vector<size_t>& v) const {

		v.clear();

		for (auto i = this->elements_.begin(); i != this->elements_.end(); ++i)
			v.push_back(i->first);

	}

	friend std::ostream& operator<<(std::ostream& os, const SmartSet& s) {

		os << '{';

		for (auto i = s.elements_.begin(); i != s.elements_.end(); ++i)
			os << ' ' << i->first << ':' << i->second;

		return os << " }";

	}

};

//std::ostream& operator<<(std::ostream& os, const VATA::Util::SmartSet& x);

#endif
