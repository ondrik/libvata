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
#include <vata/util/splitting_relation.hh>
#include <vata/util/smart_set.hh>
#include <vata/util/caching_allocator.hh>
#include <vata/util/shared_list.hh>
#include <vata/util/shared_counter.hh>
#include <vata/explicit_lts.hh>

using VATA::Util::BinaryRelation;
using VATA::Util::SplittingRelation;
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

		std::vector<std::vector<size_t>> tmp(this->_partition.size());

		for (size_t i = 0; i < this->_partition.size(); ++i) {

			StateListElem* elem = this->_partition[i]->_states;

			do {

				assert(elem);

				if (elem->index_ < size)
					tmp[i].push_back(elem->index_);

				elem = elem->next_;

			} while (elem != this->_partition[i]->_states);

		}

		for (size_t i = 0; i < this->relation_.size(); ++i) {

			for (auto j : this->relation_.row(i)) {

				for (auto& r : tmp[i]) {

					for (auto& s : tmp[j])
						result.set(r, s, true);

				}

			}

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
