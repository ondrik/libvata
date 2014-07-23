/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Binary relation class.
 *
 *****************************************************************************/

#ifndef _VATA_BINARY_RELATION_HH_
#define _VATA_BINARY_RELATION_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/transl_weak.hh>
//#include <vata/util/convert.hh>

// Standard library headers
#include <vector>
#include <algorithm>
#include <unordered_map>

namespace VATA
{
	namespace Util
	{
		class BinaryRelation;
		class Identity;
		class DiscontBinaryRelation;
	}
}


/**
 * @brief  A binary relation address continuously
 *
 * A binary relation addressed from indices from 0 to @p size_ - 1
 */
class VATA::Util::BinaryRelation
{
	std::vector<bool> data_;
	size_t rowSize_;
	size_t size_;

protected:

	void realloc(size_t newRowSize, bool defVal)
	{
		// check for sane parameters
		assert(0 < newRowSize);

		std::vector<bool> tmp(newRowSize*newRowSize, defVal);
		std::vector<bool>::const_iterator src = data_.begin();
		std::vector<bool>::iterator dst = tmp.begin();
		for (size_t i = 0; i < size_; ++i) {
			std::copy(src, src + size_, dst);
			src += rowSize_;
			dst += newRowSize;
		}
		std::swap(data_, tmp);
		rowSize_ = newRowSize;
	}

	void grow(size_t newSize, bool defVal = false)
	{
		assert(rowSize_ <= newSize);

		size_t newRowSize = rowSize_;
		while (newRowSize <= newSize)
			newRowSize <<= 1;
		assert(newSize <= newRowSize);
		this->realloc(newRowSize, defVal);

	}
/*
	void shrinkToFit() {

		assert(this->rowSize_ > this->size_);
		size_t newRowSize = this->rowSize_;
		while (this->size_ < (newRowSize >> 1))
			newRowSize >>= 1;
		assert(this->size_ <= newRowSize);
		this->realloc(newRowSize);

	}
*/
public:

	void reset(bool defVal)
	{
		std::fill(data_.begin(), data_.end(), defVal);
	}

	void resize(size_t size, bool defVal = false)
	{

		if (rowSize_ < size)
		{
			this->grow(size, defVal);
		}

		size_ = size;
	}

	size_t alloc()
	{
		if (size_ >= rowSize_)
		{
			this->grow(size_ + 1);
		}

		return size_++;
	}

	size_t split(size_t i, bool reflexive = true)
	{
		assert(i < size_);

		if (size_ >= rowSize_)
		{
			this->grow(size_ + 1);
		}

		assert((size_ + 1)*rowSize_ <= data_.size());

		// fill collumns
		auto src = data_.begin() + i;
		auto end = src + size_*rowSize_;
		auto dst = data_.begin() + size_;

		for ( ; src != end; src += rowSize_, dst += rowSize_)
		{
			*dst = *src;
		}

		// fill rows
		src = data_.begin() + i*rowSize_;
		dst = data_.begin() + size_*rowSize_;

		std::copy(src, src + size_, dst);

		// set the reflexive bit
		*(dst + size_) = reflexive;

		return size_++;
	}

	bool get(size_t r, size_t c) const
	{
		assert(r < size_ && c < size_);
		assert(r*rowSize_ + c < data_.size());

		return data_[r*rowSize_ + c];
	}

	void set(size_t r, size_t c, bool v)
	{
		assert(r < size_ && c < size_);
		assert(r*rowSize_ + c < data_.size());

		data_[r*rowSize_ + c] = v;
	}

	size_t size() const
	{
		return size_;
	}

public:

	using IndexType    = std::vector<std::vector<size_t>>;

	BinaryRelation(
		size_t         size = 0,
		bool           defVal = false,
		size_t         rowSize = 16) :
		data_(rowSize*rowSize, defVal),
		rowSize_(rowSize),
		size_(0)
	{
		this->resize(size, defVal);
	}

/*
	BinaryRelation(const BinaryRelation& rel)
		: data_(rel.data_), rowSize_(rel.rowSize_), size_(rel.size_) {}
*/

	BinaryRelation(const std::vector<std::vector<bool> >& rel) :
		data_(16*16, false),
		rowSize_(16),
		size_(0)
	{
		this->resize(rel.size(), false);
		for (size_t i = 0; i < rel.size(); ++i)
		{
			assert(rel[i].size() == rel.size());
			for (size_t j = 0; j < rel.size(); ++j)
			{
				this->set(i, j, rel[i][j]);
			}
		}
	}

	// TODO: does this do what is expected? What is expected this should do???
	// WHAT ABOUT SOME FREAKING DOCUMENTATION??????
	bool sym(size_t row, size_t column) const
	{
		return this->get(row, column) && this->get(column, row);
	}

	// build equivalence classes
	void buildClasses(std::vector<size_t>& headIndex) const
	{
		headIndex.resize(size_);
		std::vector<size_t> head;
		for (size_t i = 0; i < size_; ++i)
		{
			size_t j = 0;
			while ((j < head.size()) && !this->sym(i, head[j]))
			{
				++j;
			}

			if (j < head.size())
			{
				headIndex[i] = head[j];
			}
			else
			{
				headIndex[i] = i;
				head.push_back(i);
			}
		}
	}

	// build equivalence classes
	void buildClasses(
		std::vector<size_t>&       index,
		std::vector<size_t>&       head) const
	{
		index.resize(size_);
		head.clear();

		for (size_t i = 0; i < size_; ++i)
		{
			size_t j = 0;
			while ((j < head.size()) && !this->sym(i, head[j]))
			{
				++j;
			}

			if (j < head.size())
			{
				index[i] = j;
			}
			else
			{
				index[i] = head.size();
				head.push_back(i);
			}
		}
	}

	// and composition
	BinaryRelation& operator&=(const BinaryRelation& rhs)
	{
		assert(size_ == rhs.size_);

		for (size_t i = 0; i < size_; ++i)
		{
			for (size_t j = 0; j < size_; ++j)
			{
				this->set(i, j, this->get(i, j) && rhs.get(i,j));
			}
		}

		return *this;
	}

	// transposition
	BinaryRelation& transposed(BinaryRelation& dst) const
	{
		dst.resize(size_);
		for (size_t i = 0; i < size_; ++i)
		{
			for (size_t j = 0; j < size_; ++j)
			{
				dst.set(j, i, this->get(i, j));
			}
		}

		return dst;
	}

	// relation index
	void buildIndex(IndexType& dst) const
	{
		dst.resize(size_);

		auto rowStart = data_.begin();
		auto src = rowStart;
		auto end = src + size_*rowSize_;
		auto dstIter = dst.begin();

		for (; src != end; src = rowStart, ++dstIter)
		{
			auto rowEnd = src + size_;
			rowStart = src + rowSize_;

			for (size_t i = 0; src != rowEnd; ++src, ++i)
			{
				if (*src)
				{
					dstIter->push_back(i);
				}
			}
		}
	}

	// inverted relation index
	void buildInvIndex(IndexType& dst) const
	{
		dst.resize(size_);

		for (size_t i = 0; i < size_; ++i)
		{
			for (size_t j = 0; j < size_; ++j)
			{
				if (this->get(i, j))
				{
					dst[j].push_back(i);
				}
			}
		}
	}

	// relation index
	void buildIndex(IndexType& ind, IndexType& inv) const
	{
		ind.resize(size_);
		inv.resize(size_);

		for (size_t i = 0; i < size_; ++i)
		{
			for (size_t j = 0; j < size_; ++j)
			{
				if (this->get(i, j))
				{
					ind[i].push_back(j);
					inv[j].push_back(i);
				}
			}
		}
	}

	friend std::ostream& operator<<(
		std::ostream&             os,
		const BinaryRelation&     v)
	{
		for (size_t i = 0; i < v.size_; ++i)
		{
			for (size_t j = 0; j < v.size_; ++j)
			{
				os << v.get(i, j);
			}

			os << std::endl;
		}

		return os;
	}
};

class VATA::Util::Identity
{
private:  // data members

	size_t size_;

public:   // methods

	bool get(size_t r, size_t c) const
	{
		assert(r < size_);
		assert(c < size_);

		return r == c;
	}

	size_t size() const
	{
		return size_;
	}

public:

	typedef std::vector<std::vector<size_t>> IndexType;

	explicit Identity(size_t size) :
		size_(size)
	{ }

	bool sym(size_t r, size_t c) const
	{
		assert(r < size_);
		assert(c < size_);

		return r == c;
	}

	// build equivalence classes
	void buildClasses(std::vector<size_t>& headIndex) const
	{
		headIndex.resize(size_);
		std::vector<size_t> head;

		for (size_t i = 0; i < size_; ++i)
		{
			size_t j = 0;
			while ((j < head.size()) && !this->sym(i, head[j]))
			{
				++j;
			}

			if (j < head.size())
			{
				headIndex[i] = head[j];
			}
			else
			{
				headIndex[i] = i;
				head.push_back(i);
			}
		}
	}

	// build equivalence classes
	void buildClasses(
		std::vector<size_t>& index,
		std::vector<size_t>& head) const
	{
		index.resize(size_);
		head.clear();
		for (size_t i = 0; i < size_; ++i)
		{
			size_t j = 0;
			while ((j < head.size()) && !this->sym(i, head[j]))
			{
				++j;
			}

			if (j < head.size())
			{
				index[i] = j;
			}
			else
			{
				index[i] = head.size();
				head.push_back(i);
			}
		}
	}

	// relation index
	void buildIndex(IndexType& dst) const
	{
		dst.resize(size_, std::vector<size_t>(1));
		for (size_t i = 0; i < size_; ++i)
		{
			dst[i][0] = i;
		}
	}

	// inverted relation index
	void buildInvIndex(IndexType& dst) const
	{
		dst.resize(size_, std::vector<size_t>(1));
		for (size_t i = 0; i < size_; ++i)
		{
			dst[i][0] = i;
		}
	}

	// relation index
	void buildIndex(IndexType& ind, IndexType& inv) const
	{
		ind.resize(size_, std::vector<size_t>(1));
		inv.resize(size_, std::vector<size_t>(1));

		for (size_t i = 0; i < size_; ++i)
		{
			ind[i][0] = i;
			inv[i][0] = i;
		}
	}

	friend std::ostream& operator<<(
		std::ostream&        os,
		const Identity&      v)
	{
		for (size_t i = 0; i < v.size_; ++i)
		{
			for (size_t j = 0; j < v.size_; ++j)
			{
				os << v.get(i, j);
			}
			os << std::endl;
		}

		return os;
	}
};

/**
 * @brief  A binary relation with discontinuous indexing
 */
class VATA::Util::DiscontBinaryRelation
{
public:   // data types

	using IndexType    = BinaryRelation::IndexType;
	using DictType     = std::unordered_map<size_t, size_t>;
	using TranslType   = VATA::Util::TranslatorWeak<DictType>;

private:  // data members

	/// The underlying binary relation, indexed from 0
	BinaryRelation rel_;

	/// The counter of allocated indices
	size_t indexCnt_;

	/// The mapping of inputs to the range 0..size-1
	DictType dict_;

	/// The translator for indices
	TranslType transl_;

public:   // methods

	/**
	 * @brief  The constructor
	 */
	DiscontBinaryRelation(
		size_t         size = 0,
		bool           defVal = false,
		size_t         rowSize = 16) :
		rel_(size, defVal, rowSize),
		indexCnt_(0),
		dict_(),
		transl_(dict_, [this](const size_t&){return indexCnt_++;})
	{ }


	/**
	 * @brief  Constructor from a binary relation and a dictionary with copy semantics
	 */
	DiscontBinaryRelation(
		const BinaryRelation&      rel,
		const DictType&            dict) :
		rel_(rel),
		indexCnt_(0),
		dict_(dict),
		transl_(dict_, [this](const size_t&){return indexCnt_++;})
	{ }


	/**
	 * @brief  Constructor from a binary relation and a dictionary with move semantics
	 */
	DiscontBinaryRelation(
		BinaryRelation&&      rel,
		DictType&&            dict) :
		rel_(rel),
		indexCnt_(0),
		dict_(dict),
		transl_(dict_, [this](const size_t&){return indexCnt_++;})
	{ }


	/**
	 * @brief  Copy constructor
	 */
	DiscontBinaryRelation(const DiscontBinaryRelation& rhs) :
		rel_(rhs.rel_),
		indexCnt_(rhs.indexCnt_),
		dict_(rhs.dict_),
		transl_(dict_, [this](const size_t&){return indexCnt_++;})
	{ }


	/**
	 * @brief  Move constructor
	 */
	DiscontBinaryRelation(DiscontBinaryRelation&& rhs) :
		rel_(rhs.rel_),
		indexCnt_(rhs.indexCnt_),
		dict_(rhs.dict_),
		transl_(dict_, [this](const size_t&){return indexCnt_++;})
	{ }


	/**
	 * @brief  Assignment operator
	 */
	DiscontBinaryRelation& operator=(const DiscontBinaryRelation& rhs)
	{
		if (this != &rhs)
		{
			rel_ = rhs.rel_;
			dict_ = rhs.dict_;
			indexCnt_ = rhs.indexCnt_;

			// the following (or a similar thing because this does not work) should
			// not be necessary because dict_ is used as a reference
			//
			// transl_ = TranslType(dict_, [this](const size_t&){return indexCnt_++;});
		}

		return *this;
	}


	/**
	 * @brief  Move assignment operator
	 */
	DiscontBinaryRelation& operator=(DiscontBinaryRelation&& rhs)
	{
		assert(this != &rhs);

		rel_ = std::move(rhs.rel_);
		dict_ = std::move(rhs.dict_);
		indexCnt_ = std::move(rhs.indexCnt_);

		// the following (or a similar thing because this does not work) should
		// not be necessary because dict_ is used as a reference
		//
		// transl_ = TranslType(dict_, [this](const size_t&){return indexCnt_++;});

		return *this;
	}


	/**
	 * @brief  Output stream operator
	 */
	friend std::ostream& operator<<(
		std::ostream&                     os,
		const DiscontBinaryRelation&      rel)
	{
		for (size_t i = 0; i < rel.size(); ++i)
		{
			for (size_t j = 0; j < rel.size(); ++j)
			{
				os << rel.get(i, j);
			}
			os << std::endl;
		}

		return os;
	}

	void buildIndex(IndexType& dst) const
	{
		assert(false);
		rel_.buildIndex(dst);
	}

	// relation index
	void buildIndex(IndexType& ind, IndexType& inv) const
	{
		assert(false);
		rel_.buildIndex(ind, inv);
	}

	bool get(size_t row, size_t column) const
	{
		return rel_.get(transl_[row], transl_[column]);
	}

	void set(size_t row, size_t column, bool value)
	{
		assert(false);
		rel_.set(row, column, value);
	}

	size_t size() const
	{
		return rel_.size();
	}
};

#endif
