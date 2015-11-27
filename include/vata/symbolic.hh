/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the base class of symbolic objects.
 *
 *****************************************************************************/

#ifndef _VATA_SYMBOLIC_HH_
#define _VATA_SYMBOLIC_HH_

// VATA headers
#include <vata/sym_var_asgn.hh>
#include <vata/vata.hh>
#include <vata/util/two_way_dict.hh>


namespace VATA
{
	class Symbolic;
}


class VATA::Symbolic
{
public:   // data types

	using SymbolType = VATA::SymbolicVarAsgn;

protected:// constants

	// TODO: this is not nice... SYMBOL_SIZE should be dynamic, no?
	// Why is it needed at all? Only for the GetZeroSymbol()?
	const static size_t SYMBOL_SIZE = 16;

protected:// methods

	Symbolic() { }

public:   // methods

	static SymbolType GetZeroSymbol()
	{
		return SymbolType(SYMBOL_SIZE, 0);
	}
};

#endif
