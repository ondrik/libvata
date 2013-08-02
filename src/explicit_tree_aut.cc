/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Source file for an explicit tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/ta_expl/explicit_tree_aut.hh>

// global tuple cache definition
VATA::Explicit::TupleCache VATA::Explicit::tupleCache;

// pointer to symbol dictionary
VATA::ExplicitTreeAut::StringToSymbolDict* VATA::ExplicitTreeAut::pSymbolDict_ = nullptr;

// pointer to next symbol counter
VATA::ExplicitTreeAut::SymbolType* VATA::ExplicitTreeAut::pNextSymbol_ = nullptr;

