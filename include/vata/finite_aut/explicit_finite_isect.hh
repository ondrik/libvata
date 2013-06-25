/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Intersection for explicitly represented finite automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_ISECT_HH_
#define _VATA_EXPLICIT_FINITE_ISECT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>
#include <vata/finite_aut/explicit_finite_useless.hh>

// Standard library headers
#include <vector>

namespace VATA {
 template <class SymbolType>
		ExplicitFiniteAut<SymbolType> Intersection(
			const ExplicitFiniteAut<SymbolType> &lhs,
			const ExplicitFiniteAut<SymbolType> &rhs,
			AutBase::ProductTranslMap* pTranslMap = nullptr);
}

template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::Intersection(
		const VATA::ExplicitFiniteAut<SymbolType> &lhs,
		const VATA::ExplicitFiniteAut<SymbolType> &rhs,
		AutBase::ProductTranslMap* pTranslMap = nullptr) {

	typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;
	typedef typename ExplicitFA::StateSet StateSet;

	AutBase::ProductTranslMap translMap;


	if (!pTranslMap) {
		pTranslMap = &translMap;
	}

	VATA::ExplicitFiniteAut<SymbolType> res;

	std::vector<const AutBase::ProductTranslMap::value_type*> stack;

	// Loads the start states to the result automaton
	// Auto is here StateType of Explicit fitnite automata
	for (auto lss : lhs.startStates_) {
		for (auto rss : rhs.startStates_) {
			auto iss = pTranslMap->insert(std::make_pair(std::make_pair(lss,rss),
						pTranslMap->size())).first; // intersection start state

			stack.push_back(&*iss);
		}
	}

	typename ExplicitFA::StateToTransitionClusterMapPtr transitions
		= res.transitions_;

	while (!stack.empty()) {
		auto actState = stack.back();
		stack.pop_back();

		// Checks whether it is final state
		if (lhs.IsStateFinal(actState->first.first)
			&& rhs.IsStateFinal(actState->first.second)) {
			res.SetStateFinal(actState->second);
		}

		// Get transition clusters for given state
		auto lcluster = ExplicitFA::genericLookup
			(*lhs.transitions_,actState->first.first);

	 if (!lcluster) {
			continue;
		}

		//ExplicitFA::TransitionClusterPtr
		auto rcluster = ExplicitFA::genericLookup
			(*rhs.transitions_,actState->first.second);

		if (!rcluster) { // no such transition in right nfa
			continue;
		}

		typename ExplicitFA::TransitionClusterPtr clusterptr(nullptr);

		StateSet st;
		// Go through transitions of the given state
		for (auto lsymbolToPtrPointer : *lcluster) {
			auto lsymbol = lsymbolToPtrPointer.first;

			auto tempIter = rcluster->find(lsymbol);

			if (tempIter == rcluster->end()) {
				continue;
			}
			auto rstateSet = tempIter->second;
		 // Adding only usefull new state - for change move these two cycles before main cycle
		 if (lhs.IsStateStart(actState->first.first)) {
			for (auto& leftStartSymbol : lhs.GetStartSymbols(actState->first.first)) {
				 res.SetStateStart(actState->second,leftStartSymbol);
			}
		 }

		 if (rhs.IsStateStart(actState->first.second)) {
			 for (auto& rightStartSymbol : rhs.GetStartSymbols(actState->first.second)) {
					res.SetStateStart(actState->second,rightStartSymbol);
				}
			}

			// Get the transitions from the result automaton
			if (!clusterptr) { // Insert a new translated state
				clusterptr = transitions->uniqueCluster(actState->second);
			}

			// Insert a new symbol
			auto& stateSet = clusterptr->uniqueRStateSet(lsymbol);
			st = stateSet;

			for (auto lstate : lsymbolToPtrPointer.second) {
				for (auto rstate : rstateSet) {
					// Translate to intersection state
					auto istate = pTranslMap->insert
						(std::make_pair(std::make_pair(lstate,rstate),
														pTranslMap->size()));

					// Insert state from right side of transition
					stateSet.insert(istate.first->second);

					if (istate.second) { // New states added to stack
					 stack.push_back(&*istate.first);
					}
				}
			}
		}
	}

	return RemoveUselessStates(res);
}
#endif
