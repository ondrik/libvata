/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013 Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Source file for an explicit finite automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_finite_aut.hh>

#include "explicit_finite_aut_core.hh"
#include "loadable_aut.hh"

using VATA::ExplicitFiniteAutCore;

// global alphabet
ExplicitFiniteAutCore::AlphabetType ExplicitFiniteAutCore::globalAlphabet_ =
	AlphabetType(new ExplicitFiniteAut::OnTheFlyAlphabet);

ExplicitFiniteAutCore::ExplicitFiniteAutCore(AlphabetType& alphabet) :
	finalStates_(),
	startStates_(),
	startStateToSymbols_(),
	transitions_(StateToTransitionClusterMapPtr(new StateToTransitionClusterMap)),
	alphabet_(alphabet)
{ }

ExplicitFiniteAutCore::ExplicitFiniteAutCore(const ExplicitFiniteAutCore& aut) :
	finalStates_(aut.finalStates_),
	startStates_(aut.startStates_),
	startStateToSymbols_(aut.startStateToSymbols_),
	transitions_(aut.transitions_),
	alphabet_(aut.alphabet_)
{ }

ExplicitFiniteAutCore& ExplicitFiniteAutCore::operator=(const ExplicitFiniteAutCore& rhs)
{
	if (this != &rhs)
	{
		finalStates_          = rhs.finalStates_;
		startStates_          = rhs.startStates_;
		startStateToSymbols_  = rhs.startStateToSymbols_;
		transitions_          = rhs.transitions_;
		alphabet_             = rhs.alphabet_;
	}

	return *this;
}

/*
AutBase::StateBinaryRelation ExplicitFiniteAutCore::ComputeDownwardSimulation(
	size_t              size)
{
	AutBase::StateBinaryRelation relation;
	std::vector<std::vector<size_t>> partition(1);

	return Translate(*this, partition, relation).computeSimulation(partition,relation,size);
}
*/
