/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of the base class of symbolic objects.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/symbolic.hh>

using VATA::Symbolic;

Symbolic::SymbolType* Symbolic::pNextSymbol_ = nullptr;

const size_t Symbolic::SYMBOL_SIZE;
