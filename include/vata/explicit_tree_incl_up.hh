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


public:

	template <class Aut, class Rel>
	static bool Check(const Aut& smaller, const Aut& bigger, const Rel& preorder) {

		typedef Explicit::StateType SmallerType;
		typedef std::vector<Explicit::StateType> BiggerType;
		typedef typename Aut::TransitionPtr TransitionPtr;

		GCC_DIAG_OFF(effc++)
		struct Choice {
		GCC_DIAG_ON(effc++)

			const Util::Antichain2Cv2<SmallerType, BiggerType>::TList* biggerList_;
			Util::Antichain2Cv2<SmallerType, BiggerType>::TList::const_iterator current_;

			bool init(const Util::Antichain2Cv2<SmallerType, BiggerType>::TList* biggerList) {

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

			const Util::Antichain2Cv2<SmallerType, BiggerType>& processed_;
			const Util::Antichain2Cv2<SmallerType, BiggerType>::TList& fixed_;
			std::vector<Choice> state_;
			
		public:

			ChoiceVector(const Util::Antichain2Cv2<SmallerType, BiggerType>& processed,
				const Util::Antichain2Cv2<SmallerType, BiggerType>::TList& fixed)
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
			
			bool match(const Explicit::StateTuple& children) {

				for (size_t i = 0; i < children.size(); ++i) {

					assert(i < this->state_.size());

					auto& s = *this->state_[i].current_;

					if (!std::binary_search(s.begin(), s.end(), children[i]))
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

		public:

			AntichainSet(const Aut& aut, const typename Rel::IndexType& ind,
				const typename Rel::IndexType& inv) : aut_(aut), ind_(ind), inv_(inv),
				isAccepting_(false) {}

			bool isAccepting() const { return this->isAccepting_; }

			void testAndRefine(const Explicit::StateType& state) {

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
		
		};

		typename Rel::IndexType ind, inv;

		preorder.buildIndex(ind, inv);

		auto LTE = [&ind](const BiggerType& x, const BiggerType& y) -> bool {

			for (auto& s1 : x) {

				assert(s1 < ind.size());

				if (!checkIntersection(ind[s1], y)) {
//					VATA_LOGGER_INFO(Util::Convert::ToString(x) + " > " + Util::Convert::ToString(y));
					return false;
				}

			}

//			VATA_LOGGER_INFO(Util::Convert::ToString(x) + " <= " + Util::Convert::ToString(y));

			return true;

		};
		
		auto GTE = [&LTE](const BiggerType& x, const BiggerType& y) { return LTE(y, x); };

		typename Aut::SymbolToTransitionListMap smallerLeaves;
		typename Aut::BottomUpIndex smallerIndex;
		typename Aut::SimpleBottomUpIndex biggerIndex;

		smaller.bottomUpIndex(smallerIndex, smallerLeaves);
		bigger.bottomUpIndex(biggerIndex);

		Util::Antichain2Cv2<SmallerType, BiggerType> next, processed;

		// Post(\emptyset)

		for (auto& smallerCluster : smallerLeaves) {

			auto biggerClusterIter = biggerIndex.find(smallerCluster.first);

			if (biggerClusterIter == biggerIndex.end())
				return false;

			std::vector<Explicit::StateType> tmp;

			for (auto& transition : biggerClusterIter->second) {

				assert(transition);
				assert(transition->children().empty());

//				VATA_LOGGER_INFO("bigger: " + Util::Convert::ToString(*transition));

				tmp.push_back(transition->state());

			}

			std::sort(tmp.begin(), tmp.end());

			tmp.resize(std::unique(tmp.begin(), tmp.end()) - tmp.begin());

			bool isAccepting = checkIntersection(tmp, bigger.GetFinalStates());

			for (auto& transition : smallerCluster.second) {

				assert(transition);

//				VATA_LOGGER_INFO("smaller: " + Util::Convert::ToString(*transition));

				if (!isAccepting && smaller.IsFinalState(transition->state()))
					return false;

				assert(transition->state() < ind.size());

				if (checkIntersection(ind[transition->state()], tmp))
					continue;

				if (!next.contains(ind[transition->state()], tmp, LTE)) {

					assert(transition->state() < inv.size());
					
					next.refine(inv[transition->state()], tmp, GTE);
					next.insert(transition->state(), tmp);

				}

			}

		}

		SmallerType q;

		Util::Antichain2Cv2<SmallerType, BiggerType>::TList fixedList(1);

		BiggerType& Q = fixedList.front();

		ChoiceVector choiceVector(processed, fixedList);

//		VATA_LOGGER_INFO("next: " + Util::Convert::ToString(next));

		while (next.next(q, Q)) {

			assert(q < ind.size());

			if (processed.contains(ind[q], Q, LTE))
				continue;

			assert(q < inv.size());

			processed.refine(inv[q], Q, GTE);
			processed.insert(q, Q);

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

						AntichainSet antichainSet(bigger, ind, inv);

						do {

							for (auto& biggerTransition : biggerClusterIter->second) {

								assert(biggerTransition);

								if (!choiceVector.match(biggerTransition->children()))
									continue;

//								VATA_LOGGER_INFO("bigger: " + Util::Convert::ToString(*biggerTransition));

								antichainSet.testAndRefine(biggerTransition->state());

							}
							
						} while (choiceVector.next());

						if (antichainSet.empty())
							return false;

						if (!antichainSet.isAccepting() && smaller.IsFinalState(smallerTransition->state()))
							return false;

						BiggerType tmp(antichainSet.begin(), antichainSet.end());

						assert(smallerTransition->state() < ind.size());

						if (checkIntersection(ind[smallerTransition->state()], tmp))
							continue;

						if (!processed.contains(ind[smallerTransition->state()], tmp, LTE) &&
							!next.contains(ind[smallerTransition->state()], tmp, LTE)) {
		
							assert(smallerTransition->state() < inv.size());

							processed.refine(inv[smallerTransition->state()], tmp, GTE);
							next.refine(inv[smallerTransition->state()], tmp, GTE);

							next.insert(smallerTransition->state(), tmp);

						}

					}

					++i;

				}

			}

		}

		return true;

	}

};

#endif
