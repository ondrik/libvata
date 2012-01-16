/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2012  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Source file for an explicit downward inclusion.
 *
 *****************************************************************************/

#include <set>
#include <algorithm>

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/util/cache.hh>
#include <vata/util/cached_binary_op.hh>
#include <vata/util/antichain1c.hh>
#include <vata/util/antichain2c_v2.hh>
#include <vata/util/caching_allocator.hh>

#include <vata/explicit_tree_incl_down.hh>

template <class T1, class T2>
bool checkIntersection(const T1& s1, const T2& s2) {

	auto i1 = s1.begin(), i2 = s2.begin();

	while (i1 != s1.end() && i2 != s2.end()) {

		if (*i1 < *i2) ++i1;
		else if (*i2 < *i1) ++i2;
		else return true;

	}

	return false;

}

template <class T1, class T2>
void intersectionByLookup(T1& d, const T2& s) {

	for (auto i = d.begin(); i != d.end(); ) {

		if (s.count(*i) == 0)
			i = d.erase(i);
		else
			++i;

	}

}

typedef VATA::Explicit::StateType SmallerType;
typedef std::vector<VATA::Explicit::StateType> StateSet;

typedef size_t SymbolType;

typedef typename VATA::Util::Cache<StateSet> BiggerTypeCache;

typedef typename BiggerTypeCache::TPtr BiggerType;

typedef typename VATA::Util::Antichain1C<SmallerType> Antichain1C;
typedef typename VATA::Util::Antichain2Cv2<SmallerType, BiggerType> Antichain2C;

typedef std::pair<SmallerType, BiggerType> SmallerBiggerPair;
typedef std::pair<SmallerType, Antichain2C::TList::iterator> SmallerBiggerPairAC;

typedef VATA::Explicit::StateTuple StateTuple;

typedef VATA::ExplicitDownwardInclusion::DoubleIndexedTupleList DoubleIndexedTupleList;

struct less {

	bool operator()(const SmallerBiggerPair& p1, const SmallerBiggerPair& p2) const {

		if (p1.second->size() < p2.second->size()) return true;
		if (p1.second->size() > p2.second->size()) return false;
		if (p1.first < p2.first) return true;
		if (p1.first > p2.first) return false;
		return p1.second.get() < p2.second.get();

	}

};

typedef std::set<SmallerBiggerPair, less> OrderedType;

class ChoiceFunction {

	std::vector<size_t> data_;
	size_t arity_;

public:

	ChoiceFunction() : data_(), arity_() {}

	void init(size_t size, size_t arity) {

		this->data_ = std::vector<size_t>(size, 0);
		this->arity_ = arity;

	}

	bool next() {

		// move to the next choice function
		size_t index = 0;

		while (++this->data_[index] == this->arity_) {

			this->data_[index] = 0; // reset this counter

			++index;                // move to the next counter

			if (index == this->data_.size()) {

				// if we drop out from the n-tuple
				return false;

			}

		}

		return true;

	}

	const size_t& operator[](size_t index) {

		assert(index < this->data_.size());

		return this->data_[index];

	}

	size_t size() const {

		return this->data_.size();

	}

	const size_t& arity() const {

		return this->arity_;

	}

};

struct ExpandStackFrame {

	ExpandStackFrame* parent;
	size_t retAddr;
	SmallerType p_S;
	BiggerType P_B;
	size_t a;
	std::vector<const StateTuple*> W;
	std::vector<const StateTuple*>::const_iterator tupleSetIter;
	size_t i;
	std::vector<size_t>::const_iterator sIter;
	ChoiceFunction choiceFunction;
	Antichain2C::TList::iterator worksetIter;
	Antichain2C childrenCache;

	ExpandStackFrame() : parent(), retAddr(), p_S(), P_B(), a(), W(), tupleSetIter(), i(), sIter(),
		choiceFunction(), worksetIter(), childrenCache() {}

private:

	ExpandStackFrame(const ExpandStackFrame&);
	ExpandStackFrame& operator=(const ExpandStackFrame&);

};

class ExpandCallEmulator {

	VATA::Util::CachingAllocator<ExpandStackFrame> allocator_;
	Antichain2C workset_;
	ExpandStackFrame*& framePtr_;

public:

	ExpandCallEmulator(ExpandStackFrame*& framePtr) : allocator_(), workset_(), framePtr_(framePtr) {

		this->framePtr_ = nullptr;

	}

	void call(size_t retAddr, const SmallerType& p_S, const BiggerType& P_B) {

		ExpandStackFrame* newFrame = this->allocator_();

		newFrame->parent = this->framePtr_;
		newFrame->retAddr = retAddr;
		newFrame->p_S = p_S;
		newFrame->P_B = P_B;

		this->framePtr_ = newFrame;

	}

	void addToWorkset() {

		this->framePtr_->worksetIter = this->workset_.insert(
			this->framePtr_->p_S, this->framePtr_->P_B
		);

	}

	size_t retAddr() const {

		return this->framePtr_->retAddr;

	}

	void ret1() {

		this->allocator_.reclaim(this->framePtr_);
		this->framePtr_ = this->framePtr_->parent;

	}

	void ret2() {

		this->workset_.remove(this->framePtr_->p_S, this->framePtr_->worksetIter);
		this->allocator_.reclaim(this->framePtr_);
		this->framePtr_ = this->framePtr_->parent;

	}

	const Antichain2C& workset() const {

		return this->workset_;

	}

};

#define EXPAND_RETURN(x)\
	switch (callEmulator.retAddr()) {\
		case 0: callEmulator.ret##x(); goto _end;\
		case 1: r_i = frame->p_S; S = frame->P_B; callEmulator.ret##x(); goto _stdret;\
		case 2: callEmulator.ret##x(); goto _simret;\
	}

inline bool expand(BiggerTypeCache& biggerTypeCache,
	VATA::Util::CachedBinaryOp<const StateSet*, const StateSet*, bool>& lteCache,
	Antichain2C& nonincluded,
	const SmallerType& p_S, const BiggerType& P_B,
	const DoubleIndexedTupleList& smallerIndex, const DoubleIndexedTupleList& biggerIndex,
	const std::vector<std::vector<size_t>>& ind, const std::vector<std::vector<size_t>>& inv
) {

	auto noncachedLte = [&ind](const StateSet* x, const StateSet* y) -> bool {

		assert(x); assert(y);

		for (auto& s1 : *x) {

			assert(s1 < ind.size());

			if (!checkIntersection(ind[s1], *y))
				return false;

		}

		return true;

	};

	auto lte = [&noncachedLte, &lteCache](const BiggerType& x, const BiggerType& y) -> bool {

		assert(x); assert(y);

		return (x.get() == y.get())?(true):(lteCache.lookup(x.get(), y.get(), noncachedLte));

	};

	auto gte = [&lte](const BiggerType& x, const BiggerType& y) { return lte(y, x); };

	ExpandStackFrame* frame;

	ExpandCallEmulator callEmulator(frame);

	const std::vector<const StateTuple*>* smallerTupleSet;

	std::unordered_set<const StateTuple*> tupleSet;

	Antichain1C post;

	StateSet tmp;

	SmallerType r_i;

	BiggerType S;

	bool found; // return value of simulated calls

	callEmulator.call(0, p_S, P_B);
_call:
	if (smallerIndex.size() <= frame->p_S) {

		found = true;

		EXPAND_RETURN(1)

	}

	if (checkIntersection(ind[frame->p_S], *frame->P_B)) {

		found = true;

		EXPAND_RETURN(1)

	}

	assert(frame->p_S < ind.size());

	if (callEmulator.workset().contains(ind[frame->p_S], frame->P_B, lte)) {

		found = true;

		EXPAND_RETURN(1)

	}

	assert(frame->p_S < inv.size());

	if (nonincluded.contains(inv[frame->p_S], frame->P_B, gte)) {

		found = false;

		EXPAND_RETURN(1)

	}

	if (frame->P_B->size() > 1) {

		for (frame->sIter = frame->P_B->begin(); frame->sIter != frame->P_B->end(); ++frame->sIter) {

			callEmulator.call(2, frame->p_S, biggerTypeCache.lookup({ *frame->sIter }));

			goto _call;
_simret:
			if (found) {

				EXPAND_RETURN(2)

			}

		}

	}

	callEmulator.addToWorkset();

	assert(frame->p_S < smallerIndex.size());

	frame->childrenCache.clear();

	for (frame->a = 0; frame->a < smallerIndex[frame->p_S].size(); ++frame->a) {

		smallerTupleSet = &smallerIndex[frame->p_S][frame->a];

		if (smallerTupleSet->empty())
			continue;

		if (/* arity */ smallerTupleSet->front()->size() == 0) {

			StateSet::const_iterator i;

			for (i = frame->P_B->begin(); i != frame->P_B->end(); ++i) {

				if (*i < biggerIndex.size() && biggerIndex[*i].size())
					break;

			}

			if (i == frame->P_B->end()) {

				found = false;

				EXPAND_RETURN(2)

			}

			continue;

		}

		frame->W.clear();

		tupleSet.clear();

		for (auto& state : *frame->P_B) {

			if (biggerIndex.size() <= state)
				continue;

			auto& biggerCluster = biggerIndex[state];

			if (biggerCluster.empty())
				continue;

			if (biggerCluster.size() <= frame->a)
				continue;

			for (auto& tuple : biggerCluster[frame->a]) {

				if (tupleSet.insert(tuple).second)
					frame->W.push_back(tuple);

			}

		}

		if (frame->W.empty()) {

			found = false;

			EXPAND_RETURN(2)

		}

		for (frame->tupleSetIter = smallerTupleSet->begin(); frame->tupleSetIter != smallerTupleSet->end(); ++frame->tupleSetIter) {

			frame->choiceFunction.init(frame->W.size(), /* arity */ smallerTupleSet->front()->size());

			do {
				// we loop for each choice function
				found = false;

				for (frame->i = 0; frame->i < frame->choiceFunction.arity(); ++frame->i) {
					// for each position of the n-tuple
					post.clear();

					for (size_t j = 0; j < frame->choiceFunction.size(); ++j) {

						if (frame->choiceFunction[j] != frame->i)
							continue;

						// in case the choice function for given vector is i
						assert((*frame->W[j])[frame->i] < ind.size());

						if (post.contains(ind[(*frame->W[j])[frame->i]]))
							continue;

						assert((*frame->W[j])[frame->i] < inv.size());

						post.refine(inv[(*frame->W[j])[frame->i]]);
						post.insert((*frame->W[j])[frame->i]);

					}

					if (post.data().empty())
						continue;

					tmp = StateSet(post.data().begin(), post.data().end());

					std::sort(tmp.begin(), tmp.end());

					assert((*frame->tupleSetIter)->size() == frame->choiceFunction.arity());

					r_i = (**frame->tupleSetIter)[frame->i];

					S = biggerTypeCache.lookup(tmp);

					if (frame->childrenCache.contains(ind[r_i], S, lte))
						goto _nextchoice;

					callEmulator.call(1, r_i, S);

					goto _call;
_stdret:
					if (found) {

						frame->childrenCache.refine(inv[r_i], S, gte);
						frame->childrenCache.insert(r_i, S);

						goto _nextchoice;

					}

					if (!nonincluded.contains(inv[r_i], S, gte)) {

						nonincluded.refine(ind[r_i], S, lte);
						nonincluded.insert(r_i, S);

					}

				}

				EXPAND_RETURN(2)
_nextchoice:;
			} while (frame->choiceFunction.next());

			assert(frame->p_S < smallerIndex.size());
			assert(frame->a < smallerIndex[frame->p_S].size());

			smallerTupleSet = &smallerIndex[frame->p_S][frame->a];

		}

	}

	found = true;

	EXPAND_RETURN(2)
_end:
	assert(frame == nullptr);

	return found;

}

bool VATA::ExplicitDownwardInclusion::checkInternal(
	const DoubleIndexedTupleList& smallerIndex, const Explicit::StateSet& smallerFinalStates,
	const DoubleIndexedTupleList& biggerIndex, const Explicit::StateSet& biggerFinalStates,
	const std::vector<std::vector<size_t>>& ind, const std::vector<std::vector<size_t>>& inv
) {

	Util::CachedBinaryOp<const StateSet*, const StateSet*, bool> lteCache;

	BiggerTypeCache biggerTypeCache(
		[&lteCache](const StateSet* v) {
			lteCache.invalidateFirst(v);
			lteCache.invalidateSecond(v);
		}
	);

	Antichain2C nonincluded;

	std::vector<size_t> v(biggerFinalStates.begin(), biggerFinalStates.end());

	std::sort(v.begin(), v.end());

	auto biggerF = biggerTypeCache.lookup(v);

	for (auto& f : smallerFinalStates) {

		if (!expand(biggerTypeCache, lteCache, nonincluded, f, biggerF, smallerIndex, biggerIndex, ind, inv))
			return false;

	}

	return true;

}
