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

/**
 * @brief  An implementation of a 2-component antichain
 *
 * This class implements a 2-component @e antichain, i.e., an antichain over the
 * domain Key x T. An antichain is a set of elements from the domain which are
 * pairwise incomparable w.r.t. a given partial order.
 *
 * The antichain represents an @e up @e closure of its elements, i.e., the set
 * of all elements of the domain that are greater than or equal to some element in
 * the antichain. The notion of @e greater in our case deserves more discussion.
 * We consider a pair (p, P) to be greater than (q, Q), denoted as
 * (q, Q) <= (p, P), if and only if: q => p && Q <= P.
 *
 * This implementation is targeted for the use in checking language inclusion of
 * a pair of automata A <= B, where Key is a state of A and T is a set of states
 * of B. The partial order is a composition of the partial orders on the two
 * given domains, i.e., for the intended use, the order on A is the identity (or
 * a simulation) and the order is the set inclusion relation (or set inclusion
 * enhanced by simulation).
 *
 * @tparam  Key  The first domain
 * @tparam  T    The second domain
 */
template <typename Key, typename T>
class VATA::Util::Antichain2Cv2
{
public:

	typedef Key first_type;
	typedef T second_type;
	typedef std::list<T> TList;
	typedef std::unordered_map<Key, TList> KeyToTListMap;

protected:

	/**
	 * @brief  An eraser that does nothing
	 */
	struct DummyEraser
	{
		void operator()(const Key&, const typename TList::iterator&) const {}
	};

private:

	KeyToTListMap data_;

protected:

	/**
	 * @brief  Checks whether an iterator is in a given container
	 *
	 * This method checks whether an instance of an iterator is inside a given
	 * container.
	 *
	 * @param[in]  c      The container to be checked for the presence of @p iter
	 * @param[in]  iter   The iterator the presence of which is to be checked
	 *
	 * @returns  @p true in the case @p iter points inside @p c, @p false
	 *           otherwise
	 */
	template <class Cont>
	static bool checkIteratorPresence(const Cont& c, const typename Cont::iterator& iter)
	{
		for (auto it = c.begin(); it != c.end(); ++it)
		{
			if (it == iter)
				return true;
		}

		return false;
	}

public:

	/**
	 * @brief  Default constructor
	 *
	 * Default constructor.
	 */
	Antichain2Cv2() : data_() {}


	/**
	 * @brief  Swaps the content with another antichain
	 *
	 * Swaps the content of the antichain with another antichain.
	 *
	 * @param[in, out]  rhs  The other antichain
	 */
	void swap(
		Antichain2Cv2&                rhs)
	{
		std::swap(data_, rhs.data_);
	}


	/**
	 * @brief  Looks up a list of elements associated with a key
	 *
	 * Looks up a list of elements of T that are stored in the antichain in pairs
	 * where the first component of the pair is @p key.
	 *
	 * @param[in]  key  The first component of the pairs in the antichain
	 *
	 * @return  A pointer to a list of elements of T that are as the second
	 *          component in a pair with @p key, @p nullptr if there are none
	 */
	const TList* lookup(const Key& key) const
	{
		auto it = data_.find(key);

		return (data_.end() == it) ? (nullptr) : (&it->second);
	}


	/**
	 * @brief  Checks whether a pair (q, Q) is covered by the antichain
	 *
	 * This method determines whether a pair (q, Q) is covered by the antichain.
	 * In particular, it is given a collection of @p candidates for 'p' (such
	 * that p => q) and tries to find in the antichain a pair '(p, P)' such
	 * that P <= @p Q. The '<=' relation on elements of T is given by @p cmp.
	 * If there is such, the method returns @p true, otherwise it returns @p
	 * false.
	 *
	 * @param[in]  candidates  Candidates for 'p'
	 * @param[in]  Q           The 'Q' in the pair '(q, Q)'
	 * @param[in]  cmp         The '<=' comparison operation on elements of T
	 *
	 * @returns  @p true if '(q, Q)' is covered by the antichain, @p false
	 *           otherwise
	 *
	 * @note  The relation @p cmp needs to be complementary to the one used in
	 *        the refine() method!
	 */
	template <class Cont, class Cmp>
	bool contains(
		const Cont&                  candidates,
		const T&                     Q,
		const Cmp&                   cmp) const
	{
		for (const Key& p : candidates)
		{	// check all candidates for 'p'
			auto iter = data_.find(p);
			if (data_.end() == iter)
			{	// in the case there is no pair (p, _) in the antichain
				continue;
			}

			for (auto& P : iter->second)
			{	// for all 'P' such that (p, P) is in the antichain, check whether P <= Q
				if (cmp(P, Q))
				{	// if P <= Q
					return true;
				}
			}
		}

		return false;
	}


	/**
	 * @brief  Removes all pairs subsumed by (q, Q) from the antichain
	 *
	 * This method removes all pairs (p, P) covered by (q, Q) from the antichain.
	 * In particular, it is given a collection of @p candidates for 'p' (such that
	 * p <= q) and removes from the antichain all pairs '(p, P)' such that P => @p
	 * Q. The '=>' relation on elements of T is given by @p cmp. An optional
	 * @p eraser on the removed elements is executed.
	 *
	 * @param[in]  candidates  Candidates for 'p'
	 * @param[in]  Q           The 'Q' in the pair '(q, Q)'
	 * @param[in]  cmp         The '=>' comparison operation on elements of T
	 * @param[in]  eraser      An optional eraser for removed elements
	 *
	 * @note  The relation @p cmp needs to be complementary to the one used in
	 *        the contains() method!
	 */
	template <class Cont, class Cmp, class Eraser = DummyEraser>
	void refine(
		const Cont&          candidates,
		const T&             Q,
		const Cmp&           cmp,
		const Eraser&        eraser = DummyEraser())
	{
		for (const Key& p : candidates)
		{	// check all candidates for 'p'
			auto iter = data_.find(p);
			if (data_.end() == iter)
			{	// in the case there is no pair (p, _) in the antichain
				continue;
			}

			for (auto jter = iter->second.begin(); jter != iter->second.end(); )
			{	// for all 'P' such that (p, P) is in the antichain, check whether P <= Q
				auto kter = jter++;

				if (cmp(*kter, Q))
				{	// if P <= Q, erase (p, P) from the antichain
					eraser(p, kter);

					// we rely on the fact that erase() does not invalidate jter
					iter->second.erase(kter);
				}
			}

			if (iter->second.empty())
			{	// in case there is no (p, _) left, remove 'p'
				data_.erase(iter);
			}
		}
	}


	/**
	 * @brief  Adds a pair (q, Q) in the antichain
	 *
	 * This method inserts a new pair '(@p q, @p Q)' into the antichain and
	 * returns an iterator at the inserted element.
	 *
	 * @param[in]  q  The 'q' in '(q, Q)'
	 * @param[in]  Q  The 'Q' in '(q, Q)'
	 *
	 * @returns  An iterator at the inserted element
	 */
	typename TList::iterator insert(
		const Key&          q,
		const T&            Q)
	{
		TList& list = data_.insert(std::make_pair(q, TList())).first->second;

		return list.insert(list.end(), Q);
	}


	/**
	 * @brief  Retrieves an arbitrary element from the antichain
	 *
	 * Retrieves an arbitrary pair '(@p q, @p Q)' from the antichain. Returns @p
	 * true if there is some, @p false otherwise.
	 *
	 * @param[out]  q  The 'q' in '(q, Q)'
	 * @param[out]  Q  The 'Q' in '(q, Q)'
	 *
	 * @returns  @p true if there is some element in the antichain, @p false
	 *           otherwise
	 */
	bool get(
		Key&         q,
		T&           Q)
	{
		if (data_.empty())
		{	// stop immediately in the case the antichain is empty
			assert(data_.cbegin() == data_.cend());
			return false;
		}

		auto iter = data_.begin();
		// make sure there is something
		assert(data_.end() != iter);

		q = iter->first;

		// make sure there is something
		assert(!iter->second.empty());
		Q = iter->second.front();

		iter->second.pop_front();

		if (iter->second.empty())
		{
			data_.erase(iter);
		}

		return true;
	}


	/**
	 * @brief  Removes a pair (q, iterator) from the antichain
	 *
	 * This method removes the pair '(q, Q)', where @p Q is given by an iterator
	 * pointing at its position, from the antichain
	 *
	 * @param[in]  q  The 'q' in '(q, Q)'
	 * @param[in]  Q  The iterator pointing at 'Q' in '(q, Q)'
	 */
	void remove(const Key& q, const typename TList::iterator& Q)
	{
		auto iter = data_.find(q);

		assert(data_.end() != iter);
		assert(Antichain2Cv2::checkIteratorPresence(iter->second, Q));

		iter->second.erase(Q);

		if (iter->second.empty())
		{
			data_.erase(iter);
		}
	}


	/**
	 * @brief  Retrieves the data of the antichain
	 *
	 * Returns the data of the antichain, i.e., the contained structure.
	 *
	 * @returns  The data of the antichain.
	 */
	const KeyToTListMap& data() const
	{
		return data_;
	}


	/**
	 * @brief  The size of the antichain
	 *
	 * Returns the total number of elements in the antichain.
	 *
	 * @returns  The total number of elements in the antichain
	 */
	size_t size() const
	{
		size_t size = 0;

		for (const auto& keyElemPair : data_)
		{
			size += keyElemPair.second.size();
		}

		return size;
	}


	/**
	 * @brief  Clears the antichain
	 *
	 * Removes all elements from the antichain.
	 */
	void clear()
	{
		this->data_.clear();
	}


	/**
	 * @brief  Checks whether there is no element
	 *
	 * This method checks whether there is no element in the antichain.
	 *
	 * @returns  @p true in the case the antichain is empty, @p false otherwise
	 */
	inline bool empty()
	{
		return data_.empty();
	}


	/**
	 * @brief  Stream output operator
	 *
	 * Serializes the content of the antichain into a stream.
	 *
	 * @param[in, out]  os  The output stream
	 * @param[in]       ac  The antichain to be serialized
	 *
	 * @returns  Modified output stream
	 */
	friend std::ostream& operator<<(std::ostream& os, const Antichain2Cv2& ac)
	{
		os << '{';

		for (auto& smallerBiggerListPair : ac.data_)
		{
			os << " (" << smallerBiggerListPair.first << ", {";

			for (const T& element : smallerBiggerListPair.second)
			{
				os << ' ' << Util::Convert::ToString(element);
			}

			os << " })";
		}

		return os << " }";
	}
};

#endif
