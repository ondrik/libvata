/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Removing useless states for explicitly represented finite automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_AUT_USELESS_HH_
#define _VATA_EXPLICIT_FINITE_AUT_USELESS_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>
#include <vata/finite_aut/explicit_finite_reverse.hh>
#include <vata/finite_aut/explicit_finite_unreach.hh>


template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::ExplicitFiniteAut<SymbolType>::RemoveUselessStates(
		VATA::AutBase::StateToStateMap* pTranslMap) {

	// remove useless states by applying following operations
	return this->
		RemoveUnreachableStates(pTranslMap).
		Reverse(pTranslMap).
		RemoveUnreachableStates().
		Reverse();
}

#endif
