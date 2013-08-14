/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the base class of symbolic automata.
 *
 *****************************************************************************/

#ifndef _VATA_SYMBOLIC_AUT_BASE_HH_
#define _VATA_SYMBOLIC_AUT_BASE_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>
#include <vata/sym_var_asgn.hh>


namespace VATA
{
	class SymbolicAutBase;
}


GCC_DIAG_OFF(effc++)
class VATA::SymbolicAutBase : public AutBase
{
GCC_DIAG_ON(effc++)

public:   // data types

	typedef VATA::SymbolicVarAsgn SymbolType;

	typedef VATA::Util::TwoWayDict<std::string, SymbolType> StringToSymbolDict;

protected:// constants

	const static size_t SYMBOL_SIZE = 16;

private:  // data members

	static StringToSymbolDict* pSymbolDict_;
	static SymbolType* pNextSymbol_;

protected:// methods

	SymbolicAutBase() { }

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

	static void SetSymbolDictPtr(StringToSymbolDict* pSymbolDict)
	{
		// Assertions
		assert(pSymbolDict != nullptr);

		pSymbolDict_ = pSymbolDict;
	}

	static StringToSymbolDict& GetSymbolDict()
	{
		// Assertions
		assert(pSymbolDict_ != nullptr);

		return *pSymbolDict_;
	}

};

#endif
