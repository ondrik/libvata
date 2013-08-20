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
#include <vata/finite_aut/explicit_finite_aut.hh>

// pointer to symbol dictionary
VATA::ExplicitFiniteAut::StringToSymbolDict* VATA::ExplicitFiniteAut::pSymbolDict_ = nullptr;

// pointer to next symbol counter
VATA::ExplicitFiniteAut::SymbolType* VATA::ExplicitFiniteAut::pNextSymbol_ = nullptr;
