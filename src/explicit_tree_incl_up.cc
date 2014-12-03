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

using VATA::Util::Convert;
using VATA::ExplicitUpwardInclusion;
using VATA::ExplicitTreeAutCore;

typedef VATA::ExplicitTreeAutCore::StateType SmallerType;
typedef std::vector<VATA::ExplicitTreeAutCore::StateType> StateSet;

typedef size_t SymbolType;

using WitnessTrace      = std::set<ExplicitUpwardInclusion::TransitionPtr>;

namespace
{
	struct SetTracePair
	{
		StateSet        set = {};
		WitnessTrace    trace = {};

		size_t size() const
		{
			return set.size();
		}

		bool operator==(const SetTracePair& rhs) const
		{
			return set == rhs.set;
		}
	};

	std::size_t hash_value(SetTracePair const& stp)
	{
		boost::hash<StateSet> hasher;
		return hasher(stp.set);
	}
}


typedef typename VATA::Util::Cache<SetTracePair> BiggerTypeCache;
// typedef typename VATA::Util::Cache<StateSet> BiggerTypeCache;

typedef typename BiggerTypeCache::TPtr BiggerType;

typedef typename VATA::Util::Antichain1C<SmallerType> Antichain1C;
typedef typename VATA::Util::Antichain2Cv2<SmallerType, BiggerType> Antichain2C;


class AntichainElem
{
public:   // types

	using SmallerStateType  = SmallerType;
	using BiggerSetType     = Antichain2C::TList::iterator;
	using TraceType         = WitnessTrace;
	using TransitionPtr     = ExplicitUpwardInclusion::TransitionPtr;
	using StateType         = ExplicitTreeAutCore::StateType;

private:  // data members

	SmallerStateType smaller_;
	BiggerSetType bigger_;
	TraceType trace_;

public:   // methods

	AntichainElem(
		const SmallerStateType&   smaller,
		const BiggerSetType&      bigger,
		const TraceType&          trace = {}) :
		smaller_(smaller),
		bigger_(bigger),
		trace_(trace)
	{ }

	SmallerStateType GetSmallerState() const
	{
		return smaller_;
	}

	BiggerSetType GetBiggerSet() const
	{
		return bigger_;
	}

	void AppendToTrace(const TransitionPtr& trans)
	{
		trace_.insert(trans);
	}

	const TraceType& GetTrace() const
	{
		return trace_;
	}

	static ExplicitTreeAutCore ConvertTraceToAut(
		const TraceType&    trace,
		StateType           finalState)
	{
		ExplicitTreeAutCore aut;

		for (const TransitionPtr& trans : trace)
		{
			aut.AddTransition(
				trans->children(),
				trans->symbol(),
				trans->state());
		}

		aut.SetStateFinal(finalState);

		return aut;
	}

	struct less
	{
		bool operator()(
			const AntichainElem&       p1,
			const AntichainElem&       p2) const
		{
			if ((*p1.GetBiggerSet())->size() < (*p2.GetBiggerSet())->size()) return true;
			if ((*p1.GetBiggerSet())->size() > (*p2.GetBiggerSet())->size()) return false;
			if (p1.GetSmallerState() < p2.GetSmallerState()) return true;
			if (p1.GetSmallerState() > p2.GetSmallerState()) return false;
			return (*p1.GetBiggerSet()).get() < (*p2.GetBiggerSet()).get();
		}
	};
};

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


using OrderedType = std::set<AntichainElem, AntichainElem::less>;

struct Eraser
{
	OrderedType& data_;

	Eraser(OrderedType& data) : data_(data) {}

	void operator()(
		const SmallerType&                               q,
		const typename Antichain2C::TList::iterator&     Q) const
	{
		this->data_.erase(AntichainElem(q, Q));
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
	const StateDiscontBinaryRelation::IndexType&      ind,
	const StateDiscontBinaryRelation::IndexType&      inv,
	InclContext&                                      context)
{
	auto noncachedLte = [&ind](const StateSet* x, const StateSet* y) -> bool
	{
		assert(nullptr != x);
		assert(nullptr != y);

		for (auto& s1 : *x)
		{
			assert(s1 < ind.size());

			if (!checkIntersection(ind.at(s1), *y))
			{
				return false;
			}
		}

		return true;
	};

	Util::CachedBinaryOp<const StateSet*, const StateSet*, bool> lteCache;

	auto lte = [&noncachedLte, &lteCache](const BiggerType& x, const BiggerType& y) -> bool
	{
		assert(nullptr != x);
		assert(nullptr != y);

		return (x.get() == y.get())?(true):(lteCache.lookup(&x.get()->set, &y.get()->set, noncachedLte));
	};

	auto gte = [&lte](const BiggerType& x, const BiggerType& y) { return lte(y, x); };

	typedef VATA::ExplicitUpwardInclusion::Transition Transition;
	typedef std::unordered_set<const Transition*> TransitionSet;
	typedef typename std::shared_ptr<TransitionSet> TransitionSetPtr;

	auto noncachedEvalTransitions = [&biggerIndex](const std::pair<SymbolType, size_t>& key,
		const StateSet* states) -> TransitionSetPtr
	{
		assert(nullptr != states);

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
		const SymbolType& symbol, size_t i, const StateSet* states) -> TransitionSetPtr
		{
			assert(nullptr != states);

			return evalTransitionsCache.lookup(
				std::make_pair(symbol, i), states, noncachedEvalTransitions
			);
		};

	BiggerTypeCache biggerTypeCache(
		[&lteCache, &evalTransitionsCache](const SetTracePair* v)
		{
			assert(nullptr != v);

			lteCache.invalidateFirst(&v->set);
			lteCache.invalidateSecond(&v->set);
			evalTransitionsCache.invalidateSecond(&v->set);
		}
	);

	Antichain1C post;

	Antichain2C temporary, processed;

	OrderedType next;

	// Post(\emptyset)

	if (biggerLeaves.size() < smallerLeaves.size())
	{
		context.SetDescription("Inclusion refuted! Reason: leaves set sizes incompatible.\n"
			"Witness NOT provided");

		return false;
	}

	for (size_t symbol = 0; symbol < smallerLeaves.size(); ++symbol)
	{
		post.clear();
		bool isAccepting = false;

		for (auto& transition : biggerLeaves[symbol])
		{
			assert(nullptr != transition);
			assert(transition->children().empty());
			assert(transition->state() < ind.size());

			if (post.contains(ind.at(transition->state())))
			{
				continue;
			}

			assert(transition->state() < inv.size());

			post.refine(inv.at(transition->state()));
			post.insert(transition->state());

			isAccepting = isAccepting || biggerFinalStates.count(transition->state());
		}

		StateSet tmp(post.data().begin(), post.data().end());

		std::sort(tmp.begin(), tmp.end());

		// TODO: check if this is OK...
		SetTracePair stp;
		stp.set = tmp;
		// TODO: I don't know whether the trace should stay empty here

		auto ptr = biggerTypeCache.lookup(stp);
		// auto ptr = biggerTypeCache.lookup(tmp);

		for (auto& transition : smallerLeaves[symbol])
		{
			assert(nullptr != transition);

			VATA_DEBUG("Processing transition " + VATA::Util::Convert::ToString(*transition));

			if (!isAccepting && smallerFinalStates.count(transition->state()))
			{
				context.SetDescription("Inclusion refuted! Reason: leaves not covered.\n"
					"Witness NOT provided");

				return false;
			}

			assert(transition->state() < ind.size());

			if (checkIntersection(ind.at(transition->state()), tmp))
			{
				continue;
			}

			if (processed.contains(ind.at(transition->state()), ptr, lte))
			{
				continue;
			}

			assert(transition->state() < inv.size());

			processed.refine(inv.at(transition->state()), ptr, gte, Eraser(next));

			Antichain2C::TList::iterator iter = processed.insert(transition->state(), ptr);

			AntichainElem newElem = AntichainElem(transition->state(), iter);
			newElem.AppendToTrace(transition);
			next.insert(newElem);
		}
	}

	SmallerType q;

	Antichain2C::TList fixedList(1);

	BiggerType& Q = fixedList.front();

	ChoiceVector choiceVector(processed, fixedList);

	VATA_DEBUG("Printing transitions of the bigger guy:");
	for (const auto& biggerTransitionIndex : biggerIndex)
	{
		for (size_t symbol = 0; symbol < biggerTransitionIndex.size(); ++symbol)
		{
			for (const TransitionList& biggerTransitions : biggerTransitionIndex[symbol])
			{
				for (const TransitionPtr& biggerTransition : biggerTransitions)
				{
					assert(nullptr != biggerTransition);

					VATA_DEBUG("Transition " + Convert::ToString(*biggerTransition));
				}
			}
		}
	}

	while (!next.empty())
	{
		q = next.begin()->GetSmallerState();
		Q = *next.begin()->GetBiggerSet();

		// TODO: this might be done in a more efficient way...
		AntichainElem::TraceType trace = next.begin()->GetTrace();

		next.erase(next.begin());

		assert(q < inv.size());

		VATA_DEBUG("Processing smaller state " + VATA::Util::Convert::ToString(q));

		// Post(processed)

		const SymbolToIndexedTransitionListMap& smallerTransitionIndex = smallerIndex[q];

		for (size_t symbol = 0; symbol < smallerTransitionIndex.size(); ++symbol)
		{
			size_t j = 0;

			for (const TransitionList& smallerTransitions : smallerTransitionIndex[symbol])
			{
				for (const TransitionPtr& smallerTransition : smallerTransitions)
				{
					assert(nullptr != smallerTransition);

					VATA_DEBUG("Processing transition " + VATA::Util::Convert::ToString(*smallerTransition));

					if (!choiceVector.build(smallerTransition->children(), j))
					{
						continue;
					}

					do
					{
						post.clear();

						assert(nullptr != choiceVector(0));

						auto firstSet = evalTransitions(symbol, 0, &choiceVector(0).get()->set);

						assert(nullptr != firstSet);

						VATA_DEBUG("firstSet = " + Convert::ToString(*firstSet));

						std::list<const Transition*> biggerTransitions(
							firstSet->begin(), firstSet->end()
						);

						for (size_t k = 1; k < choiceVector.size(); ++k)
						{
							assert(nullptr != choiceVector(k));

							auto transitions = evalTransitions(
								symbol, k, &choiceVector(k).get()->set
							);

							assert(nullptr != transitions);

							intersectionByLookup(biggerTransitions, *transitions);
						}

						bool isBiggerAccepting = false;
						for (auto& biggerTransition : biggerTransitions)
						{
							assert(nullptr != biggerTransition);
							assert(biggerTransition->state() < ind.size());

							if (post.contains(ind.at(biggerTransition->state())))
							{
								continue;
							}

							assert(biggerTransition->state() < inv.size());

							post.refine(inv.at(biggerTransition->state()));
							post.insert(biggerTransition->state());

							bool isThisAccepting = (biggerFinalStates.end()
								!= biggerFinalStates.find(biggerTransition->state()));
							isBiggerAccepting = isBiggerAccepting || isThisAccepting;
						}

						bool isSmallerAccepting = (smallerFinalStates.end()
							!= smallerFinalStates.find(smallerTransition->state()));
						if (post.data().empty() ||
							(!isBiggerAccepting && isSmallerAccepting))
						{	// if the smaller can accept and the bigger cannot, we found a witness
							VATA_DEBUG("Failing trans: " + VATA::Util::Convert::ToString(*smallerTransition));
							if (post.data().empty())
							{
								context.SetDescription("Inclusion refuted! Reason: bigger post() empty");
							}
							else
							{
								context.SetDescription("Inclusion refuted! Reason: smaller accepts, bigger does not");
							}

							trace.insert(smallerTransition);

							// TODO: this is probably wrong...
							context.SetWitness(AntichainElem::ConvertTraceToAut(trace,
								smallerTransition->state()));

							return false;
						}

						StateSet tmp(post.data().begin(), post.data().end());

						std::sort(tmp.begin(), tmp.end());

						assert(smallerTransition->state() < ind.size());

						if (checkIntersection(ind.at(smallerTransition->state()), tmp))
						{
							continue;
						}

						// TODO: check if this is OK...
						SetTracePair stp;
						stp.set = tmp;
						stp.trace = trace;
						stp.trace.insert(smallerTransition);

						auto ptr = biggerTypeCache.lookup(stp);
						// auto ptr = biggerTypeCache.lookup(tmp);

						if (temporary.contains(ind.at(smallerTransition->state()), ptr, lte))
						{
							continue;
						}

						assert(smallerTransition->state() < inv.size());

						temporary.refine(inv.at(smallerTransition->state()), ptr, gte);
						temporary.insert(smallerTransition->state(), ptr);

					} while (choiceVector.next());

					for (auto& smallerBiggerListPair : temporary.data())
					{
						for (auto& bigger : smallerBiggerListPair.second)
						{
							assert(smallerBiggerListPair.first < ind.size());

							if (processed.contains(ind.at(smallerBiggerListPair.first), bigger, lte))
							{
								continue;
							}

							assert(smallerBiggerListPair.first < inv.size());

							processed.refine(
								inv.at(smallerBiggerListPair.first), bigger, gte, Eraser(next)
							);

							Antichain2C::TList::iterator iter =
								processed.insert(smallerBiggerListPair.first, bigger);

							AntichainElem newElem(smallerBiggerListPair.first, iter, trace);
							newElem.AppendToTrace(smallerTransition);
							next.insert(newElem);
						}
					}

					temporary.clear();
				}

				++j;
			}
		}
	}

	context.SetDescription("Inclusion proved!");

	return true;
}
