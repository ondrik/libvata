/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Functor for checking inclusion using antichain algorithm for explicitly
 *	represented finite automata and uses cache and some optimization.
 *
 *****************************************************************************/


#ifndef EXPLICIT_FINITE_AUT_INCL_FCTOR_CACHE_HH_
#define EXPLICIT_FINITE_AUT_INCL_FCTOR_CACHE_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include <vata/util/ordered_antichain2c.hh>
#include <vata/util/antichain1c.hh>

#include "explicit_finite_aut_core.hh"
#include "explicit_finite_abstract_fctor.hh"

#include <vata/finite_aut/util/map_to_list.hh>
#include <vata/finite_aut/util/macrostate_cache.hh>

// standard libraries
#include <vector>
#include <utility>

namespace VATA {
	template <class Rel, class Comparator> class ExplicitFAInclusionFunctorCache;
}

GCC_DIAG_OFF(effc++)
template <class Rel, class Comparator>
class VATA::ExplicitFAInclusionFunctorCache :
	public ExplicitFAAbstractFunctor <Rel> {
GCC_DIAG_ON(effc++)

public : // data types
	typedef ExplicitFAAbstractFunctor<Rel> AbstractFunctor;
	typedef typename AbstractFunctor::ExplicitFA ExplicitFA;

	typedef typename AbstractFunctor::StateType StateType;
	typedef typename AbstractFunctor::StateSet StateSet;

	typedef typename AbstractFunctor::Antichain1Type Antichain1Type;

	typedef StateType SmallerElementType;
	typedef StateSet* BiggerElementType;

	typedef std::pair<SmallerElementType,BiggerElementType> ProductPair;

	struct less { // ordering for ordered antichain
		bool operator()(const ProductPair& p1, const ProductPair& p2) const {
			if (p1.second->size() < p2.second->size()) return true;
			if (p1.second->size() > p2.second->size()) return false;
			if (p1.first < p2.first) return true;
			if (p1.first > p2.first) return false;
			return p1.second < p2.second;
		}
	};

	typedef VATA::Util::Antichain2Cv2<SmallerElementType,BiggerElementType> AntichainType;
	typedef VATA::Util::OrderedAntichain2C<AntichainType,less> AntichainNext;

	typedef AntichainType ProductStateSetType;
	typedef AntichainNext ProductNextType; // todo set is ordered antichain

	typedef typename AbstractFunctor::IndexType IndexType;
	typedef typename VATA::MacroStateCache<ExplicitFA> MacroStateCache;

	// Key is subset of all values
	typedef typename VATA::MapToList<const StateSet*,const StateSet*> SubSetMap;

private: // data memebers
	AntichainType& antichain_;
	AntichainNext& next_;
	Antichain1Type& singleAntichain_;

	const ExplicitFA& smaller_;
	const ExplicitFA& bigger_;

	IndexType& index_;
	IndexType& inv_;

	Rel preorder_; // Simulation or identity

	Comparator comparator_;
	MacroStateCache cache_;
	SubSetMap subsetMap_;
	SubSetMap subsetNotMap_;

public: // constructor
	ExplicitFAInclusionFunctorCache(AntichainType& antichain, AntichainNext& next,
			Antichain1Type& singleAntichain,
			const ExplicitFA& smaller,
			const ExplicitFA& bigger,
			IndexType& index,
			IndexType& inv,
			Rel preorder) :
		antichain_(antichain),
		next_(next),
		singleAntichain_(singleAntichain),
		smaller_(smaller),
		bigger_(bigger),
		index_(index),
		inv_(inv),
		preorder_(preorder),
		comparator_(preorder),
		cache_(),
		subsetMap_(),
		subsetNotMap_()
	{}

public: // public functions

	// Initialize the antichain from init states of given automata
	void Init() {
		bool macroFinal=false;
		StateSet procMacroState;

		int sum = 0;

		// Create macro state of initial states
		for (StateType startState : bigger_.startStates_) {
			procMacroState.insert(startState);
			sum += startState;
			macroFinal |= bigger_.IsStateFinal(startState);
		}

		// Check the initial states
		for (StateType smallState : smaller_.startStates_) {
			this->inclNotHold_ |= smaller_.IsStateFinal(smallState) && !macroFinal;
			StateSet& cachedMacro = cache_.insert(sum,procMacroState);
			this->AddNewPairToAntichain(smallState,cachedMacro);
		}
	}

	/*
	 * Make post set of the product states
	 * for given product state (r,R) and
	 * add states to the antichain
	 */
	void MakePost(StateType procState, BiggerElementType& procMacroState) {

		auto sum = [](StateSet& set, size_t& sum) {for (auto& state : set) sum+=state;};

		auto iteratorSmallerSymbolToState = smaller_.transitions_->find(procState);
		if (iteratorSmallerSymbolToState == smaller_.transitions_->end()) {
			return;
		}
		// Iterate through the all symbols in the transitions for the given state
		for (auto& smallerSymbolToState : *(iteratorSmallerSymbolToState->second)) {
			for (const StateType& newSmallerState : smallerSymbolToState.second) {
				StateSet newMacroState;

				bool IsMacroAccepting = this->CreatePostOfMacroState(
						newMacroState,*procMacroState,smallerSymbolToState.first,
						bigger_);

				size_t macroSum = 0;
				sum(newMacroState,macroSum);
				// insert macrostate to cache
				StateSet& newCachedMacro = cache_.insert(macroSum,newMacroState);

				this->inclNotHold_ |= smaller_.IsStateFinal(newSmallerState) &&
					!IsMacroAccepting;

				if (this->inclNotHold_) {
					return;
				}

				if (!comparator_.checkSmallerInBigger(newSmallerState,newCachedMacro)) {
					this->AddNewPairToAntichain(newSmallerState,newCachedMacro);
				}
			}
		}
	}

private: // private functions
	/*
	 * Add a new product state to the antichains sets
	 */
	void AddNewPairToAntichain(StateType state, StateSet &set) {
		//lss is subset of rss -> return TRUE
		auto lte = [this](const StateSet* lss,
				const StateSet* rss) -> bool {
			bool res;

			// Check whether product state has not been already processed
			if (subsetMap_.contains(lss,rss)) {
				res = true;
			}
			else if (subsetNotMap_.contains(lss,rss)) {
				res = false;
			}
			else { // product state has not been already processed
				res = this->comparator_.lte(*lss,*rss);
				if (res) {
					subsetMap_.add(lss,rss);
					subsetNotMap_.add(rss,lss);
				}
				else {
					subsetMap_.add(rss,lss);
					subsetNotMap_.add(lss,rss);
				}
			}
			return res;
		};

		// lss is greater then rss -> return TRUE
		auto gte = [this](const StateSet* lss, const StateSet* rss) -> bool {
			bool res;

			if (subsetMap_.contains(lss,rss)) {
				res = false;
			}
			else if (subsetNotMap_.contains(lss,rss)) {
				res = true;
			}
			else {
				res = this->comparator_.gte(*lss,*rss);
				if (res) {
					subsetMap_.add(rss,lss);
					subsetNotMap_.add(lss,rss);
				}
				else {
					subsetMap_.add(lss,rss); // rss is subset of lss
					subsetNotMap_.add(rss,lss);
				}
			}
			return res;
		};

		// Check whether the antichain does not already
		// contains given product state
		std::vector<StateType> candidates;
		// Get candidates for given state
		comparator_.getCandidate(candidates,state,singleAntichain_);
		if (!antichain_.contains(candidates,&set,lte)) {
			std::vector<StateType> candidates;
			comparator_.getCandidateRev(candidates,state,singleAntichain_);
			antichain_.refine(candidates,&set,gte);
			antichain_.insert(state,&set);
			AddToSingleAC(state);
			AddToNext(state,set);
		}
	}

	/*
	 * Add state to the small antichain
	 */
	void AddToSingleAC(StateType state) {
		std::vector<StateType> tempStateSet = {state};
		if (!singleAntichain_.contains(tempStateSet)) {
			singleAntichain_.insert(state);
		}
	}

	/*
	 * Add product state to the next set
	 */
	void AddToNext(StateType state, StateSet& set) {
		//lss is subset of rss -> return TRUE
		auto lte = [this](const StateSet* lss,
				const StateSet* rss) -> bool {
			bool res;

			// Check whether product state has not been already processed
			if (subsetMap_.contains(lss,rss)) {
				res = true;
			}
			else if (subsetNotMap_.contains(lss,rss)) {
				res = false;
			}
			else {
				res = this->comparator_.lte(*lss,*rss);
				if (res) {
					subsetMap_.add(lss,rss);
					subsetNotMap_.add(rss,lss);
				}
				else {
					subsetMap_.add(rss,lss);
					subsetNotMap_.add(lss,rss);
				}
			}
			return res;
		};

		// lss is greater then rss -> return TRUE
		auto gte = [this](const StateSet* lss, const StateSet* rss) -> bool {
			bool res;

			if (subsetMap_.contains(lss,rss)) {
				res = false;
			}
			else if (subsetNotMap_.contains(lss,rss)) {
				res = true;
			}
			else {
				res = this->comparator_.gte(*lss,*rss);
				if (res) {
					subsetMap_.add(rss,lss);
					subsetNotMap_.add(lss,rss);
				}
				else {
					subsetMap_.add(lss,rss); // rss is subset of lss
					subsetNotMap_.add(rss,lss);
				}
			}
			return res;
		};

		std::vector<StateType> tempStateSet;// = {state};
		comparator_.getCandidate(tempStateSet,state,singleAntichain_);
		if (!next_.contains(tempStateSet,&set,lte)) {
			std::vector<StateType> tempStateSet;// = {state};
			comparator_.getCandidateRev(tempStateSet,state,singleAntichain_);
			next_.refine(tempStateSet,&set,gte);
			next_.insert(state,&set);
		}
	}


private: // Private inline functions
	/*
	 * Copy one set to another
	 */
	inline void CopySet(StateSet& fullset, StateSet& emptyset) {
		emptyset.insert(fullset.begin(),fullset.end());
	}

};

#endif
