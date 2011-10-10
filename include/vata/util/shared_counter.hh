/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Source for shared counter class.
 *
 *****************************************************************************/

#ifndef _VATA_SHARED_COUNTER_HH_
#define _VATA_SHARED_COUNTER_HH_

#include <vector>
#include <algorithm>

#include <vata/util/caching_allocator.hh>
#include <vata/util/smart_set.hh>

namespace VATA {
		namespace Util {
				class SharedCounter;
		}
}

class VATA::Util::SharedCounter {

public:

	typedef std::vector<size_t> RowData;
	typedef CachingAllocator<RowData> Allocator;
	typedef std::vector<size_t> Key;
	typedef std::vector<std::pair<size_t, size_t>> LabelMap;

private:

	struct Row {
		
		size_t master_;
		RowData* data_;
	
		Row() : master_(0), data_(nullptr) {}
	
	};

	const Key& key_;
	const size_t& states_;
	const LabelMap& labelMap_;
	const size_t& rowSize_;
	Allocator& allocator_;

	std::vector<Row> data_;


protected:

	SharedCounter& operator=(const SharedCounter& rhs);

public:

	SharedCounter(const Key& key, const size_t& states, const LabelMap& labelMap,
		const size_t& rowSize, Allocator& allocator) : key_(key), states_(states),
		labelMap_(labelMap), rowSize_(rowSize), allocator_(allocator), data_() {}

	SharedCounter(SharedCounter& counter) : key_(counter.key_), states_(counter.states_),
		labelMap_(counter.labelMap_), rowSize_(counter.rowSize_), allocator_(counter.allocator_),
		data_() {}

	~SharedCounter() {

		for (auto& row : this->data_) {

			if (!row.data_)
				continue;

			if (!--(*row.data_)[this->rowSize_]) // refCount
				this->allocator_.reclaim(row.data_);

		}

	}

	void releaseSingletons() {

		for (auto& row : this->data_) {

			if (!row.data_)
				continue;

			for (auto& counter : *row.data_) {

				if (!counter)
					continue;

				if (counter < row.master_)
					break;

				// everything is in master

				if (!--(*row.data_)[this->rowSize_]) // refCount
					this->allocator_.reclaim(row.data_);

				row.data_ = nullptr;

			}

		}

	}

	size_t get(size_t label, size_t state) const {

		assert(label*this->states_ + state < this->key_.size());

		size_t index = this->key_[label*this->states_ + state];
		size_t bucket = index / this->rowSize_;
		size_t col = index % this->rowSize_;

		assert(bucket < this->data_.size());

		auto& row = this->data_[bucket];

		if (!row.data_)
			return row.master_;

		assert(this->rowSize_ < row.data_->size());
		assert(col < row.data_->size());

		return (*row.data_)[col];

	}

	void incr(size_t label, size_t state) {

		assert(label*this->states_ + state < this->key_.size());

		size_t index = this->key_[label*this->states_ + state];
		size_t bucket = index / this->rowSize_;
		size_t col = index % this->rowSize_;

		if (bucket >= this->data_.size())
			this->data_.resize(bucket + 1);

		auto& row = this->data_[bucket];

		if (row.master_) {

			assert(row.data_);
			assert(this->rowSize_ < row.data_->size());

			++row.master_;
			++(*row.data_)[col];

			return;
	
		}

		row.master_ = 1;
		row.data_ = this->allocator_();
		row.data_->resize(this->rowSize_ + 1);
		
		(*row.data_)[this->rowSize_] = 1; // refCount

		std::fill(row.data_->begin(), row.data_->end() - 1, 0);

		(*row.data_)[col] = 1;

	} 
	
	size_t decr(size_t label, size_t state) {

		assert(label*this->states_ + state < this->key_.size());

		size_t index = this->key_[label*this->states_ + state];
		size_t bucket = index / this->rowSize_;
		size_t col = index % this->rowSize_;

		assert(bucket < this->data_.size());

		auto& row = this->data_[bucket];

		assert(row.master_);

		if (!row.data_) // everything is in master
			return --row.master_;

		assert(this->rowSize_ < row.data_->size());

		if ((row.master_ == (*row.data_)[col]) || (row.master_ == 2)) {

			// move everything to master

			--row.master_;
	
			size_t result = ((*row.data_)[col] - 1);

			if (!--(*row.data_)[this->rowSize_]) // refCount
				this->allocator_.reclaim(row.data_);

			row.data_ = nullptr;

			return result;

		}

		if ((*row.data_)[this->rowSize_] > 1) { // refCount

			--(*row.data_)[this->rowSize_]; // refCount

			auto newData = this->allocator_();

			newData->resize(this->rowSize_ + 1);

			(*newData)[this->rowSize_] = 1; // refCount

			std::copy(row.data_->begin(), row.data_->end() - 1, newData->begin());

			row.data_ = newData;

		}

		assert((*row.data_)[this->rowSize_] == 1);

		--row.master_;

		return --(*row.data_)[col];

	}

	template <class T>
	void copyLabels(const T& labels, SharedCounter& cnt) {

		size_t sentinel = 0;

		SmartSet rowSet(cnt.data_.size());

		for (auto& label : labels) {

			assert(label < this->labelMap_.size());

			sentinel = std::max(
				sentinel, std::min(cnt.data_.size(), this->labelMap_[label].second)
			);

			for (size_t i = this->labelMap_[label].first;
				i < std::min(cnt.data_.size(), this->labelMap_[label].second); ++i)

				rowSet.add(i);

		}

		this->data_.resize(sentinel);

		for (auto& rowIndex : rowSet) {

			auto& src = cnt.data_[rowIndex];
			auto& dst = this->data_[rowIndex];

			dst.master_ = src.master_;

			if (!src.data_)
				continue;

			assert(this->rowSize_ < src.data_->size());

			++(*src.data_)[this->rowSize_]; // refCount

			dst.data_ = src.data_;

		}

	}

	friend std::ostream& operator<<(std::ostream& os, const SharedCounter& cnt) {

		for (auto& row : cnt.data_) {

			os << row.master_ << ':';
			
			if (row.data_) {

				for (auto& col : *row.data_)
					os << ' ' << col;
	
				os << std::endl;

			}

		}

		return os;

	}

};

#endif
