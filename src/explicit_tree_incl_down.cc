/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
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

struct ExpandStackFrame {

	SmallerType p_S;
	BiggerType P_B;
	size_t a;
	std::vector<const StateTuple*> W;
	std::vector<const StateTuple*>::const_iterator tupleSetIter;
	size_t i;
	std::vector<size_t> choiceFunction;
	Antichain2C::TList::iterator worksetIter;

	ExpandStackFrame(const SmallerType& p_S, const BiggerType& P_B, const Antichain2C::TList::iterator& worksetIter)
		: p_S(p_S), P_B(P_B), a(), W(), tupleSetIter(), i(), choiceFunction(), worksetIter(worksetIter) {}

};

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

	if ((smallerIndex.size() <= p_S) || checkIntersection(ind[p_S], *P_B))
		return true;

	if (nonincluded.contains(inv[p_S], P_B, gte) || P_B->empty())
		return false;

	std::vector<ExpandStackFrame> stack;

	Antichain2C workset;

	stack.push_back(ExpandStackFrame(p_S, P_B, workset.insert(p_S, P_B)));

	ExpandStackFrame* frame = &stack.back();

	const std::vector<const StateTuple*>* smallerTupleSet;

	std::unordered_set<const StateTuple*> tupleSet;

	std::vector<size_t> v;

	size_t r_i;
	BiggerType S;

	bool found; // return value of simulated calls
_call:
	assert(frame->p_S < smallerIndex.size());

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

				if (stack.size() == 1) goto _end; else goto _ret;

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

			if (stack.size() == 1) goto _end; else goto _ret;

		}

		for (frame->tupleSetIter = smallerTupleSet->begin(); frame->tupleSetIter != smallerTupleSet->end(); ++frame->tupleSetIter) {

			frame->choiceFunction = std::vector<size_t>(frame->W.size(), 0);

			while (1) {
				// we loop for each choice function
				found = false;

				for (frame->i = 0; frame->i < /* arity */ smallerTupleSet->front()->size(); ++frame->i) {
					// for each position of the n-tuple

					v.clear();

					for (size_t j = 0; j < frame->choiceFunction.size(); ++j) {

						if (frame->choiceFunction[j] == frame->i) {
							// in case the choice function for given vector is i
							v.push_back((*frame->W[j])[frame->i]);
						}

					}

					if (v.empty())
						continue;

					std::sort(v.begin(), v.end());

					v.erase(std::unique(v.begin(), v.end()), v.end());

					assert((*frame->tupleSetIter)->size() == smallerTupleSet->front()->size());

					r_i = (**frame->tupleSetIter)[frame->i];
					S = biggerTypeCache.lookup(v);

					if (checkIntersection(ind[r_i], *S)) {
						found = true;
						break;
					}

					if (workset.contains(ind[r_i], S, lte)) {
						found = true;
						break;
					}

					if (nonincluded.contains(inv[r_i], S, gte))
						continue;

					stack.push_back(ExpandStackFrame(r_i, S, workset.insert(r_i, S)));

					frame = &stack.back();

					goto _call;
_ret:
					r_i = frame->p_S;
					S = frame->P_B;

					workset.remove(r_i, frame->worksetIter);

					stack.pop_back();

					frame = &stack.back();

					if (found)
						break;

					if (!nonincluded.contains(inv[r_i], S, gte)) {

						nonincluded.refine(ind[r_i], S, lte);
						nonincluded.insert(r_i, S);

					}

				}

				if (!found) {

					if (stack.size() == 1) goto _end; else goto _ret;

				}

				smallerTupleSet = &smallerIndex[frame->p_S][frame->a];

				// move to the next choice function
				size_t index = 0;

				while (++frame->choiceFunction[index] == /* arity */ smallerTupleSet->front()->size()) {

					frame->choiceFunction[index] = 0; // reset this counter

					++index;                   // move to the next counter

					if (index == frame->choiceFunction.size()) {
						// if we drop out from the n-tuple
						goto _cf_end;
					}

				}

			}
_cf_end:;
		}

	}

	found = true;

	if (stack.size() > 1) goto _ret;
_end:
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
