/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Complementation for explicitly represented finite automata.
 *	JUST PROTOTYPE
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include "explicit_finite_aut_core.hh"

namespace VATA {
	template <class SymbolType, class Dict>
	ExplicitFiniteAutCore Complement(
			const ExplicitFiniteAutCore &aut,
			const Dict &alphabet);
}

	/**
	 * @brief	 Creates a complement of an NFA.
	 *
	 * Now works only for DFA.
	 *
	 * @note	This works only for fully-specified deterministic FA! Use at your
	 * own risk.
	 *
	 * @todo	Finish? Add check for determinism?
	 */
 template <class SymbolType, class Dict>
 VATA::ExplicitFiniteAutCore VATA::Complement(
			const VATA::ExplicitFiniteAutCore &aut,
			const Dict &/*alphabet*/) {

	VATA_WARN("Calling " << __func__);

	typedef VATA::ExplicitFiniteAutCore ExplicitFA;

	ExplicitFA res;
	res.transitions_ = aut.transitions_;
	res.startStates_ = aut.startStates_;

	auto transitions_ = aut.transitions_;

	// All nonfinal states are marked as final states
	for (auto stateToCluster : *transitions_) {
		if (!aut.IsStateFinal(stateToCluster.first)) {
			res.SetStateFinal(stateToCluster.first);
		}
		for (auto symbolToSet : *stateToCluster.second) {
			for (auto stateInSet : symbolToSet.second) {
				if (!aut.IsStateFinal(stateInSet)) {
					res.SetStateFinal(stateInSet);
				}
			}
		}
	}

	return res;
}
