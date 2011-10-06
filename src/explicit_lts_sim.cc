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

#include <vata/explicit_lts_sim.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/smart_set.hh>
#include <vata/util/lts.hh>

using VATA::Util::BinaryRelation;
using VATA::Util::SmartSet;
using VATA::Util::LTS;

template <class T>
class CachingAllocator {

public:

	typedef T* Ptr;
	typedef std::shared_ptr<T> SharedPtr;

protected:

	struct DeleteElementF {

		CachingAllocator& allocator_;

		DeleteElementF(CachingAllocator& allocator) : allocator_(allocator) {}

		void operator()(Ptr v) {

			this->allocator_.reclaim(v);

		}

	};

private:

	std::vector<T*> store_;

public:

	CachingAllocator() : store_() {}

	~CachingAllocator() {

		for (auto& element : this->store_)
			delete element;

	}

	Ptr get() {

		if (this->store_.empty())
			return new T();

		Ptr ptr = this->store_.back();

		this->store_.pop_back();

		return ptr;

	}

	void reclaim(Ptr ptr) {

		this->store_.push_back(ptr);

	}

	SharedPtr getShared() {

		return SharedPtr(this->get(), DeleteElementF(*this));

	}

	SharedPtr getShared(Ptr ptr) {

		return SharedPtr(ptr, DeleteElementF(*this));

	}

};

typedef CachingAllocator<std::vector<size_t>> VectorAllocator;

class Counter {

	std::vector<std::vector<size_t> > _data;
	const std::vector<std::vector<size_t> >& _key;
	const std::vector<size_t>& _range;
	
public:

	Counter(size_t labels, const std::vector<std::vector<size_t> >& key, const std::vector<size_t>& range)
		: _data(labels), _key(key), _range(range) {}

	Counter(const Counter& counter)
		: _data(counter._data.size()), _key(counter._key), _range(counter._range) {}

	Counter& operator=(const Counter& rhs) {

		this->_data = rhs._data;

		return *this;

	}

	size_t incr(size_t label, size_t state) {

		assert(label < this->_data.size());

		if (this->_data[label].size() == 0)
			this->_data[label].resize(this->_range[label]);

		return ++this->_data[label][this->_key[label][state]];

	} 
	
	size_t decr(size_t label, size_t state) {

		assert(label < this->_key.size());
		assert(label < this->_data.size());
		assert(state < this->_key[label].size());
		assert(this->_key[label][state] < this->_data[label].size());

		return --this->_data[label][this->_key[label][state]];

	}

	void copyRow(size_t label, const Counter& c) {

		assert(label < this->_data.size());
		assert(label < c._data.size());

		this->_data[label] = c._data[label];

	}

	friend std::ostream& operator<<(std::ostream& os, const Counter& cnt) {

		size_t i = 0;
		
		for (auto& row : cnt._data) {
			
			if (row.size()) {

				os << i << ": ";
	
				for (auto& col : row)
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

	void move(StateListElem*& src, StateListElem*& dst) {

		assert(src);

		if (this == this->next_)

			src = nullptr;

		else {

			src = this->next_;

			this->next_->prev_ = this->prev_;
			this->prev_->next_ = this->next_;

		}

		if (!dst) {

			dst = this;

			this->next_ = this;
			this->prev_ = this;

		} else {

			this->next_ = dst;
			this->prev_ = dst->prev_;
			this->next_->prev_ = this;
			this->prev_->next_ = this;

		}

	}

};

class OLRTBlock {

	size_t _index;
	StateListElem* _states;
	std::vector<VectorAllocator::Ptr> _remove;
	Counter _counter;
	SmartSet _inset;
	StateListElem* _tmp;

	OLRTBlock(const OLRTBlock&);

	OLRTBlock& operator=(const OLRTBlock&);

public:

	OLRTBlock(const LTS& lts, size_t index, StateListElem* states,
		const std::vector<std::vector<size_t> >& key, const std::vector<size_t>& range) :
		_index(index), _states(states), _remove(lts.labels()), _counter(lts.labels(), key, range),
		_inset(lts.labels()), _tmp(nullptr) {

		for (size_t q = 0; q < lts.states(); ++q) {

			for (auto& a : lts.bwLabels(q))
				this->_inset.add(a);

		}

	}
	
	OLRTBlock(const LTS& lts, OLRTBlock& parent, size_t index) : _index(index),
		_states(parent._tmp), _remove(lts.labels()), _counter(parent._counter),
		_inset(lts.labels()), _tmp(nullptr) {

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
	
	Counter& counter() {
		return this->_counter;
	}

	SmartSet& inset() {
		return this->_inset;
	}
	
	std::vector<VectorAllocator::Ptr>& remove() {
		return this->_remove;
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

	const LTS& _lts;

	VectorAllocator allocator_;

	std::vector<OLRTBlock*> _partition;
	BinaryRelation _relation;
	std::vector<StateListElem> _index;
	std::vector<std::pair<OLRTBlock*, size_t> > _queue;
	std::vector<SmartSet> _delta;
	std::vector<SmartSet> _delta1;
	std::vector<std::vector<size_t> > _key;
	std::vector<size_t> _range;

	OLRTAlgorithm(const OLRTAlgorithm&);

	OLRTAlgorithm& operator=(const OLRTAlgorithm&);

protected:

	VectorAllocator::Ptr getRemove(OLRTBlock& block, size_t label) {

		auto& remove = block.remove()[label];

		if (!remove) {

			this->_queue.push_back(std::make_pair(&block, label));

			remove = this->allocator_.get();
			remove->clear();

		}

		return remove;

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

			blockMask[block->index()] = true;;

			modifiedBlocks.push_back(block);

		}

	}

	template <class T>
	void fastSplit(const T& remove) {

		std::vector<OLRTBlock*> modifiedBlocks;

		this->internalSplit(modifiedBlocks, remove);

		for (auto& block : modifiedBlocks) {

			block->checkEmpty();

			if (!block->tmp())
				continue;

			size_t newIndex = this->_relation.split(block->index(), true);

			this->_partition.push_back(new OLRTBlock(this->_lts, *block, newIndex));

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

			assert(block->tmp());

			size_t newIndex = this->_relation.split(block->index(), true);

			OLRTBlock* newBlock = new OLRTBlock(this->_lts, *block, newIndex);

			this->_partition.push_back(newBlock);

			removeList.push_back(newBlock);

			for (auto& a : newBlock->inset()) {

				newBlock->counter().copyRow(a, block->counter());

				if (block->remove()[a]) {

					this->_queue.push_back(std::make_pair(newBlock, a));

					newBlock->remove()[a] = this->allocator_.get();
					*newBlock->remove()[a] = *block->remove()[a];

				}

			}

		}

	}

	void processRemove(OLRTBlock* block, size_t label) {

		assert(block);

		auto remove = block->remove()[label];

		assert(remove);

		block->remove()[label] = nullptr;

		std::vector<OLRTBlock*> preList, removeList;

		this->buildPre(preList, block->states(), label);

		this->split(removeList, *remove);

		this->allocator_.reclaim(remove);

		for (auto& b1 : preList) {

			for (auto& b2 : removeList) {

				assert(b1->index() != b2->index());

				if (!this->_relation.get(b1->index(), b2->index()))
					continue;

				this->_relation.set(b1->index(), b2->index(), false);

				for (auto a : b2->inset()) {

					if (!b1->inset().contains(a))
						continue;

					StateListElem* elem2 = b2->states();

					do {

						for (auto& pre2 : this->_lts.pre(a)[elem2->index_]) {

							if (!b1->counter().decr(a, pre2))
								this->getRemove(*b1, a)->push_back(pre2);

						}

						elem2 = elem2->next_;

					} while (elem2 != b2->states());

				}

			}

		}

	}

public:

	OLRTAlgorithm(const LTS& lts) : _lts(lts), allocator_(), _partition(), _relation(),
		_index(lts.states()), _queue(), _delta(), _delta1(), _key(), _range() /*_removeCache()*/ {

		assert(this->_index.size());

		OLRTBlock* block = new OLRTBlock(lts, 0, &this->_index[0], this->_key, this->_range);

		size_t q = this->_index.size();

		for (auto& state : this->_index) {

			state.index_ = q % this->_index.size();
			state.block_ = block;
			state.next_ = &this->_index[(q + 1)%this->_index.size()];
			state.prev_ = &this->_index[(q - 1)%this->_index.size()];

			++q;

		}			

		this->_partition.push_back(block);

	}

	~OLRTAlgorithm() {

		for (auto& block : this->_partition)
			delete block;

	}

	bool isConsistent() const {

		if (this->_partition.size() != this->_relation.size())
			return false;

		for (size_t i = 0; i < this->_partition.size(); ++i) {

			if (!this->_relation.get(i, i))
				return false;

		}

		return true;

	}

	void init(const BinaryRelation& relation) {

		this->_relation = relation;

		assert(this->isConsistent());

		this->_lts.buildDelta(this->_delta, this->_delta1);
		this->_key.resize(this->_lts.labels());
		this->_range.resize(this->_lts.labels());

		for (size_t a = 0; a < this->_lts.labels(); ++a) {

			this->_key[a].resize(this->_lts.states());
			this->_range[a] = this->_delta1[a].size();

			size_t x = 0;

			for (auto q : this->_delta1[a])
				this->_key[a][q] = x++;

		}

		for (size_t a = 0; a < this->_lts.labels(); ++a)

			this->fastSplit(this->_delta1[a]);

		std::vector<std::vector<size_t>> pre(this->_partition.size()), noPre(this->_lts.labels());

		for (auto& block : this->_partition) {

			StateListElem* elem = block->states();

			do {

				for (size_t a = 0; a < this->_lts.labels(); ++a) {

					this->_delta1[a].contains(elem->index_)
						? pre[block->index()].push_back(a)
						: noPre[a].push_back(block->index());

				}

				elem = elem->next_;

			} while (elem != block->states());

		}

		for (size_t b1 = 0; b1 < this->_partition.size(); ++b1) {

			for (auto& a : pre[b1]) {

				for (auto& b2 : noPre[a]) {

					assert(b1 != b2);

					this->_relation.set(b1, b2, false);

				}

			}

		}

		SmartSet s;

		for (std::vector<OLRTBlock*>::reverse_iterator i = this->_partition.rbegin(); i != this->_partition.rend(); ++i) {

			for (auto& a : (*i)->inset()) {

				for (auto q : this->_delta1[a]) {

					for (auto r : this->_lts.post(a)[q]) {

						if (this->_relation.get((*i)->index(), this->_index[r].block_->index()))
							(*i)->counter().incr(a, q);

					}

				}

				s.assignFlat(this->_delta1[a]);

				for (auto& b2 : this->_partition) {

					if (!this->_relation.get((*i)->index(), b2->index()))
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

				this->_queue.push_back(std::make_pair(*i, a));

				(*i)->remove()[a] = new std::vector<size_t>(s.begin(), s.end());

				assert(s.size() == (*i)->remove()[a]->size());

			}

		}

	}

	template <class T>
	bool makeBlock(const T& states, size_t blockIndex) {

		assert(states.size() > 0);

		OLRTBlock* block = this->_index[states.front()].block_;

		for (auto& q : states) {

			StateListElem& elem = this->_index[q];

			assert(block == elem.block_);
			assert(block->states());

			block->moveToTmp(elem);

		}

		block->checkEmpty();

		assert(block->tmp());

		this->_partition.push_back(new OLRTBlock(this->_lts, *block, blockIndex));

		return true;

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

			size_t ii = this->_index[i].block_->index();

			for (size_t j = 0; j < size; ++j)

				result.set(i, j, this->_relation.get(ii, this->_index[j].block_->index()));

		}

	}

	friend std::ostream& operator<<(std::ostream& os, const OLRTAlgorithm& alg) {

		for (auto& block : alg._partition)
	  		os << *block;

		return os << "relation:" << std::endl << alg._relation;

	}

};

void VATA::computeSimulation(BinaryRelation& result, size_t outputSize, const LTS& lts) {

	if (lts.states() == 0)
		return;

	OLRTAlgorithm alg(lts);

	alg.init(BinaryRelation(1, true));
	alg.run();
	alg.buildResult(result, outputSize);

}

void VATA::computeSimulation(BinaryRelation& result, size_t outputSize, const LTS& lts,
	const std::vector<std::vector<size_t>>& part, const std::vector<size_t>& finalStates) {

	if (lts.states() == 0)
		return;

	OLRTAlgorithm alg(lts);

	// accepting states to block 1
	alg.makeBlock(finalStates, 1);

	// environments to blocks 2, 3, ...
	for (size_t i = 0; i < part.size(); ++i)
		alg.makeBlock(part[i], i + 2);

	alg.init(result);
	alg.run();
	alg.buildResult(result, outputSize);

}
