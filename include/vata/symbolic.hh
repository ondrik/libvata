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


GCC_DIAG_OFF(effc++)
class VATA::Symbolic
{
GCC_DIAG_ON(effc++)

public:   // data types

	typedef VATA::SymbolicVarAsgn SymbolType;

protected:// constants

	const static size_t SYMBOL_SIZE = 16;

private:  // data members

	static SymbolType* pNextSymbol_;

protected:// methods

	Symbolic() { }

public:   // methods

	static SymbolType AddSymbol()
	{
		// Assertions
		assert(pNextSymbol_ != nullptr);

		return (*pNextSymbol_)++;
	}

	static void SetNextSymbolPtr(SymbolType* pNextSymbol)
	{
		// Assertions
		assert(pNextSymbol != nullptr);

		pNextSymbol_ = pNextSymbol;
	}

	static SymbolType GetZeroSymbol()
	{
		return SymbolType(SYMBOL_SIZE, 0);
	}
};

#endif
