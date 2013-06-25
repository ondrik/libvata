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


#ifndef _VATA_EXPLICIT_FINITE_AUT_COMPL_HH_
#define _VATA_EXPLICIT_FINITE_AUT_COMPL_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>

namespace VATA {
	template <class SymbolType, class Dict>
	ExplicitFiniteAut<SymbolType> Complement(
			const ExplicitFiniteAut<SymbolType> &aut,
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
 VATA::ExplicitFiniteAut<SymbolType> VATA::Complement(
			const VATA::ExplicitFiniteAut<SymbolType> &aut,
			const Dict &/*alphabet*/) {

	 VATA_LOGGER_WARN("Calling " + std::string(__func__));

	typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

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

#endif
