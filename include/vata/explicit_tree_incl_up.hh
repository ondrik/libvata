/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Upward inclusion for explicitly represented tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_INCL_UP_HH_
#define _VATA_EXPLICIT_TREE_INCL_UP_HH_

#include <algorithm>

#include <boost/functional/hash.hpp>

#include <vata/util/cache.hh>
#include <vata/util/antichain1c.hh>
#include <vata/util/antichain2c_v2.hh>

//#include <vata/vata.hh>
//#include <vata/util/convert.hh>

namespace VATA {

	class ExplicitUpwardInclusion;

}

namespace std {

	template<class T1, class T2>
	struct hash<std::pair<T1, T2>> {

		size_t operator()(const std::pair<T1, T2>& v) const {
			return boost::hash_value(v);
		}

	};

	template<class T>
	struct hash<std::vector<T>> {

		size_t operator()(const std::vector<T>& v) const {
			return boost::hash_value(v);
		}

	};

}

class VATA::ExplicitUpwardInclusion {

	template <class T1, class T2>
	static bool checkIntersection(const T1& s1, const T2& s2) {

		auto i1 = s1.begin(), i2 = s2.begin();

		while (i1 != s1.end() && i2 != s2.end()) {

			if (*i1 < *i2) ++i1;
			else if (*i2 < *i1) ++i2;
			else return true;

		}

		return false;

	}

	template <class T1, class T2>
	static void intersectionByLookup(T1& d, const T2& s) {

		for (auto i = d.begin(); i != d.end(); ) {

			if (s.count(*i) == 0)
				i = d.erase(i);
			else
				++i;

		}

	}

public:

	template <class Aut, class Rel>
	static bool Check(const Aut& smaller, const Aut& bigger, const Rel& preorder) {

		typedef Explicit::StateType SmallerType;
		typedef std::vector<Explicit::StateType> StateSet;

		typedef typename Aut::SymbolType SymbolType;
		typedef typename Aut::Transition Transition;
		typedef typename Aut::TransitionPtr TransitionPtr;

		typedef std::unordered_set<const Transition*> TransitionSet;
		typedef typename std::shared_ptr<TransitionSet> TransitionSetPtr;

		typedef typename Util::Cache<
			StateSet, std::function<void(const StateSet*)>
		> BiggerTypeCache;

		typedef typename BiggerTypeCache::TPtr BiggerType;

		typedef Util::CachedBinaryOp<const StateSet*, const StateSet*, bool> LteCache;

		typedef Util::CachedBinaryOp<
			std::pair<SymbolType, size_t>, const StateSet*, TransitionSetPtr
		> EvalTransitionsCache;

		typedef typename Util::Antichain1C<SmallerType> Antichain1C;
		typedef typename Util::Antichain2Cv2<SmallerType, BiggerType> Antichain2C;

		GCC_DIAG_OFF(effc++)
		struct Choice {
		GCC_DIAG_ON(effc++)

			const Antichain2C::TList* biggerList_;
			Antichain2C::TList::const_iterator current_;

			bool init(const Antichain2C::TList* biggerList) {

				if (!biggerList)
					return false;

				assert(biggerList->size());

				this->biggerList_ = biggerList;
				this->current_ = biggerList->begin();

				return true;

			}

			bool next() {

				if (++this->current_ != this->biggerList_->end())
					return true;

				this->current_ = this->biggerList_->begin();
				return false;

			}

			const BiggerType& get() const { return *this->current_; }
			
		};

		GCC_DIAG_OFF(effc++)
		struct ChoiceVector : public std::vector<Choice> {
		GCC_DIAG_ON(effc++)

			const Antichain2C& processed_;
			
		public:

			ChoiceVector(const Antichain2C& processed)
				: std::vector<Choice>(), processed_(processed) {}
		
			bool build(const Explicit::StateTuple& children) {

				assert(children.size());

				this->resize(children.size());

				for (size_t i = 0; i < this->size(); ++i) {

					if (!(*this)[i].init(this->processed_.lookup(children[i])))
						return false;

				}

				return true;

			}

			bool next() {

				for (auto& choice : *this) {

					if (choice.next())
						return true;

				}

				return false;

			}

		};

		typename Rel::IndexType ind, inv;

		preorder.buildIndex(ind, inv);

		auto noncachedLte = [&ind](const StateSet* x, const StateSet* y) -> bool {

			assert(x); assert(y);

			for (auto& s1 : *x) {

				assert(s1 < ind.size());

				if (!checkIntersection(ind[s1], *y))
					return false;

			}

			return true;

		};

		LteCache lteCache;

		auto lte = [&noncachedLte, &lteCache](const BiggerType& x, const BiggerType& y) -> bool {

			assert(x); assert(y);

			return (x.get() == y.get())?(true):(lteCache.lookup(x.get(), y.get(), noncachedLte));

		};
		
		auto gte = [&lte](const BiggerType& x, const BiggerType& y) { return lte(y, x); };

		typename Aut::SymbolToTransitionListMap smallerLeaves, biggerLeaves;
		typename Aut::IndexedSymbolToTransitionListMap smallerIndex;
		typename Aut::SymbolToDoubleIndexedTransitionListMap biggerIndex;

		smaller.bottomUpIndex(smallerIndex, smallerLeaves);
		bigger.bottomUpIndex(biggerIndex, biggerLeaves);

		auto noncachedEvalTransitions = [&biggerIndex](const std::pair<SymbolType, size_t>& key,
			const StateSet* states) -> TransitionSetPtr {

			assert(states);

			TransitionSetPtr result = TransitionSetPtr(new TransitionSet());

			auto iter = biggerIndex.find(key.first);

			if (iter == biggerIndex.end())
				return result;

			assert(key.second < iter->second.size());

			auto& indexedTransitionList = iter->second[key.second];

			for (auto& state: *states) {

				if (state >= indexedTransitionList.size())
					continue;

				for (auto& transition : indexedTransitionList[state])		
					result->insert(transition.get());

			}

			return result;

		};

		EvalTransitionsCache evalTransitionsCache;

		auto evalTransitions = [&noncachedEvalTransitions, &evalTransitionsCache](
			const SymbolType& symbol, size_t i, const StateSet* states)
			-> TransitionSetPtr {

			assert(states);

			return evalTransitionsCache.lookup(
				std::make_pair(symbol, i), states, noncachedEvalTransitions
			);

		};

		BiggerTypeCache biggerTypeCache(
			[&lteCache, &evalTransitionsCache](const StateSet* v) {
				lteCache.invalidateFirst(v);
				lteCache.invalidateSecond(v);
				evalTransitionsCache.invalidateSecond(v);
			}
		);

		std::unordered_set<SmallerType> active;

		Antichain1C post;
		Antichain2C next, processed;

		bool isAccepting;

		// Post(\emptyset)

		for (auto& smallerCluster : smallerLeaves) {

			auto biggerClusterIter = biggerLeaves.find(smallerCluster.first);

			if (biggerClusterIter == biggerLeaves.end())
				return false;

			post.clear();
			isAccepting = false;

			for (auto& transition : biggerClusterIter->second) {

				assert(transition);
				assert(transition->children().empty());
				assert(transition->state() < ind.size());

				if (post.contains(ind[transition->state()]))
					continue;

				assert(transition->state() < inv.size());

				post.refine(inv[transition->state()]);
				post.insert(transition->state());

				isAccepting = isAccepting || bigger.IsFinalState(transition->state());

			}

			StateSet tmp(post.data().begin(), post.data().end());

			std::sort(tmp.begin(), tmp.end());

			auto ptr = biggerTypeCache.lookup(tmp);

			for (auto& transition : smallerCluster.second) {

				assert(transition);

				if (!isAccepting && smaller.IsFinalState(transition->state()))
					return false;

				assert(transition->state() < ind.size());

				if (checkIntersection(ind[transition->state()], tmp))
					continue;

				if (processed.contains(ind[transition->state()], ptr, lte))
					continue;

				assert(transition->state() < inv.size());
					
				processed.refine(inv[transition->state()], ptr, gte);
				processed.insert(transition->state(), ptr);

				active.insert(transition->state());

			}

		}

		ChoiceVector choiceVector(processed);

		while (!active.empty()) {

			SmallerType q = *active.begin();

			active.erase(active.begin());

			// Post(processed)

			for (auto& symbolClusterPair : smallerIndex[q]) {

				assert(symbolClusterPair.second.size() > 0);

				auto biggerClusterIter = biggerIndex.find(symbolClusterPair.first);

				if (biggerClusterIter == biggerIndex.end())
					return false;

				for (auto& smallerTransition : symbolClusterPair.second) {

					assert(smallerTransition);

					if (!choiceVector.build(smallerTransition->children()))
						continue;

					do {

						post.clear();
						isAccepting = false;

						assert(biggerClusterIter->second.size() > 0);
						assert(choiceVector[0].get());

						auto firstSet = evalTransitions(
							symbolClusterPair.first, 0, choiceVector[0].get().get()
						);

						assert(firstSet);

						std::list<const Transition*> biggerTransitions(
							firstSet->begin(), firstSet->end()
						);

						for (size_t i = 1; i < choiceVector.size(); ++i) {

							assert(i < biggerClusterIter->second.size());
							assert(choiceVector[i].get());
			
							auto transitions = evalTransitions(
								symbolClusterPair.first, i, choiceVector[i].get().get()
							);

							assert(transitions);
							
							intersectionByLookup(biggerTransitions, *transitions);

						}

						for (auto& biggerTransition : biggerTransitions) {

							assert(biggerTransition);
							assert(biggerTransition->state() < ind.size());

							if (post.contains(ind[biggerTransition->state()]))
								continue;

							assert(biggerTransition->state() < inv.size());

							post.refine(inv[biggerTransition->state()]);
							post.insert(biggerTransition->state());
							
							isAccepting = isAccepting ||
								bigger.IsFinalState(biggerTransition->state());

						}
						
						if (post.data().empty())
							return false;

						if (!isAccepting && smaller.IsFinalState(smallerTransition->state()))
							return false;
	
						StateSet tmp(post.data().begin(), post.data().end());

						std::sort(tmp.begin(), tmp.end());
						
						assert(smallerTransition->state() < ind.size());
	
						if (checkIntersection(ind[smallerTransition->state()], tmp))
							continue;
	
						auto ptr = biggerTypeCache.lookup(tmp);
	
						if (next.contains(ind[smallerTransition->state()], ptr, lte))
							continue;
		
						assert(smallerTransition->state() < inv.size());
	
						next.refine(inv[smallerTransition->state()], ptr, gte);
						next.insert(smallerTransition->state(), ptr);

					} while (choiceVector.next());

					for (auto& smallerBiggerListPair : next.data()) {

						for (auto& bigger : smallerBiggerListPair.second) {

							assert(smallerBiggerListPair.first < ind.size());
	
							if (processed.contains(ind[smallerBiggerListPair.first], bigger, lte))
								continue;
			
							assert(smallerBiggerListPair.first < inv.size());
	
							processed.refine(inv[smallerBiggerListPair.first], bigger, gte);
							processed.insert(smallerBiggerListPair.first, bigger);
	
							active.insert(smallerBiggerListPair.first);
							
						}

					}

					next.clear();

				}

			}

		}

		return true;

	}

	template <class Aut, class Rel>
	static bool CheckObsolete(const Aut& smaller, const Aut& bigger, const Rel& preorder) {

		typedef Explicit::StateType SmallerType;
		typedef std::vector<Explicit::StateType> StateSet;
		typedef typename Aut::TransitionPtr TransitionPtr;

		typedef typename Util::Cache<
			StateSet, std::function<void(const StateSet*)>
		> BiggerTypeCache;

		typedef typename BiggerTypeCache::TPtr BiggerType;

		typedef Util::CachedBinaryOp<const StateSet*, const StateSet*, bool> LteCache;

		typedef typename Util::Antichain1C<SmallerType> Antichain1C;
		typedef typename Util::Antichain2Cv2<SmallerType, BiggerType> Antichain2C;

		GCC_DIAG_OFF(effc++)
		struct Choice {
		GCC_DIAG_ON(effc++)

			const Antichain2C::TList* biggerList_;
			Antichain2C::TList::const_iterator current_;

			bool init(const Antichain2C::TList* biggerList) {

				if (!biggerList)
					return false;

				this->biggerList_ = biggerList;
				this->current_ = biggerList->begin();

				return true;

			}

			bool next() {

				if (++this->current_ != this->biggerList_->end())
					return true;

				this->current_ = this->biggerList_->begin();
				return false;

			}
			
		};

		struct ChoiceVector {

			const Antichain2C& processed_;
			const Antichain2C::TList& fixed_;
			std::vector<Choice> state_;
			
		public:

			ChoiceVector(const Antichain2C& processed,
				const Antichain2C::TList& fixed)
				: processed_(processed), fixed_(fixed), state_() {}
		
			bool get(const Explicit::StateTuple& children, size_t index) {

				assert(index < children.size());

				this->state_.resize(children.size());

				for (size_t i = 0; i < index; ++i) {

					if (!this->state_[i].init(this->processed_.lookup(children[i])))
						return false;

				}

				this->state_[index].biggerList_ = &this->fixed_;
				this->state_[index].current_ = this->fixed_.begin();

				for (size_t i = index + 1; i < children.size(); ++i) {

					if (!this->state_[i].init(this->processed_.lookup(children[i])))
						return false;

				}

				return true;

			}
		
			bool next() {

				for (auto& choice : this->state_) {

					if (choice.next())
						return true;

				}

				return false;

			}
			
			bool match(const Explicit::StateTuple& children) const {

				for (size_t i = 0; i < children.size(); ++i) {

					assert(i < this->state_.size());

					auto& s = *this->state_[i].current_;

					if (!std::binary_search(s->begin(), s->end(), children[i]))
						return false;

				}

				return true;

			}

		};

		typename Rel::IndexType ind, inv;

		preorder.buildIndex(ind, inv);

		auto noncachedLte = [&ind](const StateSet* x, const StateSet* y) -> bool {

			assert(x); assert(y);

			for (auto& s1 : *x) {

				assert(s1 < ind.size());

				if (!checkIntersection(ind[s1], *y))
					return false;

			}

			return true;

		};

		LteCache lteCache;

		auto lte = [&noncachedLte, &lteCache](const BiggerType& x, const BiggerType& y) -> bool {

			assert(x); assert(y);

			return (x.get() == y.get())?(true):(lteCache.lookup(x.get(), y.get(), noncachedLte));

		};

		auto gte = [&lte](const BiggerType& x, const BiggerType& y) { return lte(y, x); };

		typename Aut::IndexedSymbolToIndexedTransitionListMap smallerIndex;
		typename Aut::SymbolToTransitionListMap smallerLeaves, biggerIndex;

		smaller.bottomUpIndex(smallerIndex, smallerLeaves);
		bigger.bottomUpIndex(biggerIndex);

		BiggerTypeCache biggerTypeCache(
			[&lteCache](const StateSet* v) {
					lteCache.invalidateFirst(v);
					lteCache.invalidateSecond(v);
			}
		);

		Antichain1C post;
		Antichain2C next, processed;

		bool isAccepting;

		// Post(\emptyset)

		for (auto& smallerCluster : smallerLeaves) {

			auto biggerClusterIter = biggerIndex.find(smallerCluster.first);

			if (biggerClusterIter == biggerIndex.end())
				return false;

			post.clear();
			isAccepting = false;

			for (auto& transition : biggerClusterIter->second) {

				assert(transition);
				assert(transition->children().empty());
				assert(transition->state() < ind.size());

				if (post.contains(ind[transition->state()]))
					continue;
				
				assert(transition->state() < inv.size());
				
				post.refine(inv[transition->state()]);
				post.insert(transition->state());
				
				isAccepting = isAccepting || bigger.IsFinalState(transition->state());

			}

			StateSet tmp(post.data().begin(), post.data().end());

			std::sort(tmp.begin(), tmp.end());

			auto ptr = biggerTypeCache.lookup(tmp);

			for (auto& transition : smallerCluster.second) {

				assert(transition);

				if (!isAccepting && smaller.IsFinalState(transition->state()))
					return false;

				assert(transition->state() < ind.size());

				if (checkIntersection(ind[transition->state()], tmp))
					continue;

				if (next.contains(ind[transition->state()], ptr, lte))
					continue;

				assert(transition->state() < inv.size());
					
				next.refine(inv[transition->state()], ptr, gte);
				next.insert(transition->state(), ptr);

			}

		}

		SmallerType q;

		Antichain2C::TList fixedList(1);

		BiggerType& Q = fixedList.front();

		ChoiceVector choiceVector(processed, fixedList);

		while (next.next(q, Q)) {

			assert(q < inv.size());

			processed.refine(inv[q], Q, gte);
			processed.insert(q, Q);

			// Post(processed)

			auto& smallerClusters = smallerIndex[q];

			for (auto& symbolClusterPair : smallerClusters) {

				assert(symbolClusterPair.second.size() > 0);

				auto biggerClusterIter = biggerIndex.find(symbolClusterPair.first);

				if (biggerClusterIter == biggerIndex.end())
					return false;

				size_t i = 0;

				for (auto& smallerTransitions : symbolClusterPair.second) {

					for (auto& smallerTransition : smallerTransitions) {

						assert(smallerTransition);

						if (!choiceVector.get(smallerTransition->children(), i))
							continue;

						do {

							post.clear();
							isAccepting = false;

							for (auto& biggerTransition : biggerClusterIter->second) {

								assert(biggerTransition);
								assert(biggerTransition->state() < ind.size());
								
								if (post.contains(ind[biggerTransition->state()]))
									continue;

								if (!choiceVector.match(biggerTransition->children()))
									continue;

								assert(biggerTransition->state() < inv.size());
								
								post.refine(inv[biggerTransition->state()]);
								post.insert(biggerTransition->state());
								
								isAccepting = isAccepting ||
									bigger.IsFinalState(biggerTransition->state());

							}
							
							if (post.data().empty())
								return false;
	
							if (!isAccepting && smaller.IsFinalState(smallerTransition->state()))
								return false;

							assert(smallerTransition->state() < ind.size());
	
							StateSet tmp(post.data().begin(), post.data().end());

							std::sort(tmp.begin(), tmp.end());
	
							if (checkIntersection(ind[smallerTransition->state()], tmp))
								continue;
	
							auto ptr = biggerTypeCache.lookup(tmp);
	
							if (processed.contains(ind[smallerTransition->state()], ptr, lte))
								continue;

							if (next.contains(ind[smallerTransition->state()], ptr, lte))
								continue;

							assert(smallerTransition->state() < inv.size());
	
							next.refine(inv[smallerTransition->state()], ptr, gte);
							next.insert(smallerTransition->state(), ptr);

						} while (choiceVector.next());

					}

					++i;

				}

			}

		}

		return true;

	}

};

#endif
