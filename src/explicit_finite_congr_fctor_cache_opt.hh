/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Functor for checking inclusion using congruence algorithm for explicitly
 *	represented finite automata. Functor works with cache and some other
 *	optimization.
 *
 *****************************************************************************/

#ifndef EXPLICIT_FINITE_AUT_CONGR_INCL_HH_
#define EXPLICIT_FINITE_AUT_CONGR_INCL_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>

#include "explicit_finite_abstract_fctor.hh"
#include <vata/finite_aut/util/map_to_list.hh>
#include <vata/finite_aut/util/macrostate_cache.hh>

namespace VATA {
	template <class Rel, class ProductSet, class NormalFormRel> class ExplicitFACongrFunctorCacheOpt;
}

GCC_DIAG_OFF(effc++)
template <class Rel, class ProductSet, class NormalFormRel>
class VATA::ExplicitFACongrFunctorCacheOpt :
	public ExplicitFAAbstractFunctor <Rel> {
GCC_DIAG_ON(effc++)

public : // data types
	typedef typename VATA::ExplicitFAAbstractFunctor<Rel>
		AbstractFunctor;
	typedef typename AbstractFunctor::ExplicitFA ExplicitFA;

	typedef typename AbstractFunctor::StateType StateType;
	typedef typename AbstractFunctor::StateSet StateSet;
	typedef typename AbstractFunctor::SymbolType SymbolType;

	typedef typename AbstractFunctor::Antichain1Type Antichain1Type;

	typedef std::unordered_set<SymbolType> SymbolSet;

	/*
	 * In the both automata are explored macrostates
	 */
	typedef StateSet SmallerElementType;
	typedef StateSet BiggerElementType;

	typedef std::unordered_map<size_t,StateSet> CongrMap;
	typedef std::pair<SmallerElementType*,BiggerElementType*> ProductState;

	/*
	 * Product state of built automaton is pair of macrostates
	 */
	/*
GCC_DIAG_OFF(effc++)
	class ProductStateSetType : public std::vector<ProductState> {
GCC_DIAG_ON(effc++)
		public:
		bool get(SmallerElementType& smaller, BiggerElementType& bigger) {
			if (this->size() == 0) {
				return false;
			}

			auto& nextPair = this->back();
			smaller = *nextPair.first;
			bigger = *nextPair.second;
			this->pop_back();

			return true;
		}
	};
*/
	typedef ProductSet ProductStateSetType;
	// todo set is the same as the processed set of product states
	typedef ProductStateSetType ProductNextType;

	typedef typename VATA::MacroStateCache<ExplicitFA> MacroStateCache;
	typedef typename VATA::MapToList<StateSet*,StateSet*> MacroStatePtrPair;

	typedef typename AbstractFunctor::IndexType IndexType;

private: // Private data members
	ProductStateSetType& relation_;
	ProductStateSetType&	next_;
	Antichain1Type& singleAntichain_; // just for compability with the antichain functor

	const ExplicitFA& smaller_;
	const ExplicitFA& bigger_;

	IndexType& index_;
	IndexType& inv_;

	NormalFormRel normalFormRel_;

	MacroStateCache cache_;
	MacroStatePtrPair visitedPairs_;
	MacroStatePtrPair usedRules_;

public:
	ExplicitFACongrFunctorCacheOpt(ProductStateSetType& relation, ProductStateSetType& next,
			Antichain1Type& singleAntichain,
			const ExplicitFA& smaller,
			const ExplicitFA& bigger,
			IndexType& index,
			IndexType& inv,
			Rel preorder) :
		relation_(relation),
		next_(next),
		singleAntichain_(singleAntichain),
		smaller_(smaller),
		bigger_(bigger),
		index_(index),
		inv_(inv),
		normalFormRel_(preorder),
		cache_(),
		visitedPairs_(),
		usedRules_()
	{}

public: // public functions

	/*
	 * The first product state of built automaton is
	 * pair of macrostates containing the initial states
	 * of both input NFA.
	 */
	void Init() {
		StateSet smallerInit;
		StateSet biggerInit;

		// Check whether the new macrostates are final
		bool smallerInitFinal = false;
		bool biggerInitFinal = false;

		// Created macrostate of smaller automaton
		size_t smallerHashNum = 0;
		for (auto state : smaller_.startStates_) {
			smallerHashNum += state;
			smallerInit.insert(state);
			smallerInitFinal |= smaller_.IsStateFinal(state);
		}

		// Created macrostate of bigger automaton
		size_t biggerHashNum = 0;
		for (auto state : bigger_.startStates_) {
			biggerHashNum += state;
			biggerInit.insert(state);
			biggerInitFinal |= bigger_.IsStateFinal(state);
		}

		// Add states to the cache
		StateSet& insertSmaller = cache_.insert(smallerHashNum,smallerInit);
		StateSet& insertBigger = cache_.insert(biggerHashNum,biggerInit);
		// Add to todo set
		next_.push_back(std::make_pair(&insertSmaller,&insertBigger));
		visitedPairs_.add(&insertSmaller,&insertBigger);
		this->inclNotHold_ = smallerInitFinal != biggerInitFinal;
	};

	/*
	 * Make post of given macrostates of the both NFA
	 */
	void MakePost(SmallerElementType& smaller, BiggerElementType& bigger) {
		SymbolSet usedSymbols;

		// Function checks whether macrostates are equal
		auto isSubSet = [] (StateSet& lss, StateSet& rss) -> bool {
			if (lss.size() > rss.size()) {
				return false;
			}
			for (auto& ls : lss) {
				if (!rss.count(ls)) {
					return false;
				}
			}

			return true;
		};


		auto sum = [](StateSet& set, size_t& sum) {for (auto& state : set) sum+=state;};
		size_t smallerHashNum = 0;
		sum(smaller,smallerHashNum);
		size_t biggerHashNum = 0;
		sum(bigger,biggerHashNum);
		SmallerElementType& s = cache_.insert(smallerHashNum,smaller);
		BiggerElementType& b = cache_.insert(biggerHashNum,bigger);


		// Comapring given set with the sets
		// which has been computed in steps of computation of congr closure
		auto isCongrClosureSet = [&s,&isSubSet](StateSet& bigger) ->
			bool {
				return !isSubSet(s,bigger);
		};

		// Compute congruence closure of bigger nfa
		StateSet congrBigger(bigger);

		normalFormRel_.applyRule(congrBigger);

		// Checks whether smaller macrostate is subset of congr. clusure of bigger
		if (GetCongrClosure(b,congrBigger,isCongrClosureSet) ||
			isSubSet(s,congrBigger)) {
			smaller.clear();
			bigger.clear();
			return;
		}

		// Create post macrostates
		MakePostForAut(smaller_,usedSymbols,smaller,bigger,smaller);
		if (this->inclNotHold_) {
			return;
		}
		MakePostForAut(bigger_,usedSymbols,smaller,bigger,bigger);

		relation_.push_back(std::make_pair(&s,&b));

		smaller.clear();
		bigger.clear();
	};

private:

	// Check if the rule is applyable
	bool MatchPair(const StateSet& closure, const StateSet& rule) {
		if (rule.size() > closure.size()) {
				return false;
		}
		for (auto& s : rule) {
			if (!closure.count(s)) {
				return false;
		 }
		}
		return true;
	}

	void AddSubSet(StateSet& mainset, StateSet& subset) {
		StateSet temp = StateSet(subset);
		normalFormRel_.applyRule(temp);
		mainset.insert(temp.begin(),temp.end());
		//mainset.insert(subset.begin(),subset.end());
	}

	/*
	 * Apply all possible rules for given relation
	 * when the congruence closure of the given macrostate
	 * has not been computed.
	 * @param origSet set for which is congr. closure computed
	 * @param set Set where congr closure is stored
	 * @param congrMapManipulator Checks	on the fly if the (X,Y) in c(R) does not hold
	 * @param usedRulesNumber Used rules for given relation
	 * @param appliedRules signify whether some rule has been applied
	 * @param relation Relation of processed states
	 */
	template<class CongrMapManipulator>
	bool ApplyRulesForRelation(StateSet& origSet, StateSet& set,
		ProductStateSetType& relation, CongrMapManipulator& congrMapManipulator,
		std::unordered_set<int>& usedRulesNumbers,
		bool& appliedRule) {

	 for (unsigned int i=0; i < relation.size(); i++) { // all items in relation
		 if (usedRulesNumbers.count(i)) { // already used rule
			 continue;
		 }
		 if (MatchPair(set, *relation[i].second)) { // Rule matched
			 AddSubSet(set,*relation[i].first);
			 AddSubSet(set,*relation[i].second);
			 usedRules_.add(&origSet,relation[i].second); // Stores applied rules
			 usedRulesNumbers.insert(i);
			 appliedRule = true;
			 if (!congrMapManipulator(set)) {
				 return true;
			 }
		 }
		}
		return false;
	}

	/*
	 * Apply all possible rules for given relation
	 * when the congruence closure of the given macrostate
	 * has been computed.
	 * @param origSet set for which is congr. closure computed
	 * @param set Set where congr closure is stored
	 * @param congrMapManipulator Checks	on the fly if the (X,Y) in c(R) does not hold
	 * @param usedRulesNumber Used rules for given relation
	 * @param appliedRules signify whether some rule has been applied
	 * @param relation Relation of processed states
	 */
	template<class CongrMapManipulator>
	bool ApplyRulesForRelationVisited(StateSet& origSet, StateSet& set,
		ProductStateSetType& relation, CongrMapManipulator& congrMapManipulator,
		std::unordered_set<int>& usedRulesNumbers,
		bool& appliedRule) {

		for (unsigned int i=0; i < relation.size(); i++) { // relation next
			if (usedRulesNumbers.count(i)) { // already used rule
				continue;
		 	}
		 	if (usedRules_.contains(&origSet,relation[i].second) ||
				MatchPair(set, *relation[i].second)) { // Rule matches

				AddSubSet(set,*relation[i].first);
			 	AddSubSet(set,*relation[i].second);
			 	usedRulesNumbers.insert(i);
			 	appliedRule = true;
			 	if (!congrMapManipulator(set)) {
					return true;
			 	}
		 	}
		}
		return false;
	}

	/*
	 * Compute congruence closure for given set
	 * @param origSet set for which is congr. closure computed
	 * @param set Set where congr closure is stored
	 * @param usedRulesNumberN Used rules in todo relation
	 * @param usedRulesNumberR Used rules in processed relation
	 * @param congrMapManipulator Checks	on the fly if the (X,Y) in c(R) does not hold
	 */
	template<class CongrMapManipulator>
	bool GetCongrClosure(StateSet& origSet,StateSet& set, CongrMapManipulator& congrMapManipulator) {
		std::unordered_set<int> usedRulesNumbersN;
		std::unordered_set<int> usedRulesNumbersR;

		bool appliedRule = true;
		bool visited = usedRules_.containsKey(&origSet);

		if (!visited) { // congr. closure for the macrostate has been computed
			while (appliedRule) { // Apply all possible rules
				appliedRule = false;

				if (ApplyRulesForRelation( //apply rules for next relation
					origSet,set,next_,congrMapManipulator,
					usedRulesNumbersN,appliedRule)) {
					return true;
				}
				// apply rules for proccesed relation
				if (ApplyRulesForRelation(origSet,set,relation_,
						congrMapManipulator,usedRulesNumbersR,appliedRule)) {
					return true;
				}
			}
		}
		else { // congr. closure computed first time
			while (appliedRule) { // Macrostate allready visited
				appliedRule = false;

				if (ApplyRulesForRelationVisited(
					origSet,set,next_,congrMapManipulator,usedRulesNumbersN,appliedRule)){
					return true;
				}
				// apply rules for proccesed relation
				if (ApplyRulesForRelationVisited(
					origSet,set,relation_,congrMapManipulator,usedRulesNumbersR,appliedRule)) {
					return true;
				}
			}
		}
		return false;
	}


	/*
	 * Create post macrostates for given macrostate (actStateSet)
	 * for all possible symbols.
	 */
	void MakePostForAut(const ExplicitFA& aut, SymbolSet& usedSymbols,
			const SmallerElementType& smaller, const BiggerElementType& bigger,
			const StateSet& actStateSet) {

		for (auto& state : actStateSet) {// for each state in processed macrostate
			auto transIter = aut.transitions_->find(state);
			if (transIter == aut.transitions_->end()) {
				continue;
			}

			// For all symbols accesible by the state
			for (auto& symbolToSet : *transIter->second) {
				if (usedSymbols.count(symbolToSet.first)) { // symbol already explored
					continue;
				}

				usedSymbols.insert(symbolToSet.first);
				SmallerElementType newSmaller;
				BiggerElementType newBigger;

				// all states accesible under given symbol for in smaller nfa
				bool newSmallerAccept =
					this->CreatePostOfMacroState(
							newSmaller,smaller,symbolToSet.first,smaller_);

				// all states accesible under given symbol for in bigger nfa
				bool newBiggerAccpet =
					this->CreatePostOfMacroState(
							newBigger,bigger,symbolToSet.first,bigger_);

				if (newSmallerAccept != newBiggerAccpet) {
					this->inclNotHold_ = true;
					return;
				}

				/*
				 * New macrostates of product state are added to cache nad then
				 * the produc state to todo set if it has not been already explored
				 */
				if (newSmaller.size() || newBigger.size()) {
					size_t smallerHashNum = 0;
					size_t biggerHashNum = 0;
					auto sum = [](StateSet& set, size_t& sum) {for (auto& state : set) sum+=state;};
					sum(newSmaller,smallerHashNum);
					sum(newBigger,biggerHashNum);
					StateSet& insertSmaller = cache_.insert(smallerHashNum,newSmaller);
					StateSet& insertBigger = cache_.insert(biggerHashNum,newBigger);
					if (!visitedPairs_.contains(&insertSmaller,&insertBigger)){
						visitedPairs_.add(&insertSmaller,&insertBigger);
						next_.add(insertSmaller,insertBigger);
						//next_.push_back(std::make_pair(&insertSmaller,&insertBigger));
					  //next_.insert(next_.begin(),std::make_pair(&insertSmaller,&insertBigger));
					}
				}
			}
		}
	}
};
#endif
