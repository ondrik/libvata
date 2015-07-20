/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011 Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a class representing antichain.
 *
 *****************************************************************************/

#ifndef _VATA_ANTICHAIN_1C_HH_
#define _VATA_ANTICHAIN_1C_HH_

#include <ostream>
#include <list>
#include <unordered_set>

#include <vata/util/convert.hh>

namespace VATA
{
	namespace Util
	{
		template <typename Key> class Antichain1C;
		template <typename Key> class SequentialAntichain1C;
	}
}

template <class Key>
class VATA::Util::Antichain1C {

public:

	typedef std::unordered_set<Key> KeySet;

private:

	KeySet data_;

public:

	Antichain1C() : data_() {}

	// return TRUE if there exists p in ac such that q <= p
	template <class Cont>
	bool contains(const Cont& candidates /* candidates for p */) const {

		for (auto& p : candidates) {
			
			if (this->data_.find(p) != this->data_.end())
				return true;

		}

		return false;

	}

	// remove all p from ac such that q <= p
	template <class Cont>
	void refine(const Cont& candidates /* candidates for p */) {

		for (auto& p : candidates)
			this->data_.erase(p);

	}

	// add q to ac
	void insert(const Key& q) {

		this->data_.insert(q);

	}

	bool next(Key& s) {

		if (this->data_.empty())
			return false;

		s = *this->data_.begin();

		this->data_.erase(this->data_.begin());

		return true;
		
	}

	const KeySet& data() const { return this->data_; }

	void clear() { this->data_.clear(); }

	friend std::ostream& operator<<(std::ostream& os, const Antichain1C& ac) {

		return os << Util::Convert::ToString(ac.data_);

	}

};

template <class Key>
class VATA::Util::SequentialAntichain1C {

public:

	typedef std::list<Key> KeyList;

private:

	KeyList data_;

public:

	SequentialAntichain1C() : data_() {}

	template <class Cmp>
	bool insert(const Key& key, const Cmp& cmp) {

		for (auto iter = this->data_.begin(); iter != this->data_.end(); ++iter) {

			if (cmp(key, *iter))
				return false;

			if (!cmp(*iter, key))
				continue;

			iter = this->data_.erase(iter);

			while (iter != this->data_.end()) {

				if (cmp(*iter, key))
					iter = this->data_.erase(iter);
				else ++iter;

			}

			break;

		}

		this->data_.push_back(key);

		return true;

	}

	const KeyList& data() const { return this->data_; }

	void clear() { this->data_.clear(); }

	friend std::ostream& operator<<(std::ostream& os, const SequentialAntichain1C& ac) {

		return os << Util::Convert::ToString(ac.data_);

	}

};

#endif
