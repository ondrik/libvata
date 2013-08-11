/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Reversion for explicitly represented finite automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>

VATA::ExplicitFiniteAut VATA::ExplicitFiniteAut::Reverse(
	AutBase::StateToStateMap* /*pTranslMap*/) const
{
	typedef VATA::ExplicitFiniteAut ExplicitFA;

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
