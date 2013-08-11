/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Removing useless states for explicitly represented finite automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>

VATA::ExplicitFiniteAut VATA::ExplicitFiniteAut::RemoveUselessStates(
		VATA::AutBase::StateToStateMap* pTranslMap) {

	// remove useless states by applying following operations
	return this->
		RemoveUnreachableStates(pTranslMap).
		Reverse(pTranslMap).
		RemoveUnreachableStates().
		Reverse();
}
