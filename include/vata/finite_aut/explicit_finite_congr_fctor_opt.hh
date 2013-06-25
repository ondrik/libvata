/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Functor for checking inclusion using congruence algorithm for explicitly
 *	represented finite automata. Functor works with some optimization.
 *
 *****************************************************************************/


#ifndef EXPLICIT_FINITE_AUT_CONGR_FCTOR_OPT_
#define EXPLICIT_FINITE_AUT_CONGR_FCTOR_OPT_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>

#include <vata/finite_aut/explicit_finite_aut.hh>
#include <vata/finite_aut/explicit_finite_abstract_fctor.hh>

namespace VATA {
	template <class SymbolType, class Rel> class ExplicitFACongrFunctorOpt;
}

GCC_DIAG_OFF(effc++)
template <class SymbolType, class Rel>
class VATA::ExplicitFACongrFunctorOpt :
	public ExplicitFAAbstractFunctor <SymbolType,Rel> {
GCC_DIAG_ON(effc++)

public : // data types
	typedef typename VATA::ExplicitFAAbstractFunctor<SymbolType,Rel>
		AbstractFunctor;
	typedef typename AbstractFunctor::ExplicitFA ExplicitFA;

	typedef typename AbstractFunctor::StateType StateType;
	typedef typename AbstractFunctor::StateSet StateSet;

	typedef typename AbstractFunctor::Antichain1Type Antichain1Type;

	typedef std::unordered_set<SymbolType> SymbolSet;

	/*
	 * In the both automata are explored macrostates
	 */
	typedef StateSet SmallerElementType;
	typedef StateSet BiggerElementType;

	typedef std::unordered_map<size_t,StateSet> CongrMap;

	/*
	 * Product state of built automaton is pair of macrostates
	 */
	class ProductStateSetType : public std::vector<std::pair<SmallerElementType,BiggerElementType>> {
		public:
		bool get(SmallerElementType& smaller, BiggerElementType& bigger) {
			if (this->size() == 0) {
				return false;
			}

			auto& nextPair = this->back();
			smaller = nextPair.first;
			bigger = nextPair.second;
			this->pop_back();

			return true;
		}
	};

	// todo set is the same as the processed set of product states
	typedef ProductStateSetType ProductNextType;

	typedef typename AbstractFunctor::IndexType IndexType;

private: // Private data members
	ProductStateSetType& relation_;
	ProductStateSetType&	next_;
	Antichain1Type& singleAntichain_;

	const ExplicitFA& smaller_;
	const ExplicitFA& bigger_;

	IndexType& index_;
	IndexType& inv_;

	Rel preorder_; // Simulation or identity

public:
	ExplicitFACongrFunctorOpt(ProductStateSetType& relation, ProductStateSetType& next,
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
		preorder_(preorder)
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
		for (auto state : smaller_.startStates_) {
			smallerInit.insert(state);
			smallerInitFinal |= smaller_.IsStateFinal(state);
		}

		// Created macrostate of bigger automaton
		for (auto state : bigger_.startStates_) {
			biggerInit.insert(state);
			biggerInitFinal |= bigger_.IsStateFinal(state);
		}

		next_.push_back(std::make_pair(StateSet(smallerInit),biggerInit));
		this->inclNotHold_ = smallerInitFinal != biggerInitFinal;
	};

	/*
	 * Make post of given macrostates of the both NFA
	 */
	void MakePost(SmallerElementType& smaller, BiggerElementType& bigger) {
		SymbolSet usedSymbols;

		// Function checks whether macrostates are equal
		auto areEqual = [] (StateSet& lss, StateSet& rss) -> bool {
			if (lss.size() != rss.size()) {
				return false;
			}
			if (!lss.size() || !rss.size()) {
				return false;
			}
			for (auto& ls : lss) {
				if (!rss.count(ls)) {
					return false;
				}
			}

			return true;
		};

		CongrMap congrMap; // Map applied rule to the state of congruence closure
		auto insertNewPair = [&congrMap](size_t i, StateSet& set) -> bool {
			congrMap.insert(std::make_pair(i,StateSet(set)));
			return true;
		};
		StateSet congrSmaller(smaller);
		// Compute the whole congruence closure of smaller macrostate
		GetCongrClosure(congrSmaller,insertNewPair);

		// Comapring given set with the sets
		// which has been computed in steps of computation of congr closure
		auto isCongrClosureSetNew = [&congrMap,&areEqual](size_t i, StateSet& set) ->
			bool {
				return !areEqual(congrMap[i],set);
		};

		// Compute congruence closure and on the fly checks
		// if the sets are not equal under or two steps
		StateSet congrBigger(bigger);
		if (GetCongrClosure(congrBigger,isCongrClosureSetNew)) {
			smaller.clear();
			bigger.clear();
			return;
		}

		// Create post macrstates
		MakePostForAut(smaller_,usedSymbols,smaller,bigger,smaller);
		if (this->inclNotHold_) {
			return;
		}
		MakePostForAut(bigger_,usedSymbols,smaller,bigger,bigger);

		relation_.push_back(std::make_pair(smaller,StateSet(bigger)));
		smaller.clear();
		bigger.clear();
	};

private:
	/*
	 * Match two sets to be able to apply rewriting rule
	 */
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
		mainset.insert(subset.begin(),subset.end());
	}

	/*
	 * Compute congruence closure. It is parameterized function
	 * where parameter manipulate with currently computed congruence closure.
	 * So it mapped the rule to actual state of congruence closure or
	 * check whether congruence closures are not same
	 */
	template<class CongrMapManipulator>
	bool GetCongrClosure(StateSet& set, CongrMapManipulator& congrMap) {
		std::unordered_set<int> usedRulesN;
		std::unordered_set<int> usedRulesR;

		bool appliedRule = true;
		while (appliedRule) { // apply all rules
			appliedRule = false;
			for (unsigned int i=0; i < next_.size(); i++) {
				if (usedRulesN.count(i)) {
				 continue;
				}

				if (MatchPair(set, next_[i].first) ||
						 MatchPair(set, next_[i].second)) { // Rule matches
					AddSubSet(set,next_[i].first);
					AddSubSet(set,next_[i].second);
					usedRulesN.insert(i);
					appliedRule = true;
					congrMap(i,set);
				}
			}
			for (unsigned int i=0; i < relation_.size(); i++) {

				if (usedRulesR.count(i)) {
				 continue;
				}
				if (MatchPair(set, relation_[i].first) ||
						 MatchPair(set, relation_[i].second)) { // Rule matches
					AddSubSet(set,relation_[i].first);
					AddSubSet(set,relation_[i].second);
					usedRulesR.insert(i);
					appliedRule = true;
					if (!congrMap(next_.size()+i,set)) {
					 return true;
					}
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

				if (newSmaller.size() || newBigger.size()) {
					next_.push_back(std::make_pair(newSmaller,newBigger));
				}
			}
		}
	}
};

#endif
