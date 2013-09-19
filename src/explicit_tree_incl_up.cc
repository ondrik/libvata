/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Source file for an explicit upward inclusion.
 *
 *****************************************************************************/

// Standard library headers
#include <set>
#include <algorithm>

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain1c.hh>
#include <vata/util/antichain2c_v2.hh>

#include "explicit_tree_aut_core.hh"
#include "explicit_tree_incl_up.hh"
#include "util/cache.hh"
#include "util/cached_binary_op.hh"


typedef VATA::ExplicitTreeAutCore::StateType SmallerType;
typedef std::vector<VATA::ExplicitTreeAutCore::StateType> StateSet;

typedef size_t SymbolType;

typedef typename VATA::Util::Cache<StateSet> BiggerTypeCache;

typedef typename BiggerTypeCache::TPtr BiggerType;

typedef typename VATA::Util::Antichain1C<SmallerType> Antichain1C;
typedef typename VATA::Util::Antichain2Cv2<SmallerType, BiggerType> Antichain2C;

typedef std::pair<SmallerType, Antichain2C::TList::iterator> SmallerBiggerPair;


namespace
{	// anonymous namespace

template <class T1, class T2>
bool checkIntersection(const T1& s1, const T2& s2)
{
	auto i1 = s1.begin(), i2 = s2.begin();

	while (i1 != s1.end() && i2 != s2.end())
	{
		if (*i1 < *i2) ++i1;
		else if (*i2 < *i1) ++i2;
		else return true;
	}

	return false;
}


template <class T1, class T2>
void intersectionByLookup(T1& d, const T2& s)
{
	for (auto i = d.begin(); i != d.end(); )
	{
		if (s.count(*i) == 0)
			i = d.erase(i);
		else
			++i;
	}
}


struct less
{
	bool operator()(
		const SmallerBiggerPair&       p1,
		const SmallerBiggerPair&       p2) const
	{
		if ((*p1.second)->size() < (*p2.second)->size()) return true;
		if ((*p1.second)->size() > (*p2.second)->size()) return false;
		if (p1.first < p2.first) return true;
		if (p1.first > p2.first) return false;
		return (*p1.second).get() < (*p2.second).get();
	}
};

typedef std::set<SmallerBiggerPair, less> OrderedType;

struct Eraser
{
	OrderedType& data_;

	Eraser(OrderedType& data) : data_(data) {}

	void operator()(
		const SmallerType&                               q,
		const typename Antichain2C::TList::iterator&     Q) const
	{
		this->data_.erase(std::make_pair(q, Q));
	}
};


GCC_DIAG_OFF(effc++)
struct Choice
{
GCC_DIAG_ON(effc++)

	const Antichain2C::TList* biggerList_;
	Antichain2C::TList::const_iterator current_;

	bool init(const Antichain2C::TList* biggerList)
	{
		if (!biggerList)
		{
			return false;
		}

		biggerList_ = biggerList;
		current_ = biggerList->begin();

		return true;
	}

	bool next()
	{
		if (++current_ != biggerList_->end())
		{
			return true;
		}

		current_ = biggerList_->begin();
		return false;
	}

	const BiggerType& get() const { return *this->current_; }
};

struct ChoiceVector
{
	const Antichain2C& processed_;
	const Antichain2C::TList& fixed_;
	std::vector<Choice> state_;

public:

	ChoiceVector(
		const Antichain2C&            processed,
		const Antichain2C::TList&     fixed) :
		processed_(processed),
		fixed_(fixed),
		state_()
	{ }

	bool build(
		const VATA::ExplicitTreeAutCore::StateTuple&    children,
		size_t                                          index)
	{
		assert(index < children.size());

		state_.resize(children.size());

		for (size_t i = 0; i < index; ++i)
		{
			if (!state_[i].init(processed_.lookup(children[i])))
			{
				return false;
			}
		}

		state_[index].biggerList_ = &fixed_;
		state_[index].current_ = fixed_.begin();

		for (size_t i = index + 1; i < children.size(); ++i)
		{
			if (!state_[i].init(processed_.lookup(children[i])))
			{
				return false;
			}
		}

		return true;
	}

	bool next()
	{
		for (auto& choice : state_)
		{
			if (choice.next())
			{
				return true;
			}
		}

		return false;
	}

	const BiggerType& operator()(size_t index) const
	{
		return state_[index].get();
	}

	size_t size() const
	{
		return state_.size();
	}
};
} // namespace


bool VATA::ExplicitUpwardInclusion::checkInternal(
	const SymbolToTransitionListMap&                  smallerLeaves,
	const IndexedSymbolToIndexedTransitionListMap&    smallerIndex,
	const ExplicitTreeAutCore::FinalStateSet&         smallerFinalStates,
	const SymbolToTransitionListMap&                  biggerLeaves,
	const SymbolToDoubleIndexedTransitionListMap&     biggerIndex,
	const ExplicitTreeAutCore::FinalStateSet&         biggerFinalStates,
	const std::vector<std::vector<size_t>>&           ind,
	const std::vector<std::vector<size_t>>&           inv)
{
	auto noncachedLte = [&ind](const StateSet* x, const StateSet* y) -> bool
	{
		assert(x); assert(y);

		for (auto& s1 : *x)
		{
			assert(s1 < ind.size());

			if (!checkIntersection(ind[s1], *y))
			{
				return false;
			}
		}

		return true;
	};

	Util::CachedBinaryOp<const StateSet*, const StateSet*, bool> lteCache;

	auto lte = [&noncachedLte, &lteCache](const BiggerType& x, const BiggerType& y) -> bool
	{
		assert(x); assert(y);

		return (x.get() == y.get())?(true):(lteCache.lookup(x.get(), y.get(), noncachedLte));
	};

	auto gte = [&lte](const BiggerType& x, const BiggerType& y) { return lte(y, x); };

	typedef VATA::ExplicitUpwardInclusion::Transition Transition;
	typedef std::unordered_set<const Transition*> TransitionSet;
	typedef typename std::shared_ptr<TransitionSet> TransitionSetPtr;

	auto noncachedEvalTransitions = [&biggerIndex](const std::pair<SymbolType, size_t>& key,
		const StateSet* states) -> TransitionSetPtr
	{
		assert(states);

		TransitionSetPtr result = TransitionSetPtr(new TransitionSet());

		if (biggerIndex.size() <= key.first)
		{
			return result;
		}

		auto& iter = biggerIndex[key.first];

		if (iter.size() <= key.second)
		{
			return result;
		}

		auto& indexedTransitionList = iter[key.second];

		for (auto& state: *states)
		{
			if (state >= indexedTransitionList.size())
			{
				continue;
			}

			for (auto& transition : indexedTransitionList[state])
			{
				result->insert(transition.get());
			}
		}

		return result;
	};

	Util::CachedBinaryOp<
		std::pair<SymbolType, size_t>, const StateSet*, TransitionSetPtr
	> evalTransitionsCache;

	auto evalTransitions = [&noncachedEvalTransitions, &evalTransitionsCache](
		const SymbolType& symbol, size_t i, const StateSet* states)
		-> TransitionSetPtr
		{
		assert(states);

		return evalTransitionsCache.lookup(
			std::make_pair(symbol, i), states, noncachedEvalTransitions
		);
	};

	BiggerTypeCache biggerTypeCache(
		[&lteCache, &evalTransitionsCache](const StateSet* v)
		{
			lteCache.invalidateFirst(v);
			lteCache.invalidateSecond(v);
			evalTransitionsCache.invalidateSecond(v);
		}
	);

	Antichain1C post;

	Antichain2C temporary, processed;

	OrderedType next;

	bool isAccepting;

	// Post(\emptyset)

	if (biggerLeaves.size() < smallerLeaves.size())
	{
		return false;
	}

	for (size_t symbol = 0; symbol < smallerLeaves.size(); ++symbol)
	{
		post.clear();
		isAccepting = false;

		for (auto& transition : biggerLeaves[symbol])
		{
			assert(transition);
			assert(transition->children().empty());
			assert(transition->state() < ind.size());

			if (post.contains(ind[transition->state()]))
			{
				continue;
			}

			assert(transition->state() < inv.size());

			post.refine(inv[transition->state()]);
			post.insert(transition->state());

			isAccepting = isAccepting || biggerFinalStates.count(transition->state());
		}

		StateSet tmp(post.data().begin(), post.data().end());

		std::sort(tmp.begin(), tmp.end());

		auto ptr = biggerTypeCache.lookup(tmp);

		for (auto& transition : smallerLeaves[symbol])
		{
			assert(transition);

			if (!isAccepting && smallerFinalStates.count(transition->state()))
			{
				return false;
			}

			assert(transition->state() < ind.size());

			if (checkIntersection(ind[transition->state()], tmp))
			{
				continue;
			}

			if (processed.contains(ind[transition->state()], ptr, lte))
			{
				continue;
			}

			assert(transition->state() < inv.size());

			processed.refine(inv[transition->state()], ptr, gte, Eraser(next));

			Antichain2C::TList::iterator iter = processed.insert(transition->state(), ptr);

			next.insert(std::make_pair(transition->state(), iter));
		}
	}

	SmallerType q;

	Antichain2C::TList fixedList(1);

	BiggerType& Q = fixedList.front();

	ChoiceVector choiceVector(processed, fixedList);

	while (!next.empty())
	{
		q = next.begin()->first;
		Q = *next.begin()->second;

		next.erase(next.begin());

		assert(q < inv.size());

		// Post(processed)

		auto& smallerTransitionIndex = smallerIndex[q];

		for (size_t symbol = 0; symbol < smallerTransitionIndex.size(); ++symbol)
		{
			size_t j = 0;

			for (auto& smallerTransitions : smallerTransitionIndex[symbol])
			{
				for (auto& smallerTransition : smallerTransitions)
				{
					assert(smallerTransition);

					if (!choiceVector.build(smallerTransition->children(), j))
					{
						continue;
					}

					do
					{
						post.clear();
						isAccepting = false;

						assert(choiceVector(0));

						auto firstSet = evalTransitions(symbol, 0, choiceVector(0).get());

						assert(firstSet);

						std::list<const Transition*> biggerTransitions(
							firstSet->begin(), firstSet->end()
						);

						for (size_t k = 1; k < choiceVector.size(); ++k)
						{
							assert(choiceVector(k));

							auto transitions = evalTransitions(
								symbol, k, choiceVector(k).get()
							);

							assert(transitions);

							intersectionByLookup(biggerTransitions, *transitions);
						}

						for (auto& biggerTransition : biggerTransitions)
						{
							assert(biggerTransition);
							assert(biggerTransition->state() < ind.size());

							if (post.contains(ind[biggerTransition->state()]))
							{
								continue;
							}

							assert(biggerTransition->state() < inv.size());

							post.refine(inv[biggerTransition->state()]);
							post.insert(biggerTransition->state());

							isAccepting = isAccepting ||
								biggerFinalStates.count(biggerTransition->state());
						}

						if (post.data().empty())
						{
							return false;
						}

						if (!isAccepting && smallerFinalStates.count(smallerTransition->state()))
						{
							return false;
						}

						StateSet tmp(post.data().begin(), post.data().end());

						std::sort(tmp.begin(), tmp.end());

						assert(smallerTransition->state() < ind.size());

						if (checkIntersection(ind[smallerTransition->state()], tmp))
						{
							continue;
						}

						auto ptr = biggerTypeCache.lookup(tmp);

						if (temporary.contains(ind[smallerTransition->state()], ptr, lte))
						{
							continue;
						}

						assert(smallerTransition->state() < inv.size());

						temporary.refine(inv[smallerTransition->state()], ptr, gte);
						temporary.insert(smallerTransition->state(), ptr);

					} while (choiceVector.next());

					for (auto& smallerBiggerListPair : temporary.data())
					{
						for (auto& bigger : smallerBiggerListPair.second)
						{
							assert(smallerBiggerListPair.first < ind.size());

							if (processed.contains(ind[smallerBiggerListPair.first], bigger, lte))
							{
								continue;
							}

							assert(smallerBiggerListPair.first < inv.size());

							processed.refine(
								inv[smallerBiggerListPair.first], bigger, gte, Eraser(next)
							);

							Antichain2C::TList::iterator iter =
								processed.insert(smallerBiggerListPair.first, bigger);

							next.insert(std::make_pair(smallerBiggerListPair.first, iter));
						}
					}

					temporary.clear();
				}

				++j;
			}
		}
	}

	return true;
}
