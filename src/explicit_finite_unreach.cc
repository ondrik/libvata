/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Removing unreachable states for explicitly represented finite automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/util/transl_strict.hh>

#include "explicit_finite_aut_core.hh"

// Standard library headers
#include <vector>
#include <unordered_set>
#include <utility>


VATA::ExplicitFiniteAutCore VATA::ExplicitFiniteAutCore::RemoveUnreachableStates(
	VATA::AutBase::StateToStateMap* pTranslMap) {

	typedef VATA::ExplicitFiniteAutCore ExplicitFA;
	typedef typename ExplicitFA::StateToTransitionClusterMapPtr
		StateToTransitionClusterMapPtr;

	// start from start states
	std::unordered_set<AutBase::StateType> reachableStates(this->GetStartStates());
	std::vector<AutBase::StateType> newStates(reachableStates.begin(),reachableStates.end());

	// Find all reachable states
	while (!newStates.empty()) { // while there are new reacheable states
		auto actState = newStates.back();

		newStates.pop_back();

		auto cluster = ExplicitFA::genericLookup(*transitions_, actState);

		if (!cluster) {
			continue;
		}

		 // Add all reachable states to reachable state set
		for (auto &symbolsToStateSet : *cluster) {
			for (auto &state : symbolsToStateSet.second) {
				if (reachableStates.insert(state).second) {
					newStates.push_back(state);
				}
			}
		}
	}

	if (pTranslMap) {
		for (auto& state : reachableStates) {
			pTranslMap->insert(std::make_pair(state, state));
		}
	}

	ExplicitFA res;
	//initialize the result automaton
	res.startStates_ = startStates_;
	res.startStateToSymbols_ = startStateToSymbols_;

	res.transitions_ = StateToTransitionClusterMapPtr(
			new typename ExplicitFA::StateToTransitionClusterMap()
			);

	// Add all reachables states and its transitions to result nfa
	for (auto& state : reachableStates) {

		if (this->IsStateFinal(state)) {
			res.SetStateFinal(state);
		}

		auto stateToClusterIterator = transitions_->find(state);
		if (stateToClusterIterator == transitions_->end()) {
			continue;
		}

		res.transitions_->insert(std::make_pair(state,stateToClusterIterator->second));
	}

	return res;
}
