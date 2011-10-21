/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Source for explicit LTS simulation algorithm.
 *
 *****************************************************************************/

#include <ostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <unordered_set>

#include <cstddef>

#include <vata/util/binary_relation.hh>
#include <vata/util/smart_set.hh>
#include <vata/util/caching_allocator.hh>
#include <vata/util/shared_list.hh>
#include <vata/util/shared_counter.hh>
#include <vata/explicit_lts.hh>

using VATA::Util::BinaryRelation;
using VATA::Util::SmartSet;
using VATA::Util::CachingAllocator;
using VATA::Util::SharedList;
using VATA::Util::SharedCounter;

typedef CachingAllocator<std::vector<size_t>> VectorAllocator;

struct SharedListInitF {

	VectorAllocator& allocator_;

	SharedListInitF(VectorAllocator& allocator) : allocator_(allocator) {}

	void operator()(SharedList<std::vector<size_t>>* list) {

		auto sublist = this->allocator_();

		sublist->clear();

		list->init(sublist);

	}

};

struct StateListElem {
	
	size_t index_;
	class OLRTBlock* block_;
	StateListElem* next_;
	StateListElem* prev_;

	static void link(StateListElem* elem1, StateListElem* elem2) {

		elem1->next_ = elem2;
		elem2->prev_ = elem1;

	}

};

typedef SharedList<std::vector<size_t>> RemoveList;
typedef CachingAllocator<RemoveList, SharedListInitF> RemoveAllocator;

struct OLRTBlock {

	size_t index_;
	StateListElem* _states;
	size_t size_;
	std::vector<RemoveList*> _remove;
	SharedCounter _counter;
	SmartSet _inset;
	std::vector<StateListElem*> tmp_;

protected:

	OLRTBlock(const OLRTBlock&);

	OLRTBlock& operator=(const OLRTBlock&);

public:

	OLRTBlock(const VATA::ExplicitLTS& lts, size_t index, StateListElem* states, size_t size,
		const SharedCounter::Key& key, const SharedCounter::LabelMap& labelMap,
		const size_t& rowSize, SharedCounter::Allocator& allocator) : index_(index),
		_states(states), size_(size), _remove(lts.labels()),
		_counter(key, lts.states(), labelMap, rowSize, allocator), _inset(lts.labels()), tmp_() {

		do {

			assert(states);

			for (auto& a : lts.bwLabels(states->index_))
				this->_inset.add(a);

			states->block_ = this;
			states = states->next_;

		} while (states != this->_states);

	}
	
	OLRTBlock(const VATA::ExplicitLTS& lts, OLRTBlock& parent, StateListElem* states, size_t size,
		size_t index) : index_(index), _states(states), size_(size), _remove(lts.labels()),
		_counter(parent._counter), _inset(lts.labels()), tmp_() {

		do {

			assert(states);

			for (auto& a : lts.bwLabels(states->index_)) {

				parent._inset.removeStrict(a);

				this->_inset.add(a);

			}

			states->block_ = this;
			states = states->next_;

		} while (states != this->_states);

	}

	StateListElem* states() {
		return this->_states;
	}

	void moveToTmp(StateListElem* elem) {

		this->tmp_.push_back(elem);	

	}

	static bool checkList(StateListElem* elem, size_t size) {

		StateListElem* first = elem;

		while (size--) {

			assert(elem);

			elem = elem->next_;

		}

		return elem == first;
		
	}

	std::pair<StateListElem*, size_t> trySplit() {

		assert(this->tmp_.size());

		if (this->tmp_.size() == this->size_) {

			this->tmp_.clear();

			assert(OLRTBlock::checkList(this->_states, this->size_));

			return std::make_pair(nullptr, 0);

		}

		StateListElem* last = this->tmp_.back();

		this->tmp_.pop_back();

		this->_states = last->next_;

		StateListElem::link(last->prev_, last->next_);

		if (this->tmp_.empty()) {

			StateListElem::link(last, last);

			assert(OLRTBlock::checkList(last, 1));
			assert(OLRTBlock::checkList(this->_states, this->size_ - 1));

			--this->size_;

			return std::make_pair(last, 1);

		}

		StateListElem* elem = last;

		for (auto& state : this->tmp_) {

			this->_states = state->next_;

			StateListElem::link(state->prev_, state->next_);
			StateListElem::link(elem, state);

			elem = state;
			
		}

		StateListElem::link(elem, last);

		size_t size = this->tmp_.size() + 1;

		this->tmp_.clear();

		assert(size < this->size_);

		this->size_ -= size;

		assert(OLRTBlock::checkList(last, size));
		assert(OLRTBlock::checkList(this->_states, this->size_));

		return std::make_pair(last, size);;

	}
	
	SharedCounter& counter() {
		return this->_counter;
	}

	SmartSet& inset() {
		return this->_inset;
	}

	size_t index() const {
		return this->index_;
	}
	
	friend std::ostream& operator<<(std::ostream& os, const OLRTBlock& block) {

		assert(block._states);

		os << block.index_ << " (";

		const StateListElem* elem = block._states;

		do {

			os << " " << elem->index_;

			elem = elem->next_;

		} while (elem != block._states);

		return os << " )";

	}
	
};

class SplittingRelation {

	struct Element {

		Element* up_;
		Element* down_;
		Element* left_;
		Element* right_;
		size_t col_;
		size_t row_;

		Element(size_t row = 0, size_t col = 0) : up_(), down_(), left_(), right_(), col_(col),
			row_(row) {}

	};

	std::vector<std::pair<Element*, Element*>> columns_;
	std::vector<std::pair<Element*, Element*>> rows_;
	size_t size_;

	CachingAllocator<Element> allocator_;

	Element* colBegin(size_t col) const {
		return const_cast<Element*>(
			reinterpret_cast<const Element*>(
				reinterpret_cast<const char*>(&this->columns_[col].first) - offsetof(Element, down_)
			)
		);
	}

	Element* colEnd(size_t col) const {
		return const_cast<Element*>(
			reinterpret_cast<const Element*>(
				reinterpret_cast<const char*>(&this->columns_[col].second) - offsetof(Element, up_)
			)
		);
	}

	Element* rowBegin(size_t row) const {
		return const_cast<Element*>(
			reinterpret_cast<const Element*>(
				reinterpret_cast<const char*>(&this->rows_[row].first) - offsetof(Element, right_)
			)
		);
	}

	Element* rowEnd(size_t row) const {
		return const_cast<Element*>(
			reinterpret_cast<const Element*>(
				reinterpret_cast<const char*>(&this->rows_[row].second) - offsetof(Element, left_)
			)
		);
	}

	bool checkCol(size_t i) const {

		assert(i < this->size_);

		Element* tmp = this->columns_[i].first;

		while (tmp != this->columns_[i].second->down_) {

			assert(tmp->up_->down_ == tmp);
			assert(tmp->down_->up_ == tmp);
			assert(tmp->left_->right_ == tmp);
			assert(tmp->right_->left_ == tmp);

			if (tmp->col_ != i)
				return false;

			if (tmp->row_ >= this->size_)
				return false;

			tmp = tmp->down_;

		}

		return true;

	}

	bool checkRow(size_t i) const {

		assert(i < this->size_);

		Element* tmp = this->rows_[i].first;

		while (tmp != this->rows_[i].second->right_) {

			assert(tmp->up_->down_ == tmp);
			assert(tmp->down_->up_ == tmp);
			assert(tmp->left_->right_ == tmp);
			assert(tmp->right_->left_ == tmp);

			if (tmp->row_ != i)
				return false;

			if (tmp->col_ >= this->size_)
				return false;

			tmp = tmp->right_;

		}

		return true;

	}

public:

	GCC_DIAG_OFF(effc++)
	struct IteratorBase {
	GCC_DIAG_ON(effc++)

		typedef std::input_iterator_tag iterator_category;
		typedef size_t difference_type;
		typedef size_t value_type;
		typedef size_t* pointer;
		typedef size_t& reference;

		Element* el_;

		IteratorBase(Element* el) : el_(el) {}
		~IteratorBase() {}

		bool operator==(const IteratorBase& rhs) { return this->el_ == rhs.el_; }
		bool operator!=(const IteratorBase& rhs) { return this->el_ != rhs.el_; }

	};

	GCC_DIAG_OFF(effc++)
	struct ColIterator : public IteratorBase {
	GCC_DIAG_ON(effc++)

		ColIterator(Element* el) : IteratorBase(el) {}

		ColIterator& operator++() {

			this->el_ = this->el_->down_;
			return *this;

		}

		ColIterator operator++(int) {

			return ++ColIterator(this->el_);

		}

		const size_t& operator*() const { return this->el_->row_; }

	};

	GCC_DIAG_OFF(effc++)
	struct RowIterator : public IteratorBase {
	GCC_DIAG_ON(effc++)

		RowIterator(Element* el) : IteratorBase(el) {}

		RowIterator& operator++() {

			this->el_ = this->el_->right_;
			return *this;

		}

		RowIterator operator++(int) {

			return ++RowIterator(this->el_);

		}

		const size_t& operator*() const { return this->el_->col_; }

	};

	struct Column {

		Element*& begin_;
		Element* end_;

		Column(Element*& begin, Element* end) : begin_(begin), end_(end) {}

		ColIterator begin() const { return ColIterator(this->begin_); }
		ColIterator end() const { return ColIterator(this->end_); }

	};

	struct Row {

		Element*& begin_;
		Element* end_;

		Row(Element*& begin, Element* end) : begin_(begin), end_(end) {}

		RowIterator begin() const { return RowIterator(this->begin_); }
		RowIterator end() const { return RowIterator(this->end_); }

	};

public:

	SplittingRelation(size_t maxSize) : columns_(maxSize), rows_(maxSize), size_(), allocator_() {}

	~SplittingRelation() {

		for (size_t i = 0; i < this->size_; ++i) {

			Element* tmp = this->rows_[i].first;

			while (tmp != this->rows_[i].second->right_) {

				this->allocator_.reclaim(tmp);

				tmp = tmp->right_;

			}
			
		}

	}

	template <class Index>
	void init(const Index& index) {

		std::vector<Element*> lastV(index.size());

		for (size_t i = 0; i < index.size(); ++i)
			lastV[i] = this->colBegin(i);

		for (size_t i = 0; i < index.size(); ++i) {

			Element* last =
				this->rowBegin(i);

			Element* el;

			assert(index[i].size());

			for (auto& j: index[i]) {

				assert(j < index.size());

				el = new Element(i, j);
				el->up_ = lastV[j];
				el->left_ = last;

				lastV[j]->down_ = el;
				lastV[j] = el;

				last->right_ = el;
				last = el;

			}

			last->right_ = this->rowEnd(i);
			this->rows_[i].second = last; // last->right_->left_

		}

		this->size_ = index.size();

		for (size_t i = 0; i < index.size(); ++i) {

			lastV[i]->down_ = this->colEnd(i);
			this->columns_[i].second = lastV[i]; // lastV[i]->down_->up_

			assert(this->checkCol(i));
			assert(this->checkRow(i));

		}

	}

	size_t split(size_t index) {

		assert(index < this->size_);

		size_t newIndex = this->size_;

		Element* el, * last;

		// copy column

		el = this->columns_[index].first;

		assert(el);

		last = this->colBegin(newIndex);

		while (el != this->colEnd(index)) {

			Element* tmp = this->allocator_();

			last->down_ = tmp;
			tmp->up_ = last;

			assert(el->row_ < this->size_);

			this->rows_[el->row_].second->right_ = tmp;
			tmp->left_ = this->rows_[el->row_].second;
			tmp->right_ = this->rowEnd(el->row_);
			this->rows_[el->row_].second = tmp; // tmp->right_->left_
			
			tmp->col_ = newIndex;
			tmp->row_ = el->row_;

			last = tmp;

			el = el->down_;

		}

		// put reflexivity

		el = this->allocator_();

		last->down_ = el;
		el->up_ = last;
		el->down_ = this->colEnd(newIndex);
		this->columns_[newIndex].second = el; // el->down_->up_

		el->right_ = this->rowEnd(newIndex);
		el->col_ = newIndex;
		el->row_ = newIndex;

		// copy row

		el = this->rows_[index].first;

		assert(el);

		last = this->rowBegin(newIndex);

		// we have to skip the last one here
		while (el != this->rows_[index].second) {

			Element* tmp = this->allocator_();

			last->right_ = tmp;
			tmp->left_ = last;

			assert(el->col_ < this->size_);

			this->columns_[el->col_].second->down_ = tmp;
			tmp->up_ = this->columns_[el->col_].second;
			tmp->down_ = this->colEnd(el->col_);
			this->columns_[el->col_].second = tmp; // tmp->down_->up_
			
			tmp->col_ = el->col_;
			tmp->row_ = newIndex;

			last = tmp;

			el = el->right_;

		}

		// finish reflexivity
		
		last->right_ = this->columns_[newIndex].second;
		this->columns_[newIndex].second->left_ = last;

		this->rows_[newIndex].second = this->columns_[newIndex].second;

		++this->size_;

		assert(this->checkCol(newIndex));
		assert(this->checkRow(newIndex));
	
		return newIndex;

	}

	Column column(size_t index) const {

		assert(index < this->columns_.size());
		assert(this->checkCol(index));

		return Column(*const_cast<Element**>(&this->columns_[index].first), this->colEnd(index));

	}

	Row row(size_t index) const {

		assert(index < this->rows_.size());
		assert(this->checkRow(index));

		return Row(*const_cast<Element**>(&this->rows_[index].first), this->rowEnd(index));

	}

	void erase(IteratorBase& iter) {

		Element* el = iter.el_;

		el->up_->down_ = el->down_;
		el->down_->up_ = el->up_;
		el->left_->right_ = el->right_;
		el->right_->left_ = el->left_;

		this->allocator_.reclaim(el);

		assert(this->checkCol(el->col_));
		assert(this->checkRow(el->row_));
		
	}

	const size_t& size() const {

		return this->size_;

	}
	
};

class OLRTAlgorithm {

protected:

	void enqueueToRemove(OLRTBlock* block, size_t label, size_t state) {

		if (RemoveList::append(block->_remove[label], state, this->removeAllocator_))
			this->_queue.push_back(std::make_pair(block, label));

	}

	template <class T>
	void buildPre(T& pre, StateListElem* states, size_t label) const {

		std::vector<bool> blockMask(this->_partition.size(), false);

		StateListElem* elem = states;

		do {

			assert(elem);

			for (auto& q : this->_lts.pre(label)[elem->index_]) {

				OLRTBlock* block = this->_index[q].block_;

				assert(block);

				if (blockMask[block->index()])
					continue;

				blockMask[block->index()] = true;

				pre.push_back(block);

			}

			elem = elem->next_;

		} while (elem != states);

	}

	template <class T1, class T2>
	void internalSplit(T1& modifiedBlocks, const T2& remove) {

		std::vector<bool> blockMask(this->_partition.size(), false);

		for (auto& q : remove) {

			assert(q < this->_index.size());

			StateListElem* elem = &this->_index[q];

			OLRTBlock* block = elem->block_;

			assert(block);

			block->moveToTmp(elem);

			assert(block->index() < this->_partition.size());

			if (blockMask[block->index()])
				continue;

			blockMask[block->index()] = true;

			modifiedBlocks.push_back(block);

		}

	}

	template <class T>
	void fastSplit(const T& remove) {

		std::vector<OLRTBlock*> modifiedBlocks;

		this->internalSplit(modifiedBlocks, remove);

		for (auto& block : modifiedBlocks) {

			assert(block);

			auto p = block->trySplit();

			if (!p.first)
				continue;

			auto newBlock = new OLRTBlock(
				this->_lts, *block, p.first, p.second, this->_partition.size()
			);

			this->_partition.push_back(newBlock);

			this->relation_.split(block->index_);

		}

	}

	template <class T>
	void split(std::vector<bool>& removeMask, const T& remove) {

		std::vector<OLRTBlock*> modifiedBlocks;

		this->internalSplit(modifiedBlocks, remove);

		for (auto& block : modifiedBlocks) {

			assert(block);

			auto p = block->trySplit();

			if (!p.first) {

				removeMask[block->index_] = true;

				continue;

			}

			OLRTBlock* newBlock = new OLRTBlock(
				this->_lts, *block, p.first, p.second, this->_partition.size()
			);

			this->_partition.push_back(newBlock);

			this->relation_.split(block->index_);

			removeMask[newBlock->index_] = true;

			newBlock->counter().copyLabels(newBlock->inset(), block->counter());

			for (auto& a : newBlock->inset()) {

				if (!block->_remove[a])
					continue;

				this->_queue.push_back(std::make_pair(newBlock, a));

				newBlock->_remove[a] = block->_remove[a]->copy();

			}

		}

	}

	template <class T>
	void makeBlock(const T& states, size_t blockIndex) {

		assert(states.size() > 0);

		StateListElem* list = &this->_index[states.back()];

		for (auto& q : states) {

			StateListElem::link(list, &this->_index[q]);

			list = list->next_;
			list->index_ = q;

		}

		this->_partition.push_back(
			new OLRTBlock(
				this->_lts,
				blockIndex,
				list,
				states.size(),
				this->_key,
				this->_labelMap,
				this->rowSize_,
				this->vectorAllocator_
			)
		);

	}

	void processRemove(OLRTBlock* block, size_t label) {

		assert(block);

		RemoveList* remove = block->_remove[label];

		block->_remove[label] = nullptr;

		assert(remove);

		std::vector<OLRTBlock*> preList;

		std::vector<bool> removeMask(this->_lts.states());

		this->buildPre(preList, block->states(), label);

		this->split(removeMask, *remove);

		remove->unsafeRelease(
			[this](RemoveList* list){
				this->vectorAllocator_.reclaim(list->subList());
				this->removeAllocator_.reclaim(list);
			}
		);

		for (auto& b1 : preList) {

			SplittingRelation::Row row = this->relation_.row(b1->index_);

			for (auto col = row.begin(); col != row.end(); ++col) {

				if (!removeMask[*col])
					continue;

				assert(b1->index_ != *col);

				this->relation_.erase(col);

				OLRTBlock* b2 = this->_partition[*col];

				for (auto a : b2->inset()) {

					if (!b1->inset().contains(a))
						continue;

					StateListElem* elem = b2->states();

					do {

						assert(elem);

						for (auto& pre : this->_lts.pre(a)[elem->index_]) {

							if (!b1->counter().decr(a, pre))
								this->enqueueToRemove(b1, a, pre);

						}

						elem = elem->next_;

					} while (elem != b2->states());

				}

			}

		}

	}

	static bool isPartition(const std::vector<std::vector<size_t>>& part, size_t states) {
	
		std::vector<bool> mask(states, false);
	
		for (auto& cls : part) {
	
			for (auto& q : cls) {
	
				if (mask[q])
					return false;
	
				mask[q] = true;
	
			}
	
		}

		for (auto b : mask) {

			if (!b)
				return false;

		}
	
		return true;
	
	}

	static bool isConsistent(const std::vector<std::vector<size_t>>& part,
		const BinaryRelation& rel) {

		if (part.size() != rel.size())
			return false;

		for (size_t i = 0; i < rel.size(); ++i) {

			if (!rel.get(i, i))
				return false;

		}

		return true;

	}

private:

	const VATA::ExplicitLTS& _lts;

	VectorAllocator vectorAllocator_;
	RemoveAllocator removeAllocator_;

	std::vector<OLRTBlock*> _partition;
	SplittingRelation relation_;

	std::vector<StateListElem> _index;
	std::vector<std::pair<OLRTBlock*, size_t> > _queue;
	std::vector<size_t> _key;
	std::vector<std::pair<size_t, size_t>> _labelMap;

	size_t rowSize_;

	OLRTAlgorithm(const OLRTAlgorithm&);

	OLRTAlgorithm& operator=(const OLRTAlgorithm&);

public:

	OLRTAlgorithm(const VATA::ExplicitLTS& lts) : _lts(lts), vectorAllocator_(),
		removeAllocator_(SharedListInitF(vectorAllocator_)), _partition(), relation_(lts.states()),
		_index(lts.states()), _queue(), _key(), _labelMap(), rowSize_() {

		assert(this->_index.size());

	}

	~OLRTAlgorithm() {

		for (auto& block : this->_partition)
			delete block;

	}

	void init(const std::vector<std::vector<size_t>>& partition, const BinaryRelation& relation) {

		assert(OLRTAlgorithm::isPartition(partition, this->_lts.states()));
		assert(OLRTAlgorithm::isConsistent(partition, relation));

		// build counter maps

		std::vector<SmartSet> delta1;

		this->_lts.buildDelta1(delta1);

		this->_key.resize(this->_lts.labels()*this->_lts.states(), static_cast<size_t>(-1));
		this->_labelMap.resize(this->_lts.labels());

		this->rowSize_ = std::sqrt(this->_lts.labels());

		if (this->rowSize_ < 16)
			this->rowSize_ = 16;

		size_t x = 0;

		for (size_t a = 0; a < this->_lts.labels(); ++a) {

			this->_labelMap[a].first = x / this->rowSize_;
			this->_labelMap[a].second =
				(x + delta1[a].size() - 1) / this->rowSize_ + ((delta1[a].size())?(1):(0));

			for (auto& q : delta1[a])
				this->_key[a*this->_lts.states() + q] = x++;

		}

		// initilize patition-relation

		for (size_t i = 0; i < partition.size(); ++i)
			this->makeBlock(partition[i], i);

		BinaryRelation::IndexType index;

		relation.buildIndex(index);

		this->relation_.init(index);

		// make initial refinement

		for (size_t a = 0; a < this->_lts.labels(); ++a)
			this->fastSplit(delta1[a]);

		assert(this->relation_.size() == this->_partition.size());

		// prune relation

		std::vector<std::vector<size_t>> pre(this->_partition.size());
		std::vector<std::vector<bool>> noPreMask(
			this->_lts.labels(), std::vector<bool>(this->_partition.size())
		);

		for (auto& block : this->_partition) {

			StateListElem* elem = block->states();

			do {

				for (size_t a = 0; a < this->_lts.labels(); ++a) {

					delta1[a].contains(elem->index_)
						? (pre[block->index_].push_back(a), true)
						: (noPreMask[a][block->index_] = true);

				}

				elem = elem->next_;

			} while (elem != block->states());

		}

		for (auto& b1 : this->_partition) {

			SplittingRelation::Row row = this->relation_.row(b1->index_);

			for (auto& a : pre[b1->index_]) {

				for (auto col = row.begin(); col != row.end(); ++col) {

					assert(a < noPreMask.size());
					assert(*col < noPreMask[a].size());

					if (!noPreMask[a][*col])
						continue;

					assert(b1->index_ != *col);
				
					this->relation_.erase(col);

				}

			}

		}

		// initialize counters

		SmartSet s;

		for (auto& b1 : this->_partition) {

			SplittingRelation::Row row = this->relation_.row(b1->index_);

			std::vector<bool> relatedBlocks(this->_partition.size());

			for (auto col = row.begin(); col != row.end(); ++col)
				relatedBlocks[*col] = true;

			for (auto& a : b1->inset()) {

				for (auto q : delta1[a]) {

					size_t count = 0;

					for (auto r : this->_lts.post(a)[q]) {

						if (relatedBlocks[this->_index[r].block_->index_])
							++count;

					}

					if (count)
						b1->counter().incr(a, q, count);

				}

				s.assignFlat(delta1[a]);

				for (auto col = row.begin(); col != row.end(); ++col) {

					OLRTBlock* b2 = this->_partition[*col];

					StateListElem* elem = b2->states();

					do {

						for (auto& q : this->_lts.pre(a)[elem->index_])
							s.remove(q);

						elem = elem->next_;

					} while (elem != b2->states());

				}

				if (s.empty())
					continue;

				b1->_remove[a] = new RemoveList(new std::vector<size_t>(s.begin(), s.end()));

				this->_queue.push_back(std::make_pair(b1, a));

				assert(s.size() == b1->_remove[a]->subList()->size());

			}

			b1->counter().releaseSingletons();

		}

	}

	void run() {

	    while (!this->_queue.empty()) {

			std::pair<OLRTBlock*, size_t> tmp(this->_queue.back());

			this->_queue.pop_back();

			this->processRemove(tmp.first, tmp.second);

		}

	}
	
	void buildResult(BinaryRelation& result, size_t size) const {

		result.resize(size);

		for (size_t i = 0; i < size; ++i) {

			SplittingRelation::Row row = this->relation_.row(this->_index[i].block_->index_);

			std::vector<bool> relatedBlocks(this->_partition.size());

			for (auto col = row.begin(); col != row.end(); ++col)
				relatedBlocks[*col] = true;

			for (size_t j = 0; j < size; ++j)
				result.set(i, j, relatedBlocks[this->_index[j].block_->index_]);

		}

	}

	friend std::ostream& operator<<(std::ostream& os, const OLRTAlgorithm& alg) {

		for (auto& block : alg._partition)
	  		os << *block;

		os << "relation:" << std::endl;

		for (size_t i = 0; i < alg._partition.size(); ++i) {

			SplittingRelation::Row row = alg.relation_.row(alg._index[i].block_->index_);

			std::vector<bool> relatedBlocks(alg._partition.size());

			for (auto col = row.begin(); col != row.end(); ++col)
				relatedBlocks[*col] = true;

			for (size_t j = 0; j < alg._partition.size(); ++j)
				os << relatedBlocks[j];

			os << std::endl;

		}

		return os;

	}

};

BinaryRelation VATA::ExplicitLTS::computeSimulation(
	const std::vector<std::vector<size_t>>& partition,
	const BinaryRelation& relation,
	size_t outputSize
) {

	if (this->states_ == 0)
		return BinaryRelation();

	OLRTAlgorithm alg(*this);

	alg.init(partition, relation);
	alg.run();

	BinaryRelation result;

	alg.buildResult(result, outputSize);

	return result;

}
