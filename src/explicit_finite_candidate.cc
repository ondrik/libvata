/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Get candidate for finite automata in explicit encoding.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include "explicit_finite_aut_core.hh"

// std library headers
#include <list>
#include <unordered_map>

/*
 * Get candidate nfa. It is the smallest nfa which
 * has language which is subset of the input nfa
 */
VATA::ExplicitFiniteAutCore VATA::ExplicitFiniteAutCore::GetCandidateTree() const {

	typedef ExplicitFiniteAutCore ExplicitFA;
	typedef typename ExplicitFA::StateType StateType;

	std::unordered_set<StateType> reachableStates;
	std::list<StateType> newStates;

	ExplicitFA res;


	// Starts from start states
	for (StateType s : this->GetStartStates()) {
		if (reachableStates.insert(s).second) {
			newStates.push_back(s);
		}
		res.SetExistingStateStart(s, this->GetStartSymbols(s));
	}

	while (!newStates.empty()) {
		StateType actState = newStates.front();
		// Find transitions for state s
		auto transitionsCluster = transitions_->find(actState);

		newStates.pop_front();

		if (transitionsCluster == transitions_->end()) {
			continue;
		}

		for (auto symbolToState : *transitionsCluster->second) {
			// All states which are reachable from actState are added to result automaton
			for (auto stateInSet : symbolToState.second){

				if (reachableStates.insert(stateInSet).second) {
					newStates.push_back(stateInSet);
				}

				if (this->IsStateFinal(stateInSet)) { // Set final state and return
					res.SetStateFinal(stateInSet);
					res.transitions_->insert(std::make_pair(actState,transitionsCluster->second));
					//res.internalAddTransition(actState,symbolToState.first,stateInSet);
					return res.RemoveUselessStates();
				}
				// add to transitions
				res.transitions_->insert(std::make_pair(actState,transitionsCluster->second));
				//res.internalAddTransition(actState,symbolToState.first,stateInSet);
			}
		}
	}

	return res.RemoveUselessStates();
}
