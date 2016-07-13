/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    File with the OrdVector class.
 *
 *****************************************************************************/

#ifndef _VATA_ORD_VECTOR_HH_
#define _VATA_ORD_VECTOR_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/convert.hh>

// Standard library headers
#include <vector>
#include <algorithm>

// Boost headers
#include <boost/functional/hash.hpp>


// insert the class into proper namespace
namespace VATA
{
	namespace Util
	{
		template <
			class Key
		>
		class OrdVector;
	}
}


/**
 * @brief  Implementation of a set using ordered vector
 *
 * This class implements the interface of a set (the same interface as
 * std::set) using ordered vector as the underlying data structure.
 *
 * @tparam  Key  Key type: type of the elements contained in the container.
 *               Each elements in a set is also its key.
 */
template
<
	class Key
>
class VATA::Util::OrdVector
{
private:  // Private data types

	typedef VATA::Util::Convert Convert;

	typedef std::vector<Key> VectorType;

public:   // Public data types

	typedef typename VectorType::iterator iterator;
	typedef typename VectorType::const_iterator const_iterator;
	typedef typename VectorType::const_reference const_reference;

private:  // Private data members

	VectorType vec_;


private:  // Private methods

	bool vectorIsSorted() const
	{
		for (auto itVec = vec_.cbegin() + 1; itVec < vec_.cend(); ++itVec)
		{	// check that the vector is sorted
			if (!(*(itVec - 1) < *itVec))
			{	// in case there is an unordered pair (or there is one element twice)
				return false;
			}
		}

		return true;
	}


public:   // Public methods

	OrdVector() :
		vec_()
	{
		// Assertions
		assert(vectorIsSorted());
	}

	explicit OrdVector(const VectorType& vec) :
		vec_(vec)
	{
		// sort
		std::sort(vec_.begin(), vec_.end());

		// remove duplicates
		auto it = std::unique(vec_.begin(), vec_.end());
		vec_.resize(it - vec_.begin());

		// Assertions
		assert(vectorIsSorted());
	}

	OrdVector(std::initializer_list<Key> list) :
		vec_(list)
	{
		// sort
		std::sort(vec_.begin(), vec_.end());

		// remove duplicates
		auto it = std::unique(vec_.begin(), vec_.end());
		vec_.resize(it - vec_.begin());

		// Assertions
		assert(vectorIsSorted());
	}

	explicit OrdVector(const Key& key) :
		vec_(1, key)
	{
		// Assertions
		assert(vectorIsSorted());
	}

	template <class InputIterator>
	OrdVector(InputIterator first, InputIterator last) :
		vec_(first, last)
	{
		// sort
		std::sort(vec_.begin(), vec_.end());

		// remove duplicates
		auto it = std::unique(vec_.begin(), vec_.end());
		vec_.resize(it - vec_.begin());

		// Assertions
		assert(vectorIsSorted());
	}

	OrdVector& operator=(const OrdVector& rhs)
	{
		// Assertions
		assert(rhs.vectorIsSorted());

		if (&rhs != this)
		{
			vec_ = rhs.vec_;
		}

		// Assertions
		assert(vectorIsSorted());

		return *this;
	}


	void insert(const Key& x)
	{
		// Assertions
		assert(vectorIsSorted());

		// perform binary search (cannot use std::binary_search because it is
		// ineffective due to not returning the iterator to the position of the
		// desirable insertion in case the searched element is not present in the
		// range)
		size_t first = 0;
		size_t last = vec_.size();

		if ((last != 0) && (vec_[last-1] < x))
		{	// for the case which would be prevalent
			vec_.push_back(x);
			return;
		}

		while (first < last)
		{	// while the pointers do not overlap
			size_t middle = first + (last - first) / 2;
			if (vec_[middle] == x)
			{	// in case we found x
				return;
			}
			else if (vec_[middle] < x)
			{	// in case middle is less than x
				first = middle + 1;
			}
			else
			{	// in case middle is greater than x
				last = middle;
			}
		}

		vec_.resize(vec_.size() + 1);
		std::copy_backward(vec_.begin() + first, vec_.end() - 1, vec_.end());

		// insert the new element
		vec_[first] = x;

		// Assertions
		assert(vectorIsSorted());
	}


	void insert(const OrdVector& vec)
	{
		// Assertions
		assert(vectorIsSorted());
		assert(vec.vectorIsSorted());

		OrdVector result = this->Union(vec);
		vec_ = result.vec_;

		// Assertions
		assert(vectorIsSorted());
	}


	inline void clear()
	{
		// Assertions
		assert(vectorIsSorted());

		vec_.clear();
	}


	inline size_t size() const
	{
		// Assertions
		assert(vectorIsSorted());

		return vec_.size();
	}


	OrdVector Union(const OrdVector& rhs) const
	{
		// Assertions
		assert(vectorIsSorted());
		assert(rhs.vectorIsSorted());

		VectorType newVector;

		auto lhsIt = vec_.begin();
		auto rhsIt = rhs.vec_.begin();

		while ((lhsIt != vec_.end()) || (rhsIt != rhs.vec_.end()))
		{	// until we get to the end of both vectors
			if (lhsIt == vec_.end())
			{	// if we are finished with the left-hand side vector
				newVector.push_back(*rhsIt);
				++rhsIt;
			}
			else if (rhsIt == rhs.vec_.end())
			{	// if we are finished with the right-hand side vector
				newVector.push_back(*lhsIt);
				++lhsIt;
			}
			else
			{
				if (*lhsIt < *rhsIt)
				{
					newVector.push_back(*lhsIt);
					++lhsIt;
				}
				else if (*rhsIt < *lhsIt)
				{
					newVector.push_back(*rhsIt);
					++rhsIt;
				}
				else
				{	// in case they are equal
					newVector.push_back(*rhsIt);
					++rhsIt;
					++lhsIt;
				}
			}
		}

		OrdVector result(newVector);

		// Assertions
		assert(result.vectorIsSorted());

		return result;
	}

	const_iterator find(const Key& key) const
	{
		// Assertions
		assert(vectorIsSorted());

		size_t first = 0;
		size_t last = vec_.size();

		while (first < last)
		{	// while the pointers do not overlap
			size_t middle = first + (last - first) / 2;
			if (vec_[middle] == key)
			{	// in case we found x
//				return const_iterator(&vec_[middle]);
				return vec_.cbegin() + middle;
			}
			else if (vec_[middle] < key)
			{	// in case middle is less than x
				first = middle + 1;
			}
			else
			{	// in case middle is greater than x
				last = middle;
			}
		}

		return end();
	}


	inline bool empty() const
	{
		// Assertions
		assert(vectorIsSorted());

		return vec_.empty();
	}

	inline const_iterator begin() const
	{
		// Assertions
		assert(vectorIsSorted());

		return vec_.begin();
	}

	inline const_iterator end() const
	{
		// Assertions
		assert(vectorIsSorted());

		return vec_.end();
	}


	inline const_iterator cbegin() const
	{
		// Assertions
		assert(vectorIsSorted());

		return begin();
	}

	inline const_iterator cend() const
	{
		// Assertions
		assert(vectorIsSorted());

		return end();
	}

	/**
	 * @brief  Overloaded << operator
	 *
	 * Overloaded << operator for output stream.
	 *
	 * @see  ToString()
	 *
	 * @param[in]  os    The output stream
	 * @param[in]  vec   Assignment to the variables
	 *
	 * @returns  Modified output stream
	 */
	friend std::ostream& operator<<(std::ostream& os, const OrdVector& vec)
	{
		// Assertions
		assert(vec.vectorIsSorted());

		std::string result = "{";

		for (auto it = vec.cbegin(); it != vec.cend(); ++it)
		{
			result += ((it != vec.begin())? ", " : " ") + Convert::ToString(*it);
		}

		return os << (result + "}");
	}

	bool operator==(const OrdVector& rhs) const
	{
		// Assertions
		assert(vectorIsSorted());
		assert(rhs.vectorIsSorted());

		return (vec_ == rhs.vec_);
	}

	bool operator<(const OrdVector& rhs) const
	{
		// Assertions
		assert(vectorIsSorted());
		assert(rhs.vectorIsSorted());

		return std::lexicographical_compare(vec_.begin(), vec_.end(),
			rhs.vec_.begin(), rhs.vec_.end());
	}

	const std::vector<Key>& ToVector() const
	{
		return vec_;
	}

	bool IsSubsetOf(const OrdVector& bigger) const
	{
		return std::includes(bigger.cbegin(), bigger.cend(),
			this->cbegin(), this->cend());
	}

	bool HaveEmptyIntersection(const OrdVector& rhs) const
	{
		// Assertions
		assert(vectorIsSorted());
		assert(rhs.vectorIsSorted());

		const_iterator itLhs = begin();
		const_iterator itRhs = rhs.begin();

		while ((itLhs != end()) || (itRhs != rhs.end()))
		{	// until we drop out of the array (or find a common element)
			if (*itLhs == *itRhs)
			{	// in case there exists a common element
				return false;
			}
			else if (*itLhs < *itRhs)
			{	// in case the element in lhs is smaller
				++itLhs;
			}
			else
			{	// in case the element in rhs is smaller
				assert(*itLhs > *itRhs);
				++itRhs;
			}
		}

		return true;
	}

	template <typename T>
	friend size_t hash_value(const OrdVector<T>& ordVec);
};

namespace VATA
{
	namespace Util
	{
		template <typename T>
		inline size_t hash_value(const OrdVector<T>& ordVec)
		{
			typedef OrdVector<T> OrdVectorType;
			boost::hash<typename OrdVectorType::VectorType> hasher;
			return hasher(ordVec.vec_);
		}
	}
}

#endif
