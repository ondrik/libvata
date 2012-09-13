/*****************************************************************************
 *  VATA Tree Automata Library
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

	typedef Key first_type;
	typedef T second_type;
	typedef std::list<T> TList;
	typedef std::unordered_map<Key, TList> KeyToTListMap;

protected:

	struct DummyEraser {
		void operator()(const Key&, const typename TList::iterator&) const {}
	};

private:

	KeyToTListMap data_;

protected:

	template <class Cont>
	static bool checkIteratorPresence(Cont& c, const typename Cont::iterator& iter) {

		for (auto i = c.begin(); i != c.end(); ++i) {

			if (i == iter)
				return true;

		}

		return false;

	}

public:

	Antichain2Cv2() : data_() {}

	void swap(Antichain2Cv2& rhs) {

		std::swap(this->data_, rhs.data_);

	}

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
	template <class Cont, class Cmp, class Eraser = DummyEraser>
	void refine(const Cont& candidates /* candidates for p */, const T& Q, const Cmp& cmp,
		const Eraser& eraser = DummyEraser()) {

		for (auto& p : candidates) {

			auto i = this->data_.find(p);

			if (i == this->data_.end())
				continue;

			for (auto j = i->second.begin(); j != i->second.end(); ) {

				auto k = j++;

				if (cmp(*k, Q)) {

					eraser(p, k);

					i->second.erase(k);

				}

			}

			if (i->second.empty())
				this->data_.erase(i);

		}

	}

	// add (q, Q) to ac
	typename TList::iterator insert(const Key& q, const T& Q) {

		auto& list = this->data_.insert(std::make_pair(q, TList())).first->second;

		return list.insert(list.end(), Q);

	}

	bool get(Key& q, T& Q) {

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

	void remove(const Key& q, const typename TList::iterator& Q) {

		auto iter = this->data_.find(q);

		assert(iter != this->data_.end());
		assert(Antichain2Cv2::checkIteratorPresence(iter->second, Q));

		iter->second.erase(Q);

		if (iter->second.empty())
			this->data_.erase(iter);

	}

	const KeyToTListMap& data() const { return this->data_; }

	size_t size() const {

		size_t size = 0;

		for (auto& p : this->data_)
			size += p.second.size();

		return size;

	}

	void clear() { this->data_.clear(); }

	inline bool empty() { return this->data_.empty();}

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
