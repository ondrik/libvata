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

#include <vata/explicit_lts_sim.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/smart_set.hh>
#include <vata/util/lts.hh>

using VATA::Util::BinaryRelation;
using VATA::Util::SmartSet;
using VATA::Util::LTS;

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
//		this->_key = rhs._key;
//		this->_range = rhs._range;

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
/*
	const std::vector<size_t>& getRow(size_t label) const {

		assert(label < this->_data.size());

		return this->_data[label]; 

	}
*/
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

class OLRTBlock;

class StateListElem {
	
	size_t _state;
	OLRTBlock* _block;
	StateListElem* _next;
	StateListElem* _prev;

protected:

	void insert(StateListElem*& dst) {
		if (!dst) {
			dst = this;
			this->_next = this;
			this->_prev = this;
		} else {
			this->_prev = dst->_prev;
			this->_next = dst;
			this->_prev->_next = this;
			this->_next->_prev = this;
		}	
	}

	StateListElem(const StateListElem&);

	StateListElem& operator=(const StateListElem&);

public:

	StateListElem(size_t state, OLRTBlock* block)
		: _state(state), _block(block), _next(this), _prev(this) {}

	StateListElem(size_t state, OLRTBlock* block, StateListElem*& dst)
		: _state(state), _block(block), _next(nullptr), _prev(nullptr) {
		this->insert(dst);
	}
	
	~StateListElem() {
		if (this != this->_next) {
			this->_next->_prev = this->_prev;
			this->_prev->_next = this->_next;
		}
	}

	void moveToList(StateListElem*& src, StateListElem*& dst) {
		assert(src);
		if (this == this->_next)
			src = NULL;
		else {
			src = this->_next;
			this->_next->_prev = this->_prev;
			this->_prev->_next = this->_next;
		}
		this->insert(dst);
	}
	
	size_t state() const {
		return this->_state;
	}
	
	OLRTBlock* block() const {
/*		if (!this->_block)
			throw std::runtime_error("StateListElem::block() : block empty");*/
		return this->_block;
	}
	
	void block(OLRTBlock* b) {
		this->_block = b;
	}
	
	StateListElem* next() const {
		return this->_next;
	}
	
};

class OLRTBlock {

	size_t _index;
	StateListElem* _states;
	std::vector<std::vector<size_t>*> _remove;
	Counter _counter;
	OLRTBlock* _intersection;
	SmartSet _inset;
	StateListElem* _tmp;

	OLRTBlock(const OLRTBlock&);

	OLRTBlock& operator=(const OLRTBlock&);

public:

	OLRTBlock(size_t index, const LTS& lts, const std::vector<std::vector<size_t> >& key, const std::vector<size_t>& range)
		: _index(index), _states(NULL), _remove(lts.labels()), _counter(lts.labels(), key, range), _intersection(NULL), _inset(lts.labels()), _tmp(NULL) {
		for (size_t i = 0; i < lts.states(); ++i) {
			new StateListElem(i, this, this->_states);
			for (SmartSet::iterator j = lts.lPre()[i].begin(); j != lts.lPre()[i].end(); ++j)
				this->_inset.add(*j);
		}
	}
	
	OLRTBlock(size_t index, OLRTBlock* parent, const LTS& lts)
		: _index(index), _states(parent->_tmp), _remove(lts.labels(), NULL), _counter(parent->_counter), _intersection(NULL), _inset(lts.labels()), _tmp(NULL) {
		parent->_tmp = NULL;
		parent->_intersection = this;
		StateListElem* elem = this->_states;
		do {
			for (SmartSet::iterator i = lts.lPre()[elem->state()].begin(); i != lts.lPre()[elem->state()].end(); ++i) {
				parent->_inset.remove(*i);
				this->_inset.add(*i);
			}
			elem->block(this);
			elem = elem->next();
		} while (elem != this->_states);
	}
	
	~OLRTBlock() {
		while (this->_states != this->_states->next())
			delete this->_states->next();
		delete this->_states;
	}

	StateListElem* states() {
		return this->_states;
	}
	
	StateListElem* tmp() {
		return this->_tmp;
	}

	void storeStates(std::vector<StateListElem*>& v) const {
		v.clear();
		StateListElem* elem = this->_states;
		do {
			v.push_back(elem);
			elem = elem->next();
		} while (elem != this->_states);
	}
	
	void moveToTmp(StateListElem* elem) {
		elem->moveToList(this->_states, this->_tmp);	
	}
	
	bool checkEmpty() {
		if (this->_states != NULL)
			return false;
		this->_states = this->_tmp;
		this->_tmp = NULL;
		return true;
	}
	
	Counter& counter() {
		return this->_counter;
	}
	
	OLRTBlock* intersection() const {
		return this->_intersection;
	}

	void intersection(OLRTBlock* b) {
		this->_intersection = b;
	}
	
	SmartSet& inset() {
		return this->_inset;
	}
	
	std::vector<std::vector<size_t>*>& remove() {
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

			os << " " << elem->state();

			elem = elem->next();

		} while (elem != block._states);

		return os << " )";

	}
	
};

class OLRTAlgorithm {

	const LTS& _lts;
	std::vector<OLRTBlock*> _partition;
	BinaryRelation _relation;
	std::vector<StateListElem*> _index;
	std::vector<std::pair<OLRTBlock*, size_t> > _queue;
	std::vector<bool> _tmp;
	std::vector<SmartSet> _delta;
	std::vector<SmartSet> _delta1;
	std::vector<std::vector<size_t> > _key;
	std::vector<size_t> _range;
	
	std::vector<std::vector<size_t>*> _removeCache;
	
	std::vector<size_t>* rcAlloc() {

		if (this->_removeCache.empty())
			return new std::vector<size_t>;

		std::vector<size_t>* v = this->_removeCache.back();

		this->_removeCache.pop_back();

		return v->clear(), v;

	}
	
	void rcFree(std::vector<size_t>* v) {
		this->_removeCache.push_back(v);
	}

	void rcCollect() {
		for (std::vector<std::vector<size_t>*>::iterator i = this->_removeCache.begin(); i != this->_removeCache.end(); ++i)
			delete *i;
	}

	OLRTAlgorithm(const OLRTAlgorithm&);

	OLRTAlgorithm& operator=(const OLRTAlgorithm&);

protected:

	template <class T>
	void fastSplit(const T& remove) {

		std::vector<OLRTBlock*> splitList;

		for (auto q : remove) {
			StateListElem* elem = this->_index[q];
			elem->block()->moveToTmp(elem);
			elem->block()->checkEmpty();
		}

		for (auto q : remove) {
			StateListElem* elem = this->_index[q];
			if (elem->block()->tmp() != NULL) {
				splitList.push_back(elem->block());
				OLRTBlock* block = new OLRTBlock(this->_relation.newEntry(true), elem->block(), this->_lts);
				this->_partition.push_back(block);
			}
		}

		for (std::vector<OLRTBlock*>::reverse_iterator i = splitList.rbegin(); i != splitList.rend(); ++i) {
			OLRTBlock* bint = (*i)->intersection();
			(*i)->intersection(NULL);
//			for (std::vector<OLRTBlock*>::iterator j = this->_partition.begin(); j != this->_partition.end(); ++j) {
			for (std::vector<OLRTBlock*>::reverse_iterator j = this->_partition.rbegin(); j != this->_partition.rend(); ++j) {
				this->_relation.set((*j)->index(), bint->index(), this->_relation.get((*j)->index(), (*i)->index()));
				this->_relation.set(bint->index(), (*j)->index(), this->_relation.get((*i)->index(), (*j)->index()));
			}
			this->_relation.set(bint->index(), bint->index(), true);
		}

	}

	void split(const std::vector<size_t>& remove, std::vector<OLRTBlock*>& removeList) {
		removeList.clear();
		std::vector<OLRTBlock*> splitList;
//		for (std::vector<int>::const_iterator i = remove.begin(); i != remove.end(); ++i) {
		for (std::vector<size_t>::const_reverse_iterator i = remove.rbegin(); i != remove.rend(); ++i) {
			StateListElem* elem = this->_index[*i];
			elem->block()->moveToTmp(elem);
			if (elem->block()->checkEmpty())
				removeList.push_back(elem->block());
		}
//		for (std::vector<int>::const_iterator i = remove.begin(); i != remove.end(); ++i) {
		for (std::vector<size_t>::const_reverse_iterator i = remove.rbegin(); i != remove.rend(); ++i) {
			StateListElem* elem = this->_index[*i];
			if (elem->block()->tmp() != NULL) {
				splitList.push_back(elem->block());
				OLRTBlock* block = new OLRTBlock(this->_relation.newEntry(true), elem->block(), this->_lts);
				this->_partition.push_back(block);
				removeList.push_back(block);
			}
		}
//		for (std::vector<OLRTBlock*>::iterator i = splitList.begin(); i != splitList.end(); ++i) {
		for (std::vector<OLRTBlock*>::reverse_iterator i = splitList.rbegin(); i != splitList.rend(); ++i) {
			OLRTBlock* bint = (*i)->intersection();
			(*i)->intersection(NULL);
//			for (std::vector<OLRTBlock*>::iterator j = this->_partition.begin(); j != this->_partition.end(); ++j) {
			for (std::vector<OLRTBlock*>::reverse_iterator j = this->_partition.rbegin(); j != this->_partition.rend(); ++j) {
				this->_relation.set((*j)->index(), bint->index(), this->_relation.get((*j)->index(), (*i)->index()));
				this->_relation.set(bint->index(), (*j)->index(), this->_relation.get((*i)->index(), (*j)->index()));
			}
			this->_relation.set(bint->index(), bint->index(), true);
			for (SmartSet::iterator j = bint->inset().begin(); j != bint->inset().end(); ++j) {
				bint->counter().copyRow(*j, (*i)->counter());
				if ((*i)->remove()[*j]) {
					bint->remove()[*j] = this->rcAlloc();
					*bint->remove()[*j] = *(*i)->remove()[*j];
					this->_queue.push_back(std::pair<OLRTBlock*, size_t>(bint, *j));
				}
			}
		}
	}

	void processRemove(OLRTBlock* block, size_t label) {

		std::vector<size_t>* remove = block->remove()[label];

		assert(remove);

		block->remove()[label] = NULL;

		std::vector<StateListElem*> prev;

		block->storeStates(prev);

		std::vector<OLRTBlock*> removeList;

		this->split(*remove, removeList);

		std::fill(this->_tmp.begin(), this->_tmp.end(), true);

		for (std::vector<StateListElem*>::iterator i = prev.begin(); i != prev.end(); ++i) {

			for (auto& pre : this->_lts.pre(label)[(*i)->state()]) {

				StateListElem* elem = this->_index[pre];

				OLRTBlock* b1 = elem->block();

				if (!this->_tmp[b1->index()])
					continue;

				this->_tmp[b1->index()] = false;

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

							for (auto& pre2 : this->_lts.pre(a)[elem2->state()]) {

								if (b1->counter().decr(a, pre2))
									continue;

								if (!b1->remove()[a]) {

									b1->remove()[a] = this->rcAlloc();

									this->_queue.push_back(std::make_pair(b1, a));

								}

								b1->remove()[a]->push_back(pre2);

							}

							elem2 = elem2->next();

						} while (elem2 != b2->states());

					}

				}

			}

		}

		this->rcFree(remove);

	}

public:

	OLRTAlgorithm(const LTS& lts, const BinaryRelation& rel) : _lts(lts), _partition(),
		_relation(rel), _index(), _queue(), _tmp(lts.states()), _delta(), _delta1(), _key(),
		_range(), _removeCache() {

		assert(lts.states());

		OLRTBlock* block = new OLRTBlock(0, lts, this->_key, this->_range);

		block->storeStates(this->_index);

		this->_partition.push_back(block);

	}

	~OLRTAlgorithm() {

		for (auto& block : this->_partition)
			delete block;

		this->rcCollect();

	}

	bool isConsistent() const {

		if (this->_partition.size() != this->_relation.size())
			return false;

		for (auto& block : this->_partition) {

			if (!this->_relation.get(block->index(), block->index()))
				return false;

		}

		return true;

	}

	void init() {

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

					this->_delta1[a].contains(elem->state())
						? pre[block->index()].push_back(a)
						: noPre[a].push_back(block->index());

				}

				elem = elem->next();

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

//		for (std::vector<OLRTBlock*>::iterator i = this->_partition.begin(); i != this->_partition.end(); ++i) {
		for (std::vector<OLRTBlock*>::reverse_iterator i = this->_partition.rbegin(); i != this->_partition.rend(); ++i) {

			for (auto a : (*i)->inset()) {

				for (auto q : this->_delta1[a]) {

					for (auto r : this->_lts.post(a)[q]) {

						if (this->_relation.get((*i)->index(), this->_index[r]->block()->index()))
							(*i)->counter().incr(a, q);

					}

				}

				s.assignFlat(this->_delta1[a]);

				for (auto& b2 : this->_partition) {

					if (!this->_relation.get((*i)->index(), b2->index()))
						continue;

					StateListElem* elem = b2->states();

					do {

						for (auto& q : this->_lts.pre(a)[elem->state()])
							s.remove(q);

						elem = elem->next();

					} while (elem != b2->states());

				}

				if (s.empty())
					continue;

				(*i)->remove()[a] = new std::vector<size_t>(s.begin(), s.end());

				assert(s.size() == (*i)->remove()[a]->size());

				this->_queue.push_back(std::make_pair(*i, a));

			}

		}

	}

	bool fakeSplit(const std::vector<size_t>& remove, size_t blockIndex) {

		assert(remove.size() > 0);

		OLRTBlock* block = this->_index[remove.front()]->block();

		for (std::vector<size_t>::const_iterator i = remove.begin(); i != remove.end(); ++i) {

			StateListElem* elem = this->_index[*i];

			assert(block == elem->block());
			assert(block->states());

			block->moveToTmp(elem);

		}

		block->checkEmpty();

		assert(block->tmp());

		this->_partition.push_back(new OLRTBlock(blockIndex, block, this->_lts));

		return true;

	}

	void run() {

	    while (!this->_queue.empty()) {

			std::pair<OLRTBlock*, size_t> tmp(this->_queue.back());

			this->_queue.pop_back();

			this->processRemove(tmp.first, tmp.second);

		}

	}
	
//	BinaryRelation& relation() { return this->_relation; }

	void buildRel(BinaryRelation& rel, size_t size) const {

		rel.resize(size);

		for (size_t i = 0; i < size; ++i) {

			size_t ii = this->_index[i]->block()->index();

			for (size_t j = 0; j < size; ++j)

				rel.set(i, j, this->_relation.get(ii, this->_index[j]->block()->index()));

		}

	}

//	size_t blockCount() const { return this->_partition.size(); }
/*
	size_t buildIndex(std::vector<size_t>& index, size_t size) const {
		size_t blockCount = 0;
		std::vector<bool> tmp(this->_partition.size(), false);
		index.resize(size);
		for (size_t i = 0; i < size; ++i) {
			index[i] = this->_index[i]->block()->index();
			if (!tmp[index[i]]) {
				tmp[index[i]] = true;
				++blockCount;
			}
		}
		return blockCount;
	}
*/
	friend std::ostream& operator<<(std::ostream& os, const OLRTAlgorithm& alg) {

		for (auto& block : alg._partition)
	  		os << *block;

		return os << "relation:" << std::endl << alg._relation;

	}

};

void VATA::computeSimulation(BinaryRelation& result, size_t outputSize, const LTS& lts) {

	if (lts.states() == 0)
		return;

	result.resize(0);
	result.newEntry(true);
	result.set(0, 0, true);

	OLRTAlgorithm alg(lts, result);

	alg.init();
	alg.run();
	alg.buildRel(result, outputSize);

}

void VATA::computeSimulation(BinaryRelation& result, size_t outputSize, const LTS& lts,
	const std::vector<std::vector<size_t>>& part, const std::vector<size_t>& finalStates) {

	if (lts.states() == 0)
		return;

	OLRTAlgorithm alg(lts, result);

	// accepting states to block 1
	alg.fakeSplit(finalStates, 1);

	// environments to blocks 2, 3, ...
	for (size_t i = 0; i < part.size(); ++i)
		alg.fakeSplit(part[i], i + 2);

	alg.init();
	alg.run();
	alg.buildRel(result, outputSize);

}
