/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the base class of symbolic tree automata cores.
 *
 *****************************************************************************/

#ifndef _VATA_SYMBOLIC_TREE_AUT_BASE_CORE_HH_
#define _VATA_SYMBOLIC_TREE_AUT_BASE_CORE_HH_

#include <vata/vata.hh>
#include <vata/aut_base.hh>

namespace VATA
{
	class SymbolicTreeAutBaseCore;
}


GCC_DIAG_OFF(effc++)
class VATA::SymbolicTreeAutBaseCore :
	public SymbolicTreeAutBase
{
GCC_DIAG_ON(effc++)

private:  // data members

	/**
	 * @brief  The alphabet of the automaton
	 *
	 * This data member is declared as mutable because it may change, e.g.,
	 * during loading of another automaton.
	 */
	mutable AlphabetType alphabet_;

protected:// data members

	static AlphabetType globalAlphabet_;

public:   // methods

	AlphabetType& GetAlphabet() const
	{
		// Assertions
		assert(nullptr != alphabet_);

		return alphabet_;
	}

protected:// methods

	/**
	 * @brief  Protected constructor
	 *
	 * This contructor enables construction only by derived classes.
	 */
	explicit SymbolicTreeAutBaseCore(AlphabetType& alphabet) :
		alphabet_(alphabet)
	{ }


	SymbolicTreeAutBaseCore(const SymbolicTreeAutBaseCore& aut) :
		alphabet_(aut.alphabet_)
	{ }


	SymbolicTreeAutBaseCore(SymbolicTreeAutBaseCore&& aut) :
		alphabet_(std::move(aut.alphabet_))
	{ }

	SymbolicTreeAutBaseCore& operator=(const SymbolicTreeAutBaseCore& rhs)
	{
		if (nullptr != this)
		{
			alphabet_ = rhs.alphabet_;
		}

		return *this;
	}

	SymbolicTreeAutBaseCore& operator=(SymbolicTreeAutBaseCore&& rhs)
	{
		assert(nullptr != this);

		alphabet_ = std::move(rhs.alphabet_);

		return *this;
	}
};

#endif
