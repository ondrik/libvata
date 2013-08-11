/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Functor for checking inclusion using congruence algorithm for explicitly
 *	represented finite automata.
 *
 *****************************************************************************/


#ifndef EXPLICIT_FINITE_AUT_CONGR_FCTOR_
#define EXPLICIT_FINITE_AUT_CONGR_FCTOR_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>

#include <vata/finite_aut/explicit_finite_aut.hh>
#include <vata/finite_aut/explicit_finite_abstract_fctor.hh>

namespace VATA {
	template <class Rel> class ExplicitFACongrFunctor;
}

GCC_DIAG_OFF(effc++) // non virtual destructors warnings supress
template <class Rel>
class VATA::ExplicitFACongrFunctor :
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
	// Constructor
	ExplicitFACongrFunctor(ProductStateSetType& relation, ProductStateSetType& next,
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

		// Add to todo set
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

		// Congruence closure of macrostate of smaller NFA
		StateSet congrSmaller(smaller);
		GetCongrClosure(congrSmaller);

		// Congruence closure of macrostate of smaller NFA
		StateSet congrBigger(bigger);
		GetCongrClosure(congrBigger);

		/*
		 * Checks whether product state is not in congruence
		 * closure of NFA
		 */
		if (areEqual(congrSmaller,congrBigger)) {
			smaller.clear();
			bigger.clear();
			return;
		}

		// Make post of macrostate of smaller NFA
		MakePostForAut(smaller_,usedSymbols,smaller,bigger,smaller);
		if (this->inclNotHold_) {
			return;
		}
		// Make post of macrostate of bigger NFA
		MakePostForAut(bigger_,usedSymbols,smaller,bigger,bigger);

		// Add to relation of processed states
		relation_.push_back(std::make_pair(smaller,StateSet(bigger)));
		smaller.clear();
		bigger.clear();
	};

private:

	/*
	 * Compute congruence closure of using rewriting rules
	 */
	void GetCongrClosure(StateSet& set) {
		auto matchPair = [](const StateSet& closure, const StateSet& rule) -> bool {
			if (rule.size() > closure.size()) {
				return false;
			}
			for (auto& s : rule) {
				if (!closure.count(s)) {
					return false;
				}
			}
			return true;
		};

		/*
		 * Add one set to another set
		 */
		auto addSubSet = [](StateSet& mainset, StateSet& subset) -> void {
			mainset.insert(subset.begin(),subset.end());
		};

		/*
		 * List of allready used rules
		 */
		std::unordered_set<int> usedRulesN;
		std::unordered_set<int> usedRulesR;

		bool appliedRule = true;
		while (appliedRule) { // until all possible rules are applied
			appliedRule = false;

			// Rules in todo set of product states
			for (unsigned int i=0; i < next_.size(); i++) {

			 if (usedRulesN.count(i)) {
				 continue;
			 }

			 if (matchPair(set, next_[i].first) ||
						 matchPair(set, next_[i].second)) { // Rule is possible to apply
				 addSubSet(set,next_[i].first);
				 addSubSet(set,next_[i].second);
				 usedRulesN.insert(i);
				 appliedRule = true;
			 }
			}
			/*
			 * Iterate through all rules in processed set
			 */
			for (unsigned int i=0; i < relation_.size(); i++) {

				 if (usedRulesR.count(i)) {
					 continue;
				 }

				 if (matchPair(set, relation_[i].first) ||
							 matchPair(set, relation_[i].second)) { // appliable rules
					addSubSet(set,relation_[i].first);
					addSubSet(set,relation_[i].second);
					usedRulesR.insert(i);
					appliedRule = true;
				}
			}
		}
	}

	/*
	 * Create post macrostates for given macrostate (actStateSet)
	 * for all possible symbols.
	 */
	void MakePostForAut(const ExplicitFA& aut, SymbolSet& usedSymbols,
			const SmallerElementType& smaller, const BiggerElementType& bigger,
			const StateSet& actStateSet) {

		for (auto& state : actStateSet) { // for each state in processed macrostate
			auto transIter = aut.transitions_->find(state);
			if (transIter == aut.transitions_->end()) {
				continue;
			}

			// For all symbols accesible by the state
			for (auto& symbolToSet : *transIter->second) {
				if (usedSymbols.count(symbolToSet.first)) {
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
