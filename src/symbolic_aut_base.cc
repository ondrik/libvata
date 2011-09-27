/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of the base class of symbolic automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/symbolic_aut_base.hh>

using VATA::SymbolicAutBase;

SymbolicAutBase::StringToSymbolDict* SymbolicAutBase::pSymbolDict_ = nullptr;

SymbolicAutBase::SymbolType* SymbolicAutBase::pNextSymbol_ = nullptr;

const size_t SymbolicAutBase::SYMBOL_SIZE;
