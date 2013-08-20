/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a BDD-based top-down tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TD_TREE_AUT_HH_
#define _VATA_BDD_TD_TREE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/symbolic_aut_base.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/notimpl_except.hh>
#include <vata/incl_param.hh>

#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>

// Utilities
#include <vata/util/ord_vector.hh>
#include <vata/util/vector_map.hh>
#include <vata/util/util.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>

// Standard library headers
#include <cstdint>
#include <unordered_set>

namespace VATA
{
	class BDDTopDownTreeAut;

	class BDDBottomUpTreeAut;

	class BDDTDTreeAutCore;
}


// TODO: both BDDTopDownTreeAut and BDDTDTreeAutCore should not be derived from
//SymbolicAutBase. And the same for BDDBottomUpTreeAut and BDDBUTreeAutCore.
GCC_DIAG_OFF(effc++)
class VATA::BDDTopDownTreeAut : public SymbolicAutBase
{
GCC_DIAG_ON(effc++)

	friend VATA::BDDBottomUpTreeAut;

public:   // data types

	using StateTuple   = std::vector<StateType>;

	typedef typename SymbolicAutBase::SymbolType SymbolType;

	typedef VATA::Util::TwoWayDict<std::string, SymbolType> StringToSymbolDict;

	typedef VATA::Util::TranslatorStrict<StringToSymbolDict> SymbolTranslatorStrict;

	using SymbolBackTranslatorStrict   =
		VATA::Util::TranslatorStrict<typename StringToSymbolDict::MapBwdType>;

	// FIXME: only stub
	typedef std::vector<SymbolType> AlphabetType;

private:  // data types

	typedef VATA::Util::Convert Convert;

private:  // methods

	BDDTopDownTreeAut(
		const BDDTDTreeAutCore&            aut);

	BDDTopDownTreeAut(
		BDDTDTreeAutCore&&                 aut);

public:   // public methods

	BDDTopDownTreeAut();

	BDDTopDownTreeAut(
		const BDDTopDownTreeAut&           aut);

	BDDTopDownTreeAut(
		BDDTopDownTreeAut&&                aut);

	BDDTopDownTreeAut& operator=(
		const BDDTopDownTreeAut&           rhs);

	BDDTopDownTreeAut& operator=(
		BDDTopDownTreeAut&&                rhs);

	~BDDTopDownTreeAut();

	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer) const;


	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		StringToStateDict&               stateDict);


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StringToStateDict&                   stateDict) const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StateBackTranslatorStrict&           stateTrans,
		const SymbolBackTranslatorStrict&          symbolTrans) const;


	void SetStateFinal(
		const StateType&               state);


	static AlphabetType GetAlphabet()
	{
		throw NotImplementedException(__func__);
	}


	static bool CheckInclusion(
		const BDDTopDownTreeAut&    smaller,
		const BDDTopDownTreeAut&    bigger,
		const VATA::InclParam&      params);


	template <
		class TranslIndex,
		class SanitizeIndex>
	std::string PrintSimulationMapping(
		TranslIndex       /*index*/,
		SanitizeIndex     /*sanitizeIndex*/)
	{
		throw NotImplementedException(__func__);
	}


	AutBase::StateBinaryRelation ComputeDownwardSimulation() const;


	AutBase::StateBinaryRelation ComputeDownwardSimulation(size_t size) const;


	AutBase::StateBinaryRelation ComputeUpwardSimulation() const;


	AutBase::StateBinaryRelation ComputeUpwardSimulation(size_t size) const;


	BDDTopDownTreeAut RemoveUnreachableStates() const;


	BDDTopDownTreeAut RemoveUselessStates() const;


	template <class Dict>
	BDDTopDownTreeAut Complement(
		const Dict&                     /*alphabet*/) const
	{
		throw NotImplementedException(__func__);
	}


	BDDTopDownTreeAut ReindexStates(
		StateToStateTranslator&     stateTrans) const;


	BDDTopDownTreeAut GetCandidateTree() const
	{
		throw NotImplementedException(__func__);
	}


	static BDDTopDownTreeAut Union(
		const BDDTopDownTreeAut&      lhs,
		const BDDTopDownTreeAut&      rhs,
		AutBase::StateToStateMap*     pTranslMapLhs = nullptr,
		AutBase::StateToStateMap*     pTranslMapRhs = nullptr);


	static BDDTopDownTreeAut UnionDisjointStates(
		const BDDTopDownTreeAut&      lhs,
		const BDDTopDownTreeAut&      rhs);


	static BDDTopDownTreeAut Intersection(
		const BDDTopDownTreeAut&      lhs,
		const BDDTopDownTreeAut&      rhs,
		AutBase::ProductTranslMap*    pTranslMap = nullptr);


private:  // data members

	std::unique_ptr<BDDTDTreeAutCore> core_;

};

#endif
