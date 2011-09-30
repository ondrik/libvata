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

#include <vata/util/cache.hh>
#include <vata/util/antichain2c_v2.hh>

//#include <vata/vata.hh>
//#include <vata/util/convert.hh>

namespace VATA {

	class ExplicitUpwardInclusion;

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

			if (s.count(*i) == 0) {

				auto j = i++;
				
				d.erase(j);

			} else ++i;

		}

	}

	template <class T>
	struct Hash {
		size_t operator()(const T& v) const { return boost::hash_value(v); }
	};
/*
	friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<std::vector<Explicit::StateType>>& s) {
		return os << Util::Convert::ToString(*s);
	}
*/
public:

	template <class Aut, class Rel>
	static bool Check(const Aut& smaller, const Aut& bigger, const Rel& preorder) {

		typedef Explicit::StateType SmallerType;
		typedef std::vector<Explicit::StateType> StateSet;
		typedef typename Aut::Transition Transition;
		typedef typename Aut::TransitionPtr TransitionPtr;
		typedef std::unordered_set<const Transition*> TransitionSet;

		typedef typename Util::Cache<
			StateSet, std::function<void(const StateSet*)>, Hash<StateSet>
		> BiggerTypeCache;

		typedef typename BiggerTypeCache::TPtr BiggerType;

		typedef Util::CachedBinaryOp<
			const StateSet*,
			const StateSet*,
			bool,
			Hash<std::pair<const StateSet*, const StateSet*>>
		> LTECache;

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

		GCC_DIAG_OFF(effc++)
		struct AntichainSet : public std::set<Explicit::StateType> {
		GCC_DIAG_ON(effc++)

			const typename Rel::IndexType& ind_;
			const typename Rel::IndexType& inv_;

		public:

			AntichainSet(const typename Rel::IndexType& ind, const typename Rel::IndexType& inv)
				: ind_(ind), inv_(inv) {}

			bool testAndRefine(const Explicit::StateType& state) {

				assert(state < this->ind_.size());

				for (auto& biggerState : this->ind_[state]) {
					
					if (this->find(biggerState) != this->end())
						return false;

				}

				assert(state < this->inv_.size());

				for (auto& smallerState : this->inv_[state])
					this->erase(smallerState);

				this->insert(state);

				return true;

			}

			bool next(Explicit::StateType& s) {

				if (this->empty())
					return false;

				s = *this->begin();

				this->erase(this->begin());

				return true;
				
			}
		
		};

		GCC_DIAG_OFF(effc++)
		struct AcceptingAntichainSet : public AntichainSet {
		GCC_DIAG_ON(effc++)

			const Aut& aut_;
			bool isAccepting_;

		public:

			AcceptingAntichainSet(const Aut& aut, const typename Rel::IndexType& ind,
				const typename Rel::IndexType& inv) : AntichainSet(ind, inv), aut_(aut),
				isAccepting_(false) {}

			bool isAccepting() const { return this->isAccepting_; }

			bool testAndRefine(const Explicit::StateType& state) {

				if (!static_cast<AntichainSet*>(this)->testAndRefine(state))
					return false;

				this->isAccepting_ = this->isAccepting_ || this->aut_.IsFinalState(state);
				return true;

			}

			void clear() {

				static_cast<AntichainSet*>(this)->clear();

				this->isAccepting_ = false;

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

		LTECache lteCache;

		auto lte = [&noncachedLte, &lteCache](const BiggerType& x, const BiggerType& y) -> bool {

			assert(x); assert(y);

			return (x.get() == y.get())?(true):(lteCache.lookup(x.get(), y.get(), noncachedLte));

		};
		
		auto gte = [&lte](const BiggerType& x, const BiggerType& y) { return lte(y, x); };

		typename Aut::SymbolToTransitionListMap smallerLeaves, biggerLeaves;
		typename Aut::IndexedSymbolToTransitionListMap smallerIndex;
		typename Aut::SymbolToDoubleIndexedTransitionListMap biggerIndex;

		auto noncachedEvalTransitions = [](
			const typename Aut::IndexedTransitionList& indexedTransitionList, const StateSet& states
			) -> TransitionSet {

			TransitionSet result;

			for (auto& state: states) {

				if (state >= indexedTransitionList.size())
					continue;

				for (auto& transition : indexedTransitionList[state])		
					result.insert(transition.get());

			}

			return result;

		};

		smaller.bottomUpIndex(smallerIndex, smallerLeaves);
		bigger.bottomUpIndex(biggerIndex, biggerLeaves);

		BiggerTypeCache biggerTypeCache(
			[&lteCache](const StateSet* v) {
				lteCache.invalidateFirst(v);
				lteCache.invalidateSecond(v);
			}
		);

		Antichain2C processed, next;

		AntichainSet active(ind, inv);

		AcceptingAntichainSet post(bigger, ind, inv);

		// Post(\emptyset)

		for (auto& smallerCluster : smallerLeaves) {

			auto biggerClusterIter = biggerLeaves.find(smallerCluster.first);

			if (biggerClusterIter == biggerLeaves.end())
				return false;

			post.clear();

			for (auto& transition : biggerClusterIter->second) {

				assert(transition);
				assert(transition->children().empty());

//				VATA_LOGGER_INFO("bigger: " + Util::Convert::ToString(*transition));

				post.testAndRefine(transition->state());

			}

			StateSet tmp(post.begin(), post.end());

			auto ptr = biggerTypeCache.lookup(tmp);

			for (auto& transition : smallerCluster.second) {

				assert(transition);

//				VATA_LOGGER_INFO("smaller: " + Util::Convert::ToString(*transition));

				if (!post.isAccepting() && smaller.IsFinalState(transition->state()))
					return false;

				assert(transition->state() < ind.size());

				if (checkIntersection(ind[transition->state()], tmp))
					continue;

				if (!processed.contains(ind[transition->state()], ptr, lte)) {

//					VATA_LOGGER_INFO(Util::Convert::ToString(transition->state()) + ", " + Util::Convert::ToString(tmp));

					assert(transition->state() < inv.size());
					
					processed.refine(inv[transition->state()], ptr, gte);
					processed.insert(transition->state(), ptr);

					active.testAndRefine(transition->state());

				}

			}

		}

		ChoiceVector choiceVector(processed);

		size_t c = 0;

		SmallerType q; BiggerType Q;

		while (active.next(q)) {

			++c;

//			VATA_LOGGER_INFO("processed: " + Util::Convert::ToString(processed));

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

//					VATA_LOGGER_INFO("smaller: " + Util::Convert::ToString(*smallerTransition));

					do {

						post.clear();

						assert(biggerClusterIter->second.size() > 0);
						assert(choiceVector[0].get());

						auto firstSet = noncachedEvalTransitions(
							biggerClusterIter->second[0], *choiceVector[0].get()
						);

						std::list<const Transition*> biggerTransitions(
							firstSet.begin(), firstSet.end()
						);

						for (size_t i = 1; i < choiceVector.size(); ++i) {

							assert(i < biggerClusterIter->second.size());
							assert(choiceVector[i].get());
			
							intersectionByLookup(
								biggerTransitions,
								noncachedEvalTransitions(
									biggerClusterIter->second[i], *choiceVector[i].get()
								)
							);

						}

						for (auto& biggerTransition : biggerTransitions) {

							assert(biggerTransition);

							post.testAndRefine(biggerTransition->state());
//							if (post.testAndRefine(biggerTransition->state()))
//								VATA_LOGGER_INFO("bigger: " + Util::Convert::ToString(*biggerTransition));

						}
						
						if (post.empty())
							return false;
	
						if (!post.isAccepting() && smaller.IsFinalState(smallerTransition->state()))
							return false;
	
						assert(smallerTransition->state() < ind.size());
	
						StateSet tmp(post.begin(), post.end());
	
						if (checkIntersection(ind[smallerTransition->state()], tmp))
							continue;
	
						auto ptr = biggerTypeCache.lookup(tmp);
	
						if (next.contains(ind[smallerTransition->state()], ptr, lte))
							continue;
		
						assert(smallerTransition->state() < inv.size());
	
						next.refine(inv[smallerTransition->state()], ptr, gte);
						next.insert(smallerTransition->state(), ptr);

					} while (choiceVector.next());

					while (next.next(q, Q)) {

						if (processed.contains(ind[q], Q, lte))
							continue;
		
						assert(smallerTransition->state() < inv.size());
	
						processed.refine(inv[q], Q, gte);
						processed.insert(q, Q);

						active.testAndRefine(q);

					}

				}

			}

		}

//		VATA_LOGGER_INFO("elements processed: " + Util::Convert::ToString(c));

		return true;

	}

};

#endif
