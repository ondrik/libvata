/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of the base class of symbolic tree automata's core.
 *
 *****************************************************************************/

// VATA headers
#include "symbolic_tree_aut_base_core.hh"

using VATA::SymbolicTreeAutBaseCore;

// global alphabet
SymbolicTreeAutBaseCore::AlphabetType SymbolicTreeAutBaseCore::globalAlphabet_ =
	AlphabetType(new OnTheFlyAlphabet());
