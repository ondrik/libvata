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
//#include <vata/util/convert.hh>

// Standard library headers
#include <vector>
#include <algorithm>

namespace VATA
{
	namespace Util
	{
		class BinaryRelation;
		class Identity;
	}
}

class VATA::Util::BinaryRelation {

	std::vector<bool> data_;
	size_t rowSize_;
	size_t size_;

protected:

	void realloc(size_t newRowSize, bool defVal) {

		assert(newRowSize);
		std::vector<bool> tmp(newRowSize*newRowSize, defVal);
		std::vector<bool>::const_iterator src = this->data_.begin();
		std::vector<bool>::iterator dst = tmp.begin();
		for (size_t i = 0; i < this->size_; ++i) {
			std::copy(src, src + this->size_, dst);
			src += this->rowSize_;
			dst += newRowSize;
		}
		std::swap(this->data_, tmp);
		this->rowSize_ = newRowSize;

	}

	void grow(size_t newSize, bool defVal = false) {

		assert(this->rowSize_ <= newSize);
		size_t newRowSize = this->rowSize_;
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

	void reset(bool defVal) {
		std::fill(this->data_.begin(), this->data_.end(), defVal);
	}

	void resize(size_t size, bool defVal = false) {

		if (this->rowSize_ < size)
			this->grow(size, defVal);

		this->size_ = size;

	}

	size_t alloc() {

		if (this->size_ >= this->rowSize_)
			this->grow(this->size_ + 1);

		return this->size_++;

	}

	size_t split(size_t i, bool reflexive = true) {

		assert(i < this->size_);

		if (this->size_ >= this->rowSize_)
			this->grow(this->size_ + 1);

		assert((this->size_ + 1)*this->rowSize_ <= this->data_.size());

		// fill collumns
		auto src = this->data_.begin() + i;
		auto end = src + this->size_*this->rowSize_;
		auto dst = this->data_.begin() + this->size_;

		for ( ; src != end; src += this->rowSize_, dst += this->rowSize_)
			*dst = *src;

		// fill rows
		src = this->data_.begin() + i*this->rowSize_;
		dst = this->data_.begin() + this->size_*this->rowSize_;

		std::copy(src, src + this->size_, dst);

		// set the reflexive bit
		*(dst + this->size_) = reflexive;

		return this->size_++;
		
	}

	bool get(size_t r, size_t c) const {

		assert(r < this->size_ && c < this->size_);
		assert(r*this->rowSize_ + c < this->data_.size());
		return this->data_[r*this->rowSize_ + c];

	}

	void set(size_t r, size_t c, bool v) {

		assert(r < this->size_ && c < this->size_);
		assert(r*this->rowSize_ + c < this->data_.size());
		this->data_[r*this->rowSize_ + c] = v;

	}

	size_t size() const { return this->size_; }

public:

	typedef std::vector<std::vector<size_t>> IndexType;

	BinaryRelation(size_t size = 0, bool defVal = false, size_t rowSize = 16)
		: data_(rowSize*rowSize, defVal), rowSize_(rowSize), size_(0) {
		this->resize(size, defVal);
	}
/*
	BinaryRelation(const BinaryRelation& rel)
		: data_(rel.data_), rowSize_(rel.rowSize_), size_(rel.size_) {}
*/
	BinaryRelation(const std::vector<std::vector<bool> >& rel)
		: data_(16*16, false), rowSize_(16), size_(0) {
		this->resize(rel.size(), false);
		for (size_t i = 0; i < rel.size(); ++i) {
			assert(rel[i].size() == rel.size());
			for (size_t j = 0; j < rel.size(); ++j)
				this->set(i, j, rel[i][j]);
		}
	}

	bool sym(size_t r, size_t c) const {
		return this->get(r, c) && this->get(c, r);
	}

	// build equivalence classes
	void buildClasses(std::vector<size_t>& headIndex) const {
		headIndex.resize(this->size_);
		std::vector<size_t> head;
		for (size_t i = 0; i < this->size_; ++i) {
			size_t j = 0;
			while ((j < head.size()) && !this->sym(i, head[j]))
				++j;
			if (j < head.size())
				headIndex[i] = head[j];
			else {
				headIndex[i] = i;
				head.push_back(i);
			}
		}
	}

	// build equivalence classes
	void buildClasses(std::vector<size_t>& index, std::vector<size_t>& head) const {
		index.resize(this->size_);
		head.clear();
		for (size_t i = 0; i < this->size_; ++i) {
			size_t j = 0;
			while ((j < head.size()) && !this->sym(i, head[j]))
				++j;
			if (j < head.size())
				index[i] = j;
			else {
				index[i] = head.size();
				head.push_back(i);
			}
		}
	}

	// and composition
	BinaryRelation& operator&=(const BinaryRelation& rhs) {
		assert(this->size_ == rhs.size_);
		for (size_t i = 0; i < this->size_; ++i) {
			for (size_t j = 0; j < this->size_; ++j)
				this->set(i, j, this->get(i, j) & rhs.get(i,j));
		}
		return *this;
	}

	// transposition
	BinaryRelation& transposed(BinaryRelation& dst) const {
		dst.resize(this->size_);
		for (size_t i = 0; i < this->size_; ++i) {
			for (size_t j = 0; j < this->size_; ++j)
				dst.set(j, i, this->get(i, j));
		}
		return dst;
	}

	// relation index
	void buildIndex(IndexType& dst) const {

		dst.resize(this->size_);

		auto rowStart = this->data_.begin();
		auto src = rowStart;
		auto end = src + this->size_*this->rowSize_;
		auto dstIter = dst.begin();

		for (; src != end; src = rowStart, ++dstIter) {

			auto rowEnd = src + this->size_;
			rowStart = src + this->rowSize_;

			for (size_t i = 0; src != rowEnd; ++src, ++i) {

				if (*src)
					dstIter->push_back(i);

			}

		}

	}

	// inverted relation index
	void buildInvIndex(IndexType& dst) const {
		dst.resize(this->size_);
		for (size_t i = 0; i < this->size_; ++i) {
			for (size_t j = 0; j < this->size_; ++j) {
				if (this->get(i, j))
					dst[j].push_back(i);
			}
		}
	}

	// relation index
	void buildIndex(IndexType& ind, IndexType& inv) const {
		ind.resize(this->size_);
		inv.resize(this->size_);
		for (size_t i = 0; i < this->size_; ++i) {
			for (size_t j = 0; j < this->size_; ++j) {
				if (this->get(i, j)) {
					ind[i].push_back(j);
					inv[j].push_back(i);
				}
			}
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const BinaryRelation& v) {

		for (size_t i = 0; i < v.size_; ++i) {
			for (size_t j = 0; j < v.size_; ++j)
				os << v.get(i, j);
			os << std::endl;
		}

		return os;

	}

};

class VATA::Util::Identity {

	size_t size_;

public:

	bool get(size_t r, size_t c) const {
		assert(r < this->size_);
		assert(c < this->size_);
		return r == c;
	}

	size_t size() const { return this->size_; }

public:

	typedef std::vector<std::vector<size_t>> IndexType;

	explicit Identity(size_t size) : size_(size) {}

	bool sym(size_t r, size_t c) const {
		assert(r < this->size_);
		assert(c < this->size_);
		return r == c;
	}

	// build equivalence classes
	void buildClasses(std::vector<size_t>& headIndex) const {
		headIndex.resize(this->size_);
		std::vector<size_t> head;
		for (size_t i = 0; i < this->size_; ++i) {
			size_t j = 0;
			while ((j < head.size()) && !this->sym(i, head[j]))
				++j;
			if (j < head.size())
				headIndex[i] = head[j];
			else {
				headIndex[i] = i;
				head.push_back(i);
			}
		}
	}

	// build equivalence classes
	void buildClasses(std::vector<size_t>& index, std::vector<size_t>& head) const {
		index.resize(this->size_);
		head.clear();
		for (size_t i = 0; i < this->size_; ++i) {
			size_t j = 0;
			while ((j < head.size()) && !this->sym(i, head[j]))
				++j;
			if (j < head.size())
				index[i] = j;
			else {
				index[i] = head.size();
				head.push_back(i);
			}
		}
	}

	// relation index
	void buildIndex(IndexType& dst) const {
		dst.resize(this->size_, std::vector<size_t>(1));
		for (size_t i = 0; i < this->size_; ++i)
			dst[i][0] = i;
	}

	// inverted relation index
	void buildInvIndex(IndexType& dst) const {
		dst.resize(this->size_, std::vector<size_t>(1));
		for (size_t i = 0; i < this->size_; ++i)
			dst[i][0] = i;
	}

	// relation index
	void buildIndex(IndexType& ind, IndexType& inv) const {
		ind.resize(this->size_, std::vector<size_t>(1));
		inv.resize(this->size_, std::vector<size_t>(1));
		for (size_t i = 0; i < this->size_; ++i) {
			ind[i][0] = i;
			inv[i][0] = i;
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const Identity& v) {

		for (size_t i = 0; i < v.size_; ++i) {
			for (size_t j = 0; j < v.size_; ++j)
				os << v.get(i, j);
			os << std::endl;
		}

		return os;

	}

};

#endif
