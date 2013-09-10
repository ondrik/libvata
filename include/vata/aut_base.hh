/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for base classes of automata.
 *
 *****************************************************************************/

#ifndef _VATA_AUT_BASE_HH_
#define _VATA_AUT_BASE_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/notimpl_except.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/symbolic.hh>

// Utilities
#include <vata/util/binary_relation.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/transl_weak.hh>
#include <vata/util/transl_strict.hh>


namespace VATA
{
	class AutBase;
	class TreeAutBase;
	class SymbolicTreeAutBase;

	template <class Automaton>
	Automaton Reduce(const Automaton&)
	{
		throw NotImplementedException(__func__);
	}

	template <class Automaton>
	Automaton GetCandidateTree(const Automaton&)
	{
		throw NotImplementedException(__func__);
	}

	/**
	 * @brief  Generic procedure for checking equivalence of automata
	 *
	 * To be used when a specific implementation is not available
	 */
	template <class Automaton>
	bool CheckEquivalence(const Automaton& lhs, const Automaton& rhs)
	{
		return CheckInclusion(lhs, rhs) && CheckInclusion(rhs, lhs);
	}
}


/**
 * @brief  Base class for automata
 *
 * This is the base class for automata. Contains  mostly static definitions, no
 * virtual methods are present (so that the overhead of virtual method table
 * would not propagate.
 */
class VATA::AutBase
{
public:   // data types

	using StateType = size_t;

	using AutDescription             = VATA::Util::AutDescription;
	using StateDict                  = Util::TwoWayDict<std::string, StateType>;
	using StringToStateTranslWeak    = Util::TranslatorWeak<StateDict>;
	using StringToStateTranslStrict  = Util::TranslatorStrict<StateDict>;
	using StateBackTranslStrict      = Util::TranslatorStrict<StateDict::MapBwdType>;

	using StateToStateMap         = std::unordered_map<StateType, StateType>;
	using StateToStateTranslWeak  = Util::TranslatorWeak<StateToStateMap>;
	using StateToStateTranslStrict= Util::TranslatorStrict<StateToStateMap>;
	using StateToStringConvFunc   = std::function<std::string(const StateType&)>;

	using StatePair = std::pair<StateType, StateType>;

	using ProductTranslMap =
		std::unordered_map<StatePair, StateType, boost::hash<StatePair>>;

	using StateBinaryRelation = Util::BinaryRelation;

protected:// methods

	AutBase() { }

public:   // methods

	template <class Automaton>
	static StateType SanitizeAutsForInclusion(
		Automaton&     smaller,
		Automaton&     bigger)
	{
		StateType stateCnt = 0;
		StateToStateMap stateMap;
		StateToStateTranslWeak stateTrans(stateMap,
			[&stateCnt](const StateType&){return stateCnt++;});

		Automaton tmpAut = smaller.RemoveUselessStates();
		Automaton newSmaller = tmpAut.ReindexStates(stateTrans);

		tmpAut = bigger.RemoveUselessStates();
		stateMap.clear();
		Automaton newBigger = tmpAut.ReindexStates(stateTrans);

		smaller = newSmaller;
		bigger = newBigger;

		return stateCnt;
	}

	/*
	template <class Automaton>
	static StateType SanitizeAutForSimulation(Automaton& aut)
	{
		StateType stateCnt = 0;
		StateToStateMap stateMap;
		StateToStateTranslator stateTrans(stateMap,
			[&stateCnt](const StateType&){return stateCnt++;});

		Automaton newAut = aut.RemoveUselessStates();
		Automaton reindexedAut = newAut.ReindexStates(stateTrans);

		aut = reindexedAut;

		return stateCnt;
	}
	*/

	template <
		class Automaton,
		class Index>
	static StateType SanitizeAutForSimulation(
		Automaton&       aut,
		StateType&       stateCnt,
		Index&           index)
	{
		Automaton newAut = aut.RemoveUselessStates();
		aut = newAut.ReindexStates(index);

		return stateCnt;
	}
};


GCC_DIAG_OFF(effc++)
class VATA::TreeAutBase : public AutBase
{
GCC_DIAG_ON(effc++)

protected:// methods

	TreeAutBase() { }

public:   // data types

	using StateTuple     = std::vector<StateType>;

protected:// data types

	template <
		class TSymbol>
	class TTransition
	{
	private:  // data types

		using SymbolType = TSymbol;

	private:  // data members

		StateType parent_;
		SymbolType symbol_;
		StateTuple children_;

	public:

		TTransition(
			const StateType&      parent,
			const SymbolType&     symbol,
			const StateTuple&     children) :
			parent_(parent),
			symbol_(symbol),
			children_(children)
		{ }

		const StateType& Parent()    const { return parent_;   }
		const SymbolType& Symbol()   const { return symbol_;   }
		const StateTuple& Children() const { return children_; }
	};
};


GCC_DIAG_OFF(effc++)
class VATA::SymbolicTreeAutBase :
	public TreeAutBase,
	public Symbolic
{
GCC_DIAG_ON(effc++)

public:   // data types

	using StringSymbolType = std::string;

	using SymbolDict                      = Util::TwoWayDict<std::string, SymbolType>;
	using StringSymbolToSymbolTranslStrict= Util::TranslatorStrict<SymbolDict>;
	using StringSymbolToSymbolTranslWeak  = Util::TranslatorWeak<SymbolDict>;
	using SymbolBackTranslStrict          =
		Util::TranslatorStrict<typename SymbolDict::MapBwdType>;

	class Alphabet
	{
	private:  // data members

		SymbolDict symbolDict_{};
		SymbolType nextSymbol_;

	public:   // methods

		Alphabet() :
			nextSymbol_(Symbolic::GetZeroSymbol())
		{ }

		StringSymbolToSymbolTranslWeak GetSymbolTransl()
		{
			return StringSymbolToSymbolTranslWeak{symbolDict_,
				[&](const StringSymbolType&){return nextSymbol_++;}};
		}

		SymbolDict& GetSymbolDict()
		{
			return symbolDict_;
		}

	};

	using AlphabetType = std::shared_ptr<Alphabet>;

	using Transition = TreeAutBase::TTransition<SymbolType>;

protected:// methods

	SymbolicTreeAutBase() { }

public:   // methods

	static const StringSymbolType& ToStringSymbolType(
		const std::string&         str,
		size_t                     /* rank */)
	{
		return str;
	}
};

#endif
