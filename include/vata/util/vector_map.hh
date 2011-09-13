/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a map of vectors to keys
 *
 *****************************************************************************/

#ifndef _VATA_VECTOR_MAP_HH_
#define _VATA_VECTOR_MAP_HH_

// VATA headers
#include <vata/vata.hh>

// Standard library headers
#include <unordered_map>

// Boost library headers
#include <boost/functional/hash.hpp>


// insert the class into proper namespace
namespace VATA
{
	namespace Util
	{
		template
		<
			typename KeyElement,
			typename T
		>
		class VectorMap;
	}
}


/**
 * @brief   Map of vectors to elements
 *
 * This class implements map that projects vectors of elements to arbitrary
 * elements.
 *
 * @tparam  KeyElement  Data type that is used as contained type of the vector.
 * @tparam  T           Data type that is used as the image of the function.
 */
template
<
	typename KeyElement,
	typename T
>
class VATA::Util::VectorMap
{
public:   // Public data types

	typedef KeyElement KeyElementType;

	typedef std::vector<KeyElementType> key_type;
	typedef T mapped_type;
	typedef std::pair<const key_type, mapped_type> value_type;

	typedef VectorMap
		<
			KeyElementType,
			mapped_type
		> Type;

	typedef std::vector<value_type> SameLengthIndexValueVector;
	typedef std::vector<SameLengthIndexValueVector> IndexValueArray;

private:  // Private data types

	typedef VATA::Util::Convert Convert;

	typedef std::pair<KeyElementType, KeyElementType> KeyElementPairType;

	typedef std::unordered_map<KeyElementType, mapped_type,
		boost::hash<KeyElementType>> HashTableUnary;
	typedef std::unordered_map<KeyElementPairType, mapped_type,
		boost::hash<KeyElementPairType>> HashTableBinary;
	typedef std::unordered_map<key_type, mapped_type,
		boost::hash<key_type>> HashTableNnary;


	/**
	 * @brief  Base iterator
	 *
	 * The class for a base iterator.
	 */
	template <
		class ContainerPtr,
		class UnaryIter,
		class BinaryIter,
		class NnaryIter
		>
	struct Tbase_iterator
	{
	private:  // data types

		enum IteratorState
		{
			ITERATOR_INVALID = 0,
			ITERATOR_NULLARY,
			ITERATOR_UNARY,
			ITERATOR_BINARY,
			ITERATOR_NNARY,
			ITERATOR_END
		};

	protected:// data types

		typedef std::pair<key_type, mapped_type&> ref_value_type;
		typedef std::pair<key_type, const mapped_type&> const_ref_value_type;

		typedef UnaryIter UnaryIterator;
		typedef BinaryIter BinaryIterator;
		typedef NnaryIter NnaryIterator;

	private:  // Private data members

		ContainerPtr vecMap_;

		IteratorState state_;

		UnaryIterator itUnary_;
		BinaryIterator itBinary_;
		NnaryIterator itNnary_;

	private:  // Private methods

		inline void reset()
		{
			// Assertions
			assert(vecMap_ != nullptr);

			state_ = ITERATOR_INVALID;
		}

		template <class Tret>
		Tret getIndexValue() const
		{
			// Assertions
			assert(vecMap_ != nullptr);
			assert(state_ != ITERATOR_INVALID);
			assert(state_ != ITERATOR_END);

			key_type index;
			switch (state_)
			{
				case ITERATOR_NULLARY:
					assert(vecMap_->container0_ != nullptr);
					return std::make_pair(index, std::ref(*(vecMap_->container0_)));

				case ITERATOR_UNARY:
					assert(!vecMap_->container1_.empty());
					assert(itUnary_ != vecMap_->container1_.end());
					index.push_back(itUnary_->first);
					return std::make_pair(index, std::ref(itUnary_->second));

				case ITERATOR_BINARY:
					assert(!vecMap_->container2_.empty());
					assert(itBinary_ != vecMap_->container2_.end());
					index.push_back(itBinary_->first.first);
					index.push_back(itBinary_->first.second);
					return std::make_pair(index, std::ref(itBinary_->second));

				case ITERATOR_NNARY:
					assert(!vecMap_->containerN_.empty());
					assert(itNnary_ != vecMap_->containerN_.end());
					return std::make_pair(itNnary_->first, std::ref(itNnary_->second));

				default: assert(false);
			}
		}

	protected:   // methods

		Tbase_iterator() :
			vecMap_(nullptr),
			state_(ITERATOR_INVALID),
			itUnary_(),
			itBinary_(),
			itNnary_()
		{ }

		explicit Tbase_iterator(ContainerPtr vecMap, bool end = false) :
			vecMap_(vecMap),
			state_((end)? ITERATOR_END : ITERATOR_INVALID),
			itUnary_(),
			itBinary_(),
			itNnary_()
		{
			// Assertions
			assert(vecMap_ != nullptr);

			if (state_ != ITERATOR_END)
			{
				reset();
				++(*this);
			}
		}

		Tbase_iterator(const Tbase_iterator& it) :
			vecMap_(it.vecMap_),
			state_(it.state_),
			itUnary_(it.itUnary_),
			itBinary_(it.itBinary_),
			itNnary_(it.itNnary_)
		{
			// Assertions
			assert(vecMap_ != nullptr);
		}

		Tbase_iterator(ContainerPtr vecMap,
			mapped_type& /* itNullary */) :
			vecMap_(vecMap),
			state_(ITERATOR_NULLARY),
			itUnary_(),
			itBinary_(),
			itNnary_()
		{
			// Assertions
			assert(vecMap_ != nullptr);
		}

		Tbase_iterator(ContainerPtr vecMap,
			UnaryIterator& itUnary) :
			vecMap_(vecMap),
			state_(ITERATOR_UNARY),
			itUnary_(itUnary),
			itBinary_(),
			itNnary_()
		{
			// Assertions
			assert(vecMap_ != nullptr);
		}

		Tbase_iterator(ContainerPtr vecMap,
			BinaryIterator& itBinary) :
			vecMap_(vecMap),
			state_(ITERATOR_BINARY),
			itUnary_(),
			itBinary_(itBinary),
			itNnary_()
		{
			// Assertions
			assert(vecMap_ != nullptr);
		}

		Tbase_iterator(ContainerPtr vecMap,
			NnaryIterator& itNnary) :
			vecMap_(vecMap),
			state_(ITERATOR_NNARY),
			itUnary_(),
			itBinary_(),
			itNnary_(itNnary)
		{
			// Assertions
			assert(vecMap_ != nullptr);
		}

		Tbase_iterator& operator=(const Tbase_iterator& rhs)
		{
			if (&rhs != this)
			{
				vecMap_ = rhs.vecMap_;
				state_ = rhs.state_;
				itUnary_ = rhs.itUnary_;
				itBinary_ = rhs.itBinary_;
				itNnary_ = rhs.itNnary_;
			}

			return *this;
		}

		inline const_ref_value_type const_deref() const
		{
			// Assertions
			assert(vecMap_ != nullptr);
			assert(state_ != ITERATOR_INVALID);

			return getIndexValue<const_ref_value_type>();
		}

		inline ref_value_type deref() const
		{
			// Assertions
			assert(vecMap_ != nullptr);
			assert(state_ != ITERATOR_INVALID);

			return getIndexValue<ref_value_type>();
		}

		bool operator==(const Tbase_iterator& rhs) const
		{
			// Assertions
			assert(state_ != ITERATOR_INVALID);
			assert(vecMap_ != nullptr);

			if (vecMap_ != rhs.vecMap_)
			{
				throw std::logic_error(__func__ +
					std::string(": an attempt to compare iterators from different containers"));
			}

			if (state_ != rhs.state_)
			{
				return false;
			}

			switch (state_)
			{
				case ITERATOR_INVALID: return false; break;
				case ITERATOR_NULLARY: return true; break;
				case ITERATOR_UNARY: return itUnary_ == rhs.itUnary_; break;
				case ITERATOR_BINARY: return itBinary_ == rhs.itBinary_; break;
				case ITERATOR_NNARY: return itNnary_ == rhs.itNnary_; break;
				case ITERATOR_END: return true; break;
				default: throw std::logic_error(__func__ +
					std::string(": invalid state")); break;
			}
		}

		inline bool operator!=(const Tbase_iterator& rhs) const
		{
			// Assertions
			assert(vecMap_ != nullptr);

			return !(*this == rhs);
		}

	public:

		Tbase_iterator& operator++()
		{
			// Assertions
			assert(vecMap_ != nullptr);

			bool sound = false;

			while (!sound)
			{	// until we reach a sound value
				switch (state_)
				{
					case ITERATOR_INVALID:
						state_ = ITERATOR_NULLARY;
						if (vecMap_->container0_)
						{
							sound = true;
						}

						break;

					case ITERATOR_NULLARY:
						state_ = ITERATOR_UNARY;
						itUnary_ = vecMap_->container1_.begin();
						if (itUnary_ != vecMap_->container1_.end())
						{
							sound = true;
							++itUnary_;
						}

						break;

					case ITERATOR_UNARY:
						if (itUnary_ == vecMap_->container1_.end())
						{
							state_ = ITERATOR_BINARY;
							itBinary_ = vecMap_->container2_.begin();
							if (itBinary_ != vecMap_->container2_.end())
							{
								sound = true;
								++itBinary_;
							}
						}
						else
						{
							sound = true;
							++itUnary_;
						}

						break;

					case ITERATOR_BINARY:
						if (itBinary_ == vecMap_->container2_.end())
						{
							state_ = ITERATOR_NNARY;
							itNnary_ = vecMap_->containerN_.begin();
							if (itNnary_ != vecMap_->containerN_.end())
							{
								sound = true;
								++itNnary_;
							}
						}
						else
						{
							sound = true;
							++itBinary_;
						}

						break;

					case ITERATOR_NNARY:
						if (itNnary_ == vecMap_->containerN_.end())
						{
							sound = true;
							state_ = ITERATOR_END;
						}
						else
						{
							sound = true;
							++itNnary_;
						}

						break;

					default: throw std::logic_error(__func__ +
						std::string(": invalid attempt to increment iterator")); break;
				}
			}

			return *this;
		}
	};


	struct Titerator;

	GCC_DIAG_OFF(effc++)
	struct Tconst_iterator :
		public Tbase_iterator<
			const Type*,
			typename HashTableUnary::const_iterator,
			typename HashTableBinary::const_iterator,
			typename HashTableNnary::const_iterator
		>
	{
	GCC_DIAG_ON(effc++)

	private:  // data types

		typedef Tbase_iterator<
			const Type*,
			typename HashTableUnary::const_iterator,
			typename HashTableBinary::const_iterator,
			typename HashTableNnary::const_iterator
			> BaseType;

		typedef typename BaseType::ref_value_type ref_value_type;
		typedef typename BaseType::const_ref_value_type const_ref_value_type;

	public:   // methods

		Tconst_iterator() :
			BaseType()
		{ }

		explicit Tconst_iterator(const Type* vecMap, bool end = false) :
			BaseType(vecMap, end)
		{ }

		Tconst_iterator(const Tconst_iterator& it) :
			BaseType(it)
		{ }

		Tconst_iterator(const Type* vecMap,
			const mapped_type& itNullary) :
			BaseType(vecMap, itNullary)
		{ }

		Tconst_iterator(const Type* vecMap,
			typename BaseType::UnaryIterator itUnary) :
			BaseType(vecMap, itUnary)
		{ }

		Tconst_iterator(const Type* vecMap,
			typename BaseType::BinaryIterator itBinary) :
			BaseType(vecMap, itBinary)
		{ }

		Tconst_iterator(const Type* vecMap,
			typename BaseType::NnaryIterator itNnary) :
			BaseType(vecMap, itNnary)
		{ }

		inline const_ref_value_type operator*() const
		{
			return BaseType::const_deref();
		}

		bool operator==(const Tconst_iterator& rhs) const
		{
			return BaseType::operator==(rhs);
		}

		bool operator!=(const Tconst_iterator& rhs) const
		{
			return !operator==(rhs);
		}
	};

	GCC_DIAG_OFF(effc++)
	struct Titerator :
		public Tbase_iterator <
			Type*,
			typename HashTableUnary::iterator,
			typename HashTableBinary::iterator,
			typename HashTableNnary::iterator
		>
	{
	GCC_DIAG_ON(effc++)

	private:  // data types

		typedef Tbase_iterator<
			Type*,
			typename HashTableUnary::iterator,
			typename HashTableBinary::iterator,
			typename HashTableNnary::iterator
			> BaseType;

		typedef typename BaseType::ref_value_type ref_value_type;
		typedef typename BaseType::const_ref_value_type const_ref_value_type;

	public:   // methods

		Titerator() :
			BaseType()
		{ }

		explicit Titerator(Type* vecMap, bool end = false) :
			BaseType(vecMap, end)
		{ }

		Titerator(const Titerator& it) :
			BaseType(it)
		{ }

		Titerator(Type* vecMap,
			mapped_type& itNullary) :
			BaseType(vecMap, itNullary)
		{ }

		Titerator(Type* vecMap,
			typename BaseType::UnaryIterator itUnary) :
			BaseType(vecMap, itUnary)
		{ }

		Titerator(Type* vecMap,
			typename BaseType::BinaryIterator itBinary) :
			BaseType(vecMap, itBinary)
		{ }

		Titerator(Type* vecMap,
			typename BaseType::NnaryIterator itNnary) :
			BaseType(vecMap, itNnary)
		{ }

		inline ref_value_type operator*()
		{
			return BaseType::deref();
		}

		bool operator==(const Titerator& rhs) const
		{
			return BaseType::operator==(rhs);
		}

		bool operator!=(const Titerator& rhs) const
		{
			return !operator==(rhs);
		}
	};


public:   // Public data types

	typedef Tconst_iterator const_iterator;
	typedef Titerator iterator;

private:  // Private data members

	mapped_type* container0_;

	HashTableUnary container1_;

	HashTableBinary container2_;

	HashTableNnary containerN_;

private:  // Private methods

	inline iterator findForArity0(const key_type& lhs)
	{
		// Assertions
		assert(lhs.size() == 0);

		if (container0_ == nullptr)
		{
			return end();
		}
		else
		{
			return iterator(this, *container0_);
		}
	}

	inline const_iterator findForArity0(const key_type& lhs) const
	{
		// Assertions
		assert(lhs.size() == 0);

		if (container0_ == nullptr)
		{
			return end();
		}
		else
		{
			return const_iterator(this, *container0_);
		}
	}

	inline iterator findForArity1(const key_type& lhs)
	{
		// Assertions
		assert(lhs.size() == 1);

		return iterator(this, container1_.find(lhs[0]));
	}

	inline const_iterator findForArity1(const key_type& lhs) const
	{
		// Assertions
		assert(lhs.size() == 1);

		return const_iterator(this, container1_.find(lhs[0]));
	}

	inline iterator findForArity2(const key_type& lhs)
	{
		// Assertions
		assert(lhs.size() == 2);

		return iterator(this, container2_.find(std::make_pair(lhs[0], lhs[1])));
	}

	inline const_iterator findForArity2(const key_type& lhs) const
	{
		// Assertions
		assert(lhs.size() == 2);

		return const_iterator(this, container2_.find(std::make_pair(lhs[0], lhs[1])));
	}

	inline iterator findForArityN(const key_type& lhs)
	{
		// Assertions
		assert(lhs.size() > 2);

		return iterator(this, containerN_.find(lhs));
	}

	inline const_iterator findForArityN(const key_type& lhs) const
	{
		// Assertions
		assert(lhs.size() > 2);

		return const_iterator(this, containerN_.find(lhs));
	}

	inline std::pair<iterator, bool> insertForArity0(const value_type& ivt)
	{
		// Assertions
		assert(ivt.first.size() == 0);

		if (container0_ != nullptr)
		{	// in case there is something
			return std::make_pair(iterator(this, container0_), false);
		}
		else
		{	// in case there is nothing
			container0_ = new mapped_type(ivt.second);
			return std::make_pair(iterator(this, container0_), true);
		}
	}

	inline std::pair<iterator, bool> insertForArity1(const value_type& ivt)
	{
		// Assertions
		assert(ivt.first.size() == 1);

		assert(false);
	}

	inline std::pair<iterator, bool> insertForArity2(const value_type& ivt)
	{
		// Assertions
		assert(ivt.first.size() == 2);

		assert(false);
	}

	inline std::pair<iterator, bool> insertForArityN(const value_type& ivt)
	{
		// Assertions
		assert(ivt.first.size() > 2);

		assert(false);
	}

//	inline void setValueForArity0(const key_type& lhs, const mapped_type& value)
//	{
//		// Assertions
//		assert(lhs.size() == 0);
//
//		container0_ = value;
//	}
//
//	inline void setValueForArity1(const key_type& lhs, const mapped_type& value)
//	{
//		// Assertions
//		assert(lhs.size() == 1);
//
//		typename HashTableUnary::iterator itHash;
//		if ((itHash = container1_.find(lhs[0])) == container1_.end())
//		{
//			container1_.insert(std::make_pair(lhs[0], value));
//		}
//		else
//		{
//			itHash->second = value;
//		}
//	}
//
//	inline void setValueForArity2(const key_type& lhs, const mapped_type& value)
//	{
//		// Assertions
//		assert(lhs.size() == 2);
//
//		typename HashTableBinary::iterator itHash;
//		if ((itHash = container2_.find(std::make_pair(lhs[0], lhs[1]))) == container2_.end())
//		{
//			container2_.insert(std::make_pair(std::make_pair(lhs[0], lhs[1]), value));
//		}
//		else
//		{
//			itHash->second = value;
//		}
//	}
//
//	inline void setValueForArityN(const key_type& lhs, const mapped_type& value)
//	{
//		// Assertions
//		assert(lhs.size() > 2);
//
//		typename HashTableNnary::iterator itHash;
//		if ((itHash = containerN_.find(lhs)) == containerN_.end())
//		{
//			containerN_.insert(std::make_pair(lhs, value));
//		}
//		else
//		{
//			itHash->second = value;
//		}
//	}

public:   // Public methods

	VectorMap()
		: container0_(nullptr),
			container1_(),
			container2_(),
			containerN_()
	{ }

	VectorMap(const VectorMap& vecMap)
		: container0_((vecMap.container0_ == nullptr)?
				nullptr : new mapped_type(*(vecMap.container0_))),
			container1_(vecMap.container1_),
			container2_(vecMap.container2_),
			containerN_(vecMap.containerN_)
	{ }

	VectorMap(VectorMap&& vecMap)
		: container0_(vecMap.container0_),
			container1_(vecMap.container1_),
			container2_(vecMap.container2_),
			containerN_(vecMap.containerN_)
	{
		vecMap.container0_ = nullptr;
	}

	VectorMap& operator=(const VectorMap& vecMap)
	{
		if (&vecMap != this)
		{
			if (vecMap.container0_ != nullptr)
			{
				container0_ = new mapped_type(*(vecMap.container0_));
			}
			else
			{
				container0_ = nullptr;
			}

			container1_ = vecMap.container1_;
			container2_ = vecMap.container2_;
			containerN_ = vecMap.containerN_;
		}

		return *this;
	}

	VectorMap& operator=(VectorMap&& vecMap)
	{
		if (&vecMap != this)
		{
			container0_ = vecMap.container0_;
			vecMap.container0_ = nullptr;
			container1_ = vecMap.container1_;
			container2_ = vecMap.container2_;
			containerN_ = vecMap.containerN_;
		}

		return *this;
	}

	inline iterator find(const key_type& index)
	{
		switch (index.size())
		{
			case 0: return findForArity0(index); break;
			case 1: return findForArity1(index); break;
			case 2: return findForArity2(index); break;
			default: return findForArityN(index); break;
		}
	}

	inline const_iterator find(const key_type& index) const
	{
		switch (index.size())
		{
			case 0: return findForArity0(index); break;
			case 1: return findForArity1(index); break;
			case 2: return findForArity2(index); break;
			default: return findForArityN(index); break;
		}
	}

	inline std::pair<iterator, bool> insert(const value_type& ivt)
	{
		switch (ivt.first.size())
		{
			case 0: return insertForArity0(ivt); break;
			case 1: return insertForArity1(ivt); break;
			case 2: return insertForArity2(ivt); break;
			default: return insertForArityN(ivt); break;
		}
	}


//	inline void SetValue(const key_type& index, const mapped_type& value)
//	{
//		switch (index.size())
//		{
//			case 0: setValueForArity0(index, value); break;
//			case 1: setValueForArity1(index, value); break;
//			case 2: setValueForArity2(index, value); break;
//			default: setValueForArityN(index, value); break;
//		}
//	}

	template <template <typename> class TSet>
	IndexValueArray GetItemsWith(const KeyElementType& elem,
		const TSet<KeyElementType>& elemDomain) const
	{
		typedef TSet<KeyElementType> DomainSetType;
		// start with arrays for nullary, unary and binary vectors
		IndexValueArray result(3);


		{	// for unary items
			typename HashTableUnary::const_iterator itUnary;
			if ((itUnary = container1_.find(elem)) != container1_.end())
			{	// in case the value is in the hash table
				key_type index;
				index.push_back(elem);
				value_type valuePair = std::make_pair(index, itUnary->second);

				result[1].push_back(valuePair);
			}
		}


		// for binary items
		for (typename DomainSetType::const_iterator itDom = elemDomain.begin();
			itDom != elemDomain.end(); ++itDom)
		{
			// when desired element is at the first position
			KeyElementPairType binaryKey;
			binaryKey.first = elem;
			binaryKey.second = *itDom;

			typename HashTableBinary::const_iterator itBinary;
			if ((itBinary = container2_.find(binaryKey)) != container2_.end())
			{	// in case the value is in the hash table
				key_type index;
				index.push_back(elem);
				index.push_back(*itDom);
				value_type valuePair = std::make_pair(index, itBinary->second);

				result[2].push_back(valuePair);
			}

			if (elem == *itDom)
			{	// in case the first and second element is the same, return only once
				continue;
			}

			// when desired element is at the second position
			binaryKey.first = *itDom;
			binaryKey.second = elem;

			if ((itBinary = container2_.find(binaryKey)) != container2_.end())
			{	// in case the value is in the hash table
				key_type index;
				index.push_back(*itDom);
				index.push_back(elem);
				value_type valuePair = std::make_pair(index, itBinary->second);

				result[2].push_back(valuePair);
			}
		}


		// for n-nary items
		for (typename HashTableNnary::const_iterator itNnary = containerN_.begin();
			itNnary != containerN_.end(); ++itNnary)
		{	// traverse the whole container
			const key_type& vec = itNnary->first;
			for (auto indexElem : vec)
			{	// try to find desired element in the vector
				if (indexElem == elem)
				{	// in case it is there
					value_type valuePair = std::make_pair(vec, itNnary->second);

					while (result.size() <= vec.size())
					{
						result.push_back(SameLengthIndexValueVector());
					}

					result[vec.size()].push_back(valuePair);

					break;
				}
			}
		}

		return result;
	}

	inline void insert(const VectorMap& vecMap)
	{
		// copy all vectors (without the nullary one)
		container1_.insert(vecMap.container1_.begin(), vecMap.container1_.end());
		container2_.insert(vecMap.container2_.begin(), vecMap.container2_.end());
		containerN_.insert(vecMap.containerN_.begin(), vecMap.containerN_.end());
	}

	inline const_iterator begin() const
	{
		return const_iterator(this);
	}

	inline iterator begin()
	{
		return iterator(this);
	}

	inline const_iterator end() const
	{
		return const_iterator(this, true);
	}

	inline iterator end()
	{
		return iterator(this, true);
	}

	inline const_iterator cbegin() const
	{
		return begin();
	}

	inline iterator cbegin()
	{
		return begin();
	}

	inline const_iterator cend() const
	{
		return end();
	}

	inline iterator cend()
	{
		return end();
	}

	inline bool empty() const
	{
		return (container0_ == nullptr) && container1_.empty() &&
			container2_.empty() && containerN_.empty();
	}

	inline size_t size() const
	{
		return ((container0_ != nullptr)? 1 : 0) + container1_.size() +
			container2_.size() + containerN_.size();
	}

	inline void clear()
	{
		container0_ = nullptr;
		container1_.clear();
		container2_.clear();
		containerN_.clear();
	}

	/**
	 * @brief  Overloaded << operator
	 *
	 * Overloaded << operator for output stream.
	 *
	 * @see  ToString()
	 *
	 * @param[in]  os      The output stream
	 * @param[in]  vecMap  Vector map
	 *
	 * @returns  Modified output stream
	 */
	friend std::ostream& operator<<(std::ostream& os, const VectorMap& vecMap)
	{
		os << "[";		// opening tag
		for (auto it = vecMap.cbegin(); it != vecMap.cend(); ++it)
		{	// for each element of the unordered map
			if (it != vecMap.cbegin())
			{	// if we are not at the first element
				os << ", ";
			}

			// the string of the element
			os << Convert::ToString(it->first) << " -> " <<
				Convert::ToString(it->second);
		}

		os << "]";		// closing tag

		return os;
	}

	~VectorMap()
	{
		if (container0_ != nullptr)
		{
			delete container0_;
		}
	}
};

#endif
