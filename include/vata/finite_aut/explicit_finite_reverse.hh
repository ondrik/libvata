/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Reversion for explicitly represented finite automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_AUT_REVERSE_HH_
#define _VATA_EXPLICIT_FINITE_AUT_REVERSE_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>

template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::ExplicitFiniteAut<SymbolType>::Reverse(
	AutBase::StateToStateMap* /*pTranslMap*/) const
{
	typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

	ExplicitFA res;

	res.finalStates_ = startStates_; // set final states
	res.startStates_ = finalStates_; // set start states
	res.startStateToSymbols_ = startStateToSymbols_; // start symbols

	// Changing the order of left and right in each transition
	for (auto stateToCluster : *transitions_) {
		for (auto symbolToSet : *stateToCluster.second) {
			for (auto stateInSet : symbolToSet.second) {
				res.AddTransition(stateInSet,symbolToSet.first,
					stateToCluster.first);
			}
		}
	}

	return res;
}

#endif
