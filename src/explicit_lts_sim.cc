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

#include <vata/util/binary_relation.hh>
#include <vata/util/smart_set.hh>
#include <vata/util/caching_allocator.hh>
#include <vata/util/shared_list.hh>
#include <vata/explicit_lts.hh>

using VATA::Util::BinaryRelation;
using VATA::Util::SmartSet;
using VATA::Util::CachingAllocator;
using VATA::Util::SharedList;

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

class SharedCounter {

	const std::vector<size_t>& key_;
	const size_t& states_;
	const std::vector<size_t>& range_;

	std::vector<VectorAllocator::Ptr> data_;

	VectorAllocator& vectorAllocator_;

protected:

	VectorAllocator::Ptr allocateEmptyData(size_t size) {

		auto data = this->vectorAllocator_();

		assert(data);

		data->resize(size);
		
		std::fill(data->begin(), data->end(), 0);

		return data;
		
	}

	SharedCounter& operator=(const SharedCounter& rhs);

public:

	SharedCounter(size_t labels, const std::vector<size_t>& key, const size_t& states,
		const std::vector<size_t>& range, VectorAllocator& vectorAllocator) : key_(key),
		states_(states), range_(range), data_(labels), vectorAllocator_(vectorAllocator) {}

	SharedCounter(SharedCounter& counter) : key_(counter.key_), states_(counter.states_),
		range_(counter.range_), data_(counter.data_.size()),
		vectorAllocator_(counter.vectorAllocator_) {}

	void incr(size_t label, size_t state) {

		assert(label < this->data_.size());

		auto& row = this->data_[label];

		if (row) {

			assert((*row)[0] == 1); // refCount
			assert(2 + this->key_[label*this->states_ + state] < row->size());
	
			++(*row)[1]; // master;
			++(*row)[2 + this->key_[label*this->states_ + state]];

			return;
	
		}

		row = this->vectorAllocator_();
		row->resize(2 + this->range_[label]);
		
		assert(2 + this->key_[label*this->states_ + state] < row->size());

		(*row)[0] = 1; // refCount
		(*row)[1] = 1; // master

		std::fill(row->begin() + 2, row->end(), 0);

		(*row)[2 + this->key_[label*this->states_ + state]] = 1;

	} 
	
	size_t decr(size_t label, size_t state) {

		assert(label < this->data_.size());

		auto*& row = this->data_[label];

		assert(row);
		assert(2 + this->key_[label*this->states_ + state] < row->size());

		if ((*row)[1] == 1) { // master

			assert((*row)[2 + this->key_[label*this->states_ + state]] == 1);

			if ((*row)[0] == 1) { // refCount

				this->vectorAllocator_.reclaim(row);

			} else {
				
				--(*row)[0]; // refCount

			}

			assert(!(row = nullptr));

			return 0;

		}

		if ((*row)[0] > 1) { // refCount

			--(*row)[0]; // refCount

			auto newRow = this->vectorAllocator_();

			newRow->resize(row->size());

			(*newRow)[0] = 1; // refCount

			std::copy(row->begin() + 1, row->end(), newRow->begin() + 1);

			row = newRow;

		}

		--(*row)[1]; // master

		return --(*row)[2 + this->key_[label*this->states_ + state]];

	}

	void copyRow(size_t label, SharedCounter& cnt) {

		assert(label < this->data_.size());
		assert(this->data_.size() == cnt.data_.size());
		assert(cnt.data_[label]);
		assert(this->data_[label] == nullptr);

		++(*cnt.data_[label])[0]; // refCount

		this->data_[label] = cnt.data_[label];

	}

	friend std::ostream& operator<<(std::ostream& os, const SharedCounter& cnt) {

		size_t i = 0;
		
		for (auto& row : cnt.data_) {
			
			if (row) {

				os << i << ": ";
	
				for (auto& col : *row)
					os << col;
	
				os << std::endl;

			}

			++i;

		}

		return os;

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

	void move(StateListElem*& src, StateListElem*& dst) {

		assert(src);

		if (this == this->next_) {

			assert(this == this->prev_);
			assert(this == src);

			src = nullptr;

			if (!dst) {

				dst = this;

				return;

			}

		} else {

			src = this->next_;
			src->prev_ = this->prev_;
			src->prev_->next_ = src;
	
			if (!dst) {
	
				dst = this;
	
				dst->next_ = dst;
				dst->prev_ = dst;

				return;
	
			}

		}
		
		this->next_ = dst;
		this->prev_ = dst->prev_;
		dst->prev_->next_ = this;
		dst->prev_ = this;

	}

};

typedef SharedList<std::vector<size_t>> RemoveList;
typedef CachingAllocator<RemoveList, SharedListInitF> RemoveAllocator;

struct OLRTBlock {

	size_t _index;
	StateListElem* _states;
	std::vector<RemoveList*> _remove;
	SharedCounter _counter;
	SmartSet _inset;
	StateListElem* _tmp;
	std::unordered_set<OLRTBlock*> bigger_;
	std::unordered_set<OLRTBlock*> smaller_;

protected:

	OLRTBlock(const OLRTBlock&);

	OLRTBlock& operator=(const OLRTBlock&);

public:

	OLRTBlock(const VATA::ExplicitLTS& lts, size_t index, StateListElem* states,
		const std::vector<size_t>& key, const std::vector<size_t>& range,
		VectorAllocator& vectorAllocator) : _index(index), _states(states), _remove(lts.labels()),
		_counter(lts.labels(), key, lts.states(), range, vectorAllocator), _inset(lts.labels()),
		_tmp(nullptr), bigger_(), smaller_() {

		StateListElem* elem = this->_states;

		do {

			for (auto& a : lts.bwLabels(elem->index_))
				this->_inset.add(a);

			elem->block_ = this;
			elem = elem->next_;

		} while (elem != this->_states);

	}
	
	OLRTBlock(const VATA::ExplicitLTS& lts, OLRTBlock& parent, size_t index) : _index(index),
		_states(parent._tmp), _remove(lts.labels()), _counter(parent._counter),
		_inset(lts.labels()), _tmp(nullptr), bigger_(), smaller_() {

		assert(this->_states);

		parent._tmp = nullptr;

		StateListElem* elem = this->_states;

		do {

			for (auto& a : lts.bwLabels(elem->index_)) {

				parent._inset.removeStrict(a);

				this->_inset.add(a);

			}

			elem->block_ = this;
			elem = elem->next_;

		} while (elem != this->_states);

	}

	bool isRelated(OLRTBlock* block) {

		return this->bigger_.find(block) != this->bigger_.end();

	} 

	bool eraseIfRelated(OLRTBlock* block) {

		auto iter = this->bigger_.find(block);
		if (iter == this->bigger_.end())
			return false;
	
		this->bigger_.erase(iter);
		block->smaller_.erase(this);

		return true;

	} 

	void makeRelated(OLRTBlock* block) {

		this->bigger_.insert(block);
//		block->smaller_.insert(this);

	}

	void breakRelated(OLRTBlock* block) {

		this->bigger_.erase(block);
//		block->smaller_.erase(this);

	}

	template <class T>
	void initBigger(OLRTBlock* parent, const T& candidates) {

		for (auto& block : candidates) {

			if (block->isRelated(parent))
				block->makeRelated(this);
				
		}

		this->bigger_ = parent->bigger_;
		this->makeRelated(this);

	}

	void initSmaller() {

		for (auto& block : this->bigger_)
			block->smaller_.insert(this);

		this->smaller_.insert(this);

	}

	void initRelation(OLRTBlock* parent) {

		this->bigger_ = parent->bigger_;
		this->smaller_ = parent->smaller_;

		for (auto& bigger : this->bigger_)
			bigger->smaller_.insert(this);

		for (auto& smaller : this->smaller_)
			smaller->bigger_.insert(this);

		this->bigger_.insert(this);
		this->smaller_.insert(this);

	}
	
	StateListElem* states() {
		return this->_states;
	}
	
	StateListElem* tmp() {
		return this->_tmp;
	}

	void moveToTmp(StateListElem& elem) {

		elem.move(this->_states, this->_tmp);	

	}
	
	bool checkEmpty() {

		if (this->_states)
			return false;

		this->_states = this->_tmp;
		this->_tmp = nullptr;

		return true;

	}
	
	SharedCounter& counter() {
		return this->_counter;
	}

	SmartSet& inset() {
		return this->_inset;
	}

	size_t index() const {
		return this->_index;
	}
	
	friend std::ostream& operator<<(std::ostream& os, const OLRTBlock& block) {

		assert(block._states);

		os << block._index << " (";

		const StateListElem* elem = block._states;

		do {

			os << " " << elem->index_;

			elem = elem->next_;

		} while (elem != block._states);

		return os << " )";

	}
	
};

class OLRTAlgorithm {

	const VATA::ExplicitLTS& _lts;

	VectorAllocator vectorAllocator_;
	RemoveAllocator removeAllocator_;

	std::vector<OLRTBlock*> _partition;
	std::vector<StateListElem> _index;
	std::vector<std::pair<OLRTBlock*, size_t> > _queue;
	std::vector<SmartSet> _delta;
	std::vector<SmartSet> _delta1;
	std::vector<size_t> _key;
	std::vector<size_t> _range;

	OLRTAlgorithm(const OLRTAlgorithm&);

	OLRTAlgorithm& operator=(const OLRTAlgorithm&);

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

			StateListElem& elem = this->_index[q];

			OLRTBlock* block = elem.block_;

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

			block->checkEmpty();

			if (!block->_tmp)
				continue;

			auto newBlock = new OLRTBlock(this->_lts, *block, this->_partition.size());

			newBlock->initBigger(block, this->_partition);

			this->_partition.push_back(newBlock);

		}

	}

	template <class T1, class T2>
	void split(T1& removeList, const T2& remove) {

		std::vector<OLRTBlock*> modifiedBlocks;

		this->internalSplit(modifiedBlocks, remove);

		for (auto& block : modifiedBlocks) {

			if (block->checkEmpty()) {

				removeList.push_back(block);

				continue;

			}

			assert(block->_tmp);

			OLRTBlock* newBlock = new OLRTBlock(this->_lts, *block, this->_partition.size());

			newBlock->initRelation(block);

			this->_partition.push_back(newBlock);

			removeList.push_back(newBlock);

			for (auto& a : newBlock->inset()) {

				newBlock->counter().copyRow(a, block->counter());

				if (block->_remove[a]) {

					this->_queue.push_back(std::make_pair(newBlock, a));

					newBlock->_remove[a] = block->_remove[a]->copy();

				}

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
				this->_lts, blockIndex, list, this->_key, this->_range, this->vectorAllocator_
			)
		);

	}

	void processRemove(OLRTBlock* block, size_t label) {

		assert(block);

		RemoveList* remove = block->_remove[label];

		block->_remove[label] = nullptr;

		assert(remove);

		std::vector<OLRTBlock*> preList, removeList;

		this->buildPre(preList, block->states(), label);

		this->split(removeList, *remove);

		remove->unsafeRelease(
			[this](RemoveList* list){
					this->vectorAllocator_.reclaim(list->subList());
					this->removeAllocator_.reclaim(list);
			}
		);

		for (auto& b1 : preList) {

			for (auto& b2 : removeList) {

				assert(b1->index() != b2->index());

				if (!b1->eraseIfRelated(b2))
					continue;

				for (auto a : b2->inset()) {

					if (!b1->inset().contains(a))
						continue;

					StateListElem* elem2 = b2->states();

					do {

						for (auto& pre2 : this->_lts.pre(a)[elem2->index_]) {

							if (!b1->counter().decr(a, pre2))
								this->enqueueToRemove(b1, a, pre2);

						}

						elem2 = elem2->next_;

					} while (elem2 != b2->states());

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

public:

	OLRTAlgorithm(const VATA::ExplicitLTS& lts) : _lts(lts), vectorAllocator_(),
		removeAllocator_(SharedListInitF(vectorAllocator_)), _partition(), _index(lts.states()),
		_queue(), _delta(), _delta1(), _key(), _range() {

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

		this->_lts.buildDelta(this->_delta, this->_delta1);
		this->_key.resize(this->_lts.labels()*this->_lts.states(), static_cast<size_t>(-1));
		this->_range.resize(this->_lts.labels());

		for (size_t a = 0; a < this->_lts.labels(); ++a) {

			this->_range[a] = this->_delta1[a].size();

			size_t x = 0;

			for (auto& q : this->_delta1[a])
				this->_key[a*this->_lts.states() + q] = x++;

		}

		// initilize patition-relation

		for (size_t i = 0; i < partition.size(); ++i)
			this->makeBlock(partition[i], i);

		BinaryRelation::IndexType index;

		relation.buildIndex(index);

		for (size_t i = 0; i < this->_partition.size(); ++i) {

			OLRTBlock*& block = this->_partition[i];

			for (auto& bigger : index[i]) {

				assert(bigger < this->_partition.size());

				block->makeRelated(this->_partition[bigger]);

			}

		}

		// make initial refinement

		for (size_t a = 0; a < this->_lts.labels(); ++a)
			this->fastSplit(this->_delta1[a]);

		// prune relation

		std::vector<std::vector<size_t>> pre(this->_partition.size());
		std::vector<std::vector<OLRTBlock*>> noPre(this->_lts.labels());

		for (auto& block : this->_partition) {

			StateListElem* elem = block->states();

			do {

				for (size_t a = 0; a < this->_lts.labels(); ++a) {

					this->_delta1[a].contains(elem->index_)
						? pre[block->_index].push_back(a)
						: noPre[a].push_back(block);

				}

				elem = elem->next_;

			} while (elem != block->states());

		}

		for (auto& b1 : this->_partition) {

			for (auto& a : pre[b1->_index]) {

				for (auto& b2 : noPre[a]) {

					assert(b1 != b2);

					b1->breakRelated(b2);

				}

			}

			b1->initSmaller();

		}

		// initialize counters

		SmartSet s;

		for (std::vector<OLRTBlock*>::reverse_iterator i = this->_partition.rbegin(); i != this->_partition.rend(); ++i) {

			for (auto& a : (*i)->inset()) {

				for (auto q : this->_delta1[a]) {

					for (auto r : this->_lts.post(a)[q]) {

						if ((*i)->isRelated(this->_index[r].block_))
							(*i)->counter().incr(a, q);

					}

				}

				s.assignFlat(this->_delta1[a]);

				for (auto& b2 : this->_partition) {

					if (!(*i)->isRelated(b2))
						continue;

					StateListElem* elem = b2->states();

					do {

						for (auto& q : this->_lts.pre(a)[elem->index_])
							s.remove(q);

						elem = elem->next_;

					} while (elem != b2->states());

				}

				if (s.empty())
					continue;

				(*i)->_remove[a] = new RemoveList(new std::vector<size_t>(s.begin(), s.end()));

				this->_queue.push_back(std::make_pair(*i, a));

				assert(s.size() == (*i)->_remove[a]->subList()->size());

			}

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

			auto& block = this->_index[i].block_;

			for (size_t j = 0; j < size; ++j)

				result.set(i, j, block->isRelated(this->_index[j].block_));

		}

	}

	friend std::ostream& operator<<(std::ostream& os, const OLRTAlgorithm& alg) {

		for (auto& block : alg._partition)
	  		os << *block;

		os << "relation:" << std::endl;

		for (size_t i = 0; i < alg._partition.size(); ++i) {

			auto& block = alg._index[i].block_;

			for (size_t j = 0; j < alg._partition.size(); ++j)

				os << block->isRelated(alg._index[j].block_);

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
