/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
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

// Standard library headers
#include <vector>
#include <algorithm>


namespace VATA
{
	namespace Util
	{
		class BinaryRelation;
	}
}


class VATA::Util::BinaryRelation {

	std::vector<bool> _data;

protected:

	size_t _cap;
	size_t _size;

	void grow(size_t newCap, bool defVal) {
		assert(this->_cap <= newCap);
		std::vector<bool> tmp(newCap*newCap, defVal);
		std::vector<bool>::const_iterator src = this->_data.begin();
		std::vector<bool>::iterator dst = tmp.begin();
		while (src != this->_data.end()) {
			std::copy(src, src + this->_size, dst);
			src += this->_cap;
			dst += newCap;
		}
		std::swap(this->_data, tmp);
		this->_cap = newCap;
	}

public:

	void reset(bool defVal = false) {
		std::fill(this->_data.begin(), this->_data.end(), defVal);
		this->_size = 0;
	}

	void resize(size_t size, bool defVal = false) {
		size_t newCap = this->_cap;
		while (newCap < size)
			newCap *= 2;
		this->grow(newCap, defVal);
	}

	size_t newEntry(bool defVal = false) {
		if (this->_size == this->_cap)
			this->grow(2*this->_cap, defVal);
		return this->_size++;
	}

	bool get(size_t r, size_t c) const {
		assert(r < this->_size && c < this->_size);
		return this->_data[r*this->_cap + c];
	}

	void set(size_t r, size_t c, bool v) {
		assert(r < this->_size && c < this->_size);
		this->_data[r*this->_cap + c] = v;
	}

	size_t size() const {
		return this->_size;
	}

public:

	typedef std::vector<std::vector<size_t>> IndexType;

	BinaryRelation(size_t size = 0, bool defVal = false, size_t cap = 16)
		: _data((size < cap)?(cap*cap):(size*size), defVal), _cap((size < cap)?(cap):(size)),
		_size(size) {}

	BinaryRelation(const std::vector<std::vector<bool> >& rel)
		: _data(rel.size()*rel.size(), false), _cap(rel.size()), _size(rel.size()) {
		for (size_t i = 0; i < rel.size(); ++i) {
			for (size_t j = 0; j < rel.size(); ++j)
				this->set(i, j, rel[i][j]);
		}
	}

	bool sym(size_t r, size_t c) const {
		assert(r < this->_size && c < this->_size);
		return this->get(r, c) && this->get(c, r);
	}

	// build equivalence classes
	void buildClasses(std::vector<size_t>& headIndex) const {
		headIndex.resize(this->_size);
		std::vector<size_t> head;
		for (size_t i = 0; i < this->_size; ++i) {
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
		index.resize(this->_size);
		head.clear();
		for (size_t i = 0; i < this->_size; ++i) {
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
		assert(this->_size == rhs._size);
		for (size_t i = 0; i < this->_size; ++i) {
			for (size_t j = 0; j < this->_size; ++j)
				this->set(i, j, this->get(i, j) & rhs.get(i,j));
		}
	}

	// transposition
	BinaryRelation& transposed(BinaryRelation& dst) const {
		dst.resize(this->_size);
		for (size_t i = 0; i < this->_size; ++i) {
			for (size_t j = 0; j < this->_size; ++j)
				dst.set(j, i, this->get(i, j));
		}
		return dst;
	}

	// relation index
	void buildIndex(IndexType& dst) const {
		dst.resize(this->_size);
		for (size_t i = 0; i < this->_size; ++i) {
			for (size_t j = 0; j < this->_size; ++j) {
				if (this->get(i, j))
					dst[i].push_back(j);
			}
		}
	}

	// inverted relation index
	void buildInvIndex(IndexType& dst) const {
		dst.resize(this->_size);
		for (size_t i = 0; i < this->_size; ++i) {
			for (size_t j = 0; j < this->_size; ++j) {
				if (this->get(i, j))
					dst[j].push_back(i);
			}
		}
	}

	// relation index
	void buildIndex(IndexType& ind, IndexType& inv) const {
		ind.resize(this->_size);
		inv.resize(this->_size);
		for (size_t i = 0; i < this->_size; ++i) {
			for (size_t j = 0; j < this->_size; ++j) {
				if (this->get(i, j)) {
					ind[i].push_back(j);
					inv[j].push_back(i);
				}
			}
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const BinaryRelation& v) {
		for (size_t i = 0; i < v._size; ++i) {
			for (size_t j = 0; j < v._size; ++j)
				os << v.get(i, j);
			os << std::endl;
		}

		return os;
	}

};

#endif
