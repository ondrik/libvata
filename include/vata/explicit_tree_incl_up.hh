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

	template <class T>
	struct Hash {
		size_t operator()(const T& v) const { return boost::hash_value(v); }
	};

public:

	template <class Aut, class Rel>
	static bool Check(const Aut& smaller, const Aut& bigger, const Rel& preorder) {

		typedef Explicit::StateType SmallerType;
		typedef std::vector<Explicit::StateType> StateSet;
//		typedef std::set<Explicit::StateType> StateSet;
		typedef typename Aut::TransitionPtr TransitionPtr;

		typedef typename Util::Cache<
			StateSet, std::function<void(const StateSet*)>, Hash<StateSet>
		> BiggerTypeCache;

		typedef typename BiggerTypeCache::TPtr BiggerType;

		typedef Util::CachedBinaryOp<
			const StateSet*, bool, Hash<std::pair<const StateSet*, const StateSet*>>
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
//					if (s->count(children[i]) == 0)
						return false;

				}

				return true;

			}

		};

		GCC_DIAG_OFF(effc++)
		struct AntichainSet : public std::set<Explicit::StateType> {
		GCC_DIAG_ON(effc++)

			const Aut& aut_;
			const typename Rel::IndexType& ind_;
			const typename Rel::IndexType& inv_;
			bool isAccepting_;

//			std::unordered_set<Explicit::StateType> cache_;

		public:

			AntichainSet(const Aut& aut, const typename Rel::IndexType& ind,
				const typename Rel::IndexType& inv) : aut_(aut), ind_(ind), inv_(inv),
				isAccepting_(false) {}

			bool isAccepting() const { return this->isAccepting_; }

			bool cached(const Explicit::StateType& state) const {

				return false;

//				return this->cache_.count(state);

			}

			void testAndRefine(const Explicit::StateType& state) {

//				assert(!this->cached(state));

//				this->cache_.insert(state);

				assert(state < this->ind_.size());

				for (auto& biggerState : this->ind_[state]) {
					
					if (this->find(biggerState) != this->end())
						return;

				}

				assert(state < this->inv_.size());

				for (auto& smallerState : this->inv_[state])
					this->erase(smallerState);

				this->insert(state);
				this->isAccepting_ = this->isAccepting_ || this->aut_.IsFinalState(state);

			}

			void clear() {

				static_cast<std::set<Explicit::StateType>*>(this)->clear();

				this->isAccepting_ = false;
//				this->cache_.clear();

			}
		
		};

		typename Rel::IndexType ind, inv;

		preorder.buildIndex(ind, inv);

		auto coreLTE = [&ind](const StateSet* x, const StateSet* y) -> bool {

			assert(x); assert(y);

			for (auto& s1 : *x) {

				assert(s1 < ind.size());

				if (!checkIntersection(ind[s1], *y))
					return false;

			}

			return true;

		};

		LTECache lteCache;

		auto LTE = [&coreLTE, &lteCache](const BiggerType& x, const BiggerType& y) -> bool {

			assert(x); assert(y);

			return (x.get() == y.get())?(true):(lteCache.lookup(x.get(), y.get(), coreLTE));

		};
		
		auto GTE = [&LTE](const BiggerType& x, const BiggerType& y) { return LTE(y, x); };

		typename Aut::SymbolToTransitionListMap smallerLeaves;
		typename Aut::BottomUpIndex smallerIndex;
		typename Aut::SimpleBottomUpIndex biggerIndex;

		smaller.bottomUpIndex(smallerIndex, smallerLeaves);
		bigger.bottomUpIndex(biggerIndex);

		BiggerTypeCache biggerTypeCache(
			[&lteCache](const StateSet* v) { lteCache.invalidateKey(v); }
		);

		Antichain2C next, processed;

		AntichainSet antichainSet(bigger, ind, inv);

		// Post(\emptyset)

		for (auto& smallerCluster : smallerLeaves) {

			auto biggerClusterIter = biggerIndex.find(smallerCluster.first);

			if (biggerClusterIter == biggerIndex.end())
				return false;

			antichainSet.clear();

			for (auto& transition : biggerClusterIter->second) {

				assert(transition);
				assert(transition->children().empty());

//				VATA_LOGGER_INFO("bigger: " + Util::Convert::ToString(*transition));

				if (antichainSet.cached(transition->state()))
					continue;

				antichainSet.testAndRefine(transition->state());

			}

			StateSet tmp(antichainSet.begin(), antichainSet.end());

			auto ptr = biggerTypeCache.lookup(tmp);
//			auto ptr = biggerTypeCache.lookup(antichainSet);

			for (auto& transition : smallerCluster.second) {

				assert(transition);

//				VATA_LOGGER_INFO("smaller: " + Util::Convert::ToString(*transition));

				if (!antichainSet.isAccepting() && smaller.IsFinalState(transition->state()))
					return false;

				assert(transition->state() < ind.size());

				if (checkIntersection(ind[transition->state()], tmp))
					continue;

//				if (checkIntersection(ind[transition->state()], antichainSet))
//					continue;

				if (!next.contains(ind[transition->state()], ptr, LTE)) {

//					VATA_LOGGER_INFO(Util::Convert::ToString(transition->state()) + ", " + Util::Convert::ToString(tmp));

					assert(transition->state() < inv.size());
					
					next.refine(inv[transition->state()], ptr, GTE);
					next.insert(transition->state(), ptr);

				}

			}

		}

		SmallerType q;

		Antichain2C::TList fixedList(1);

		BiggerType& Q = fixedList.front();

		ChoiceVector choiceVector(processed, fixedList);

//		VATA_LOGGER_INFO("next: " + Util::Convert::ToString(next));

		size_t c = 0;

		while (next.next(q, Q)) {

			++c;

			assert(q < inv.size());

			processed.refine(inv[q], Q, GTE);
			processed.insert(q, Q);

//			VATA_LOGGER_INFO(Util::Convert::ToString(q) + ", " + Util::Convert::ToString(*Q));
//			VATA_LOGGER_INFO("processed: " + Util::Convert::ToString(processed.size()) + ", " + "next: " + Util::Convert::ToString(next.size()));

//			VATA_LOGGER_INFO("processed: " + Util::Convert::ToString(processed));

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

//						VATA_LOGGER_INFO("smaller: " + Util::Convert::ToString(*smallerTransition));

						do {

							antichainSet.clear();

							for (auto& biggerTransition : biggerClusterIter->second) {

								assert(biggerTransition);

								if (antichainSet.cached(biggerTransition->state()))
									continue;

								if (!choiceVector.match(biggerTransition->children()))
									continue;

//								VATA_LOGGER_INFO("bigger: " + Util::Convert::ToString(*biggerTransition));

								antichainSet.testAndRefine(biggerTransition->state());

							}
							
							if (antichainSet.empty())
								return false;
	
							if (!antichainSet.isAccepting() && smaller.IsFinalState(smallerTransition->state()))
								return false;
	
							assert(smallerTransition->state() < ind.size());
	
							StateSet tmp(antichainSet.begin(), antichainSet.end());
	
							if (checkIntersection(ind[smallerTransition->state()], tmp))
								continue;
	
//							if (checkIntersection(ind[smallerTransition->state()], antichainSet))
//								continue;
	
							auto ptr = biggerTypeCache.lookup(tmp);
	
//							auto ptr = biggerTypeCache.lookup(antichainSet);
	
							if (!processed.contains(ind[smallerTransition->state()], ptr, LTE) &&
								!next.contains(ind[smallerTransition->state()], ptr, LTE)) {
			
								assert(smallerTransition->state() < inv.size());
	
								next.refine(inv[smallerTransition->state()], ptr, GTE);
								next.insert(smallerTransition->state(), ptr);
	
							}

						} while (choiceVector.next());

					}

					++i;

				}

			}

		}

		VATA_LOGGER_INFO("elements processed: " + Util::Convert::ToString(c));

		return true;

	}

};

#endif
