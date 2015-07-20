/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011 Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a class representing antichain.
 *
 *****************************************************************************/

#ifndef _VATA_ORDERED_ANTICHAIN_2C_HH_
#define _VATA_ORDERED_ANTICHAIN_2C_HH_

#include <ostream>
#include <set>

#include <vata/util/convert.hh>

namespace VATA
{
	namespace Util
	{
		template <typename Antichain2C, class Less> class OrderedAntichain2C;
	}
}

template <class Antichain2C, class Less>
class VATA::Util::OrderedAntichain2C {

public:

	typedef typename Antichain2C::first_type first_type;
	typedef typename Antichain2C::second_type second_type;
	typedef std::pair<first_type, typename Antichain2C::TList::iterator> KeyIteratorPair;

	struct less {

		bool operator()(const KeyIteratorPair& p1, const KeyIteratorPair& p2) const {

			return Less()(
				std::make_pair(p1.first, *p1.second), std::make_pair(p2.first, *p2.second)
			);

		}

	};
	
	typedef std::set<KeyIteratorPair, less> OrderedType;

private:

	Antichain2C antichain_;
	OrderedType data_;

public:

	OrderedAntichain2C() : antichain_(), data_() {}

	const typename Antichain2C::TList* lookup(const first_type& key) const {

		return this->antichain_.lookup(key);

	}

	// return TRUE if there exists (p, P) in ac such that q <= p and P <= Q
	template <class Cont, class Cmp>
	bool contains(const Cont& candidates /* candidates for p */, const second_type& Q,
		const Cmp& cmp) const {

		return this->antichain_.contains(candidates, Q, cmp);

	}

	// remove all (p, P) from ac such that q <= p and P <= Q
	template <class Cont, class Cmp>
	void refine(const Cont& candidates /* candidates for p */, const second_type& Q,
		const Cmp& cmp) {

		struct Eraser {

			OrderedType& data_;

			Eraser(OrderedType& data) : data_(data) {}

			void operator()(const first_type& q,
				const typename Antichain2C::TList::iterator& Q) const {

				assert(this->data_.count(std::make_pair(q, Q)));

				this->data_.erase(std::make_pair(q, Q));
				
			}

		};

		this->antichain_.refine(candidates, Q, cmp, Eraser(this->data_));

		assert(this->antichain_.size() == this->data_.size());

	}

	// add (q, Q) to ac
	typename OrderedType::iterator insert(const first_type& q, const second_type& Q) {

		auto iter = this->antichain_.insert(q, Q);

		assert(this->data_.count(std::make_pair(q, iter)) == 0);

		return this->data_.insert(std::make_pair(q, iter)).first;

	}

	bool get(first_type& q, second_type& Q) {

		assert(this->antichain_.size() == this->data_.size());

		if (this->data_.empty())
			return false;

		auto i = this->data_.begin();

		q = i->first;
		Q = *i->second;

		this->antichain_.remove(i->first, i->second);

		this->data_.erase(i);

		assert(this->antichain_.size() == this->data_.size());

		return true;

	}

	const Antichain2C& antichain() const { return this->antichain_; }

	const OrderedType& data() const { return this->data_; }

	void clear() {

		this->antichain_.clear();
		this->data_.clear();

	}

	inline bool empty() {

		assert(this->antichain_.size() == this->data_.size());

		return this->data_.empty();

	}

	friend std::ostream& operator<<(std::ostream& os, const OrderedAntichain2C& ac) {

		os << '{';

		for (auto& smallerBiggerPair : ac.data_) {

			os << " (" << Util::Convert::ToString(smallerBiggerPair.first) << ", " <<
				Util::Convert::ToString(*smallerBiggerPair.second) << ')';

		}

		return os << " }";

	}

};

#endif
