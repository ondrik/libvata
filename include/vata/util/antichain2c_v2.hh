/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011 Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a class representing antichain.
 *
 *****************************************************************************/

#ifndef _VATA_ANTICHAIN_2C_V2_HH_
#define _VATA_ANTICHAIN_2C_V2_HH_

#include <ostream>
#include <list>
#include <unordered_map>

#include <vata/util/convert.hh>

namespace VATA
{
	namespace Util
	{
		template <typename Key, typename T> class Antichain2Cv2;
	}
}

template <typename Key, typename T>
class VATA::Util::Antichain2Cv2 {

public:

	typedef std::list<T> TList;
	typedef std::unordered_map<Key, TList> KeyToTListMap;

private:

	KeyToTListMap data_;

public:

	Antichain2Cv2() : data_() {}

	const TList* lookup(const Key& key) const {

		auto i = this->data_.find(key);

		return (i == this->data_.end())?(nullptr):(&i->second);

	}

	// return TRUE if there exists (p, P) in ac such that q <= p and P <= Q
	template <class Cont, class Cmp>
	bool contains(const Cont& candidates /* candidates for p */, const T& Q, const Cmp& cmp) const {

		for (auto& p : candidates) {

			auto i = this->data_.find(p);

			if (i == this->data_.end())
				continue;

			for (auto& P : i->second) {

				if (cmp(P, Q))
					return true;

			}

		}

		return false;

	}

	// remove all (p, P) from ac such that q <= p and P <= Q
	template <class Cont, class Cmp>
	void refine(const Cont& candidates /* candidates for p */, const T& Q, const Cmp& cmp) {

		for (auto& p : candidates) {

			auto i = this->data_.find(p);

			if (i == this->data_.end())
				continue;

			for (auto j = i->second.begin(); j != i->second.end(); ) {

				auto k = j++;

				if (cmp(*k, Q))
					i->second.erase(k);

			}

			if (i->second.empty())
				this->data_.erase(i);

		}

	}

	// add (q, Q) to ac
	void insert(const Key& q, const T& Q) {

		this->data_.insert(std::make_pair(q, TList())).first->second.push_back(Q);

	}

	bool next(Key& q, T& Q) {

		if (this->data_.empty())
			return false;

		auto i = this->data_.begin();

		q = i->first;
		Q = i->second.front();

		i->second.pop_front();

		if (i->second.empty())
			this->data_.erase(i);

		return true;
		
	}

	bool empty() const { return this->data_.empty(); }

	size_t size() const { return this->data_.size(); }
	
	void clear() { this->data_.clear(); }

	friend std::ostream& operator<<(std::ostream& os, const Antichain2Cv2& ac) {

		os << '{';

		for (auto& smallerBiggerListPair : ac.data_) {

			os << " (" << smallerBiggerListPair.first << ", {"; 

			for (auto& element : smallerBiggerListPair.second)
				os << ' ' << Util::Convert::ToString(element);

			os << " })";			

		}

		return os << " }";

	}

};

#endif
