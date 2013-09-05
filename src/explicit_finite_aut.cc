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

using VATA::ExplicitFiniteAut;

// pointer to symbol dictionary
ExplicitFiniteAut::AlphabetType ExplicitFiniteAut::globalAlphabet_ =
	AlphabetType(new AlphabetType::element_type());
