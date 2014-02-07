/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Header file for abstract functor for inlcusion checking
 *	which contains functions shared by functors for antichains and
 *	congruence algorithm.
 *
 *****************************************************************************/

#ifndef EXPLICIT_FINITE_AUT_ABSTRACT_FCTOR_HH_
#define EXPLICIT_FINITE_AUT_ABSTRACT_FCTOR_HH_

// VATA headers
#include <vata/vata.hh>

#include "explicit_finite_aut_core.hh"

#include <vata/util/antichain1c.hh>

namespace VATA {
	template <class Rel> class ExplicitFAAbstractFunctor;
}


GCC_DIAG_OFF(effc++)
template <class Rel>
class VATA::ExplicitFAAbstractFunctor {
GCC_DIAG_ON(effc++)
public: // data types
	typedef VATA::ExplicitFiniteAutCore ExplicitFA;

	typedef typename ExplicitFA::StateType StateType;
	typedef typename ExplicitFA::StateSet StateSet;
	typedef typename ExplicitFA::SymbolType SymbolType;

	// Define single antichain
	typedef VATA::Util::Antichain1C<StateType> Antichain1Type;

	typedef typename Rel::IndexType IndexType;

protected: // data memebers

	bool inclNotHold_;

public:

	ExplicitFAAbstractFunctor() : inclNotHold_(false) {}

protected:

	/*
	 * Create a new post macrostate of the given
	 * macrostate (procMacroState) for given symbol (symbol) in FA in parameter macroFA.
	 * New macrostate is stored to the given StateSet newMacroState.
	 * @Return True if created macrostates is final in bigger NFA
	 */
	bool CreatePostOfMacroState(StateSet& newMacroState,
			const StateSet& procMacroState, const SymbolType& symbol,
			const ExplicitFA& macroFA) {

		bool res = false;
		// Create new macro state from current macro state for given symbol
		for (const StateType& stateInMacro : procMacroState) {

			// Find transition for given state
			auto iteratorTransForState = macroFA.transitions_->find(stateInMacro);
			if (iteratorTransForState == macroFA.transitions_->end()) {
				continue;
			}
			auto transForState = iteratorTransForState->second;

			// States for given symbol
			auto symbolToStateSet = transForState->find(symbol);
			if (symbolToStateSet == transForState->end()) {
				continue;
			}

			// The states are added to created macostate and at the
			// same time is checked	 whether they are final or not
			for (auto& s : symbolToStateSet->second) {
				newMacroState.insert(s);
				res |= macroFA.IsStateFinal(s);
			}
		}
		return res;
	}

	/*
	 * Create macrostate as the previous function but also
	 * count the sum of all states of the new macrostate
	 */
	bool CreatePostOfMacroStateWithSum(StateSet& newMacroState,
			const StateSet& procMacroState, const SymbolType& symbol,
			const ExplicitFA& macroFA, size_t& sum) {

		bool res = false;
		// Create new macro state from current macro state for given symbol
		for (const StateType& stateInMacro : procMacroState) {

			// Find transition for given state
			auto iteratorTransForState = macroFA.transitions_->find(stateInMacro);
			if (iteratorTransForState == macroFA.transitions_->end()) {
				continue;
			}
			auto transForState = iteratorTransForState->second;

			// States for given symbol
			auto symbolToStateSet = transForState->find(symbol);
			if (symbolToStateSet == transForState->end()) {
				continue;
			}

			for (auto& s : symbolToStateSet->second) {
				newMacroState.insert(s);
				sum += s;
				res |= macroFA.IsStateFinal(s);
			}
		}
		return res;
	}

	/*
	 * Just print a macrostate
	 */
	void macroPrint(StateSet& set) {
			for (auto& s : set) {
				std::cerr << s << " ";
			}
			std::cerr << std::endl;
	}

public: // Public inline functions
	/*
	 * Return true if inclusion hold
	 */
	inline bool DoesInclusionHold() {
		return !inclNotHold_;
	}


};

#endif
