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
#include <vata/symbolic.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/notimpl_except.hh>
#include <vata/incl_param.hh>
#include <vata/sim_param.hh>

#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>

// Utilities
#include <vata/util/ord_vector.hh>
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

	template <
		class>
	class LoadableAut;

	class BDDTDTreeAutCore;
}


// TODO: both BDDTopDownTreeAut and BDDTDTreeAutCore should not be derived from
//SymbolicAutBase. And the same for BDDBottomUpTreeAut and BDDBUTreeAutCore.
class VATA::BDDTopDownTreeAut : public SymbolicTreeAutBase
{
	friend VATA::BDDBottomUpTreeAut;

private:  // data types

	using CoreAut        = VATA::LoadableAut<BDDTDTreeAutCore>;

private:  // data members

	std::unique_ptr<CoreAut> core_;

private:  // methods

	explicit BDDTopDownTreeAut(
		const CoreAut&                     core);

	explicit BDDTopDownTreeAut(
		CoreAut&&                          core);

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


	AlphabetType& GetAlphabet() const;


	const FinalStateSet& GetFinalStates() const;

	bool IsStateFinal(const StateType& state) const;

	void AddTransition(
		const StateTuple&       children,
		const SymbolType&       symbol,
		const StateType&        parent);


	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		const std::string&               params = "");


	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		StateDict&                       stateDict,
		const std::string&               params = "");


	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		StringToStateTranslWeak&         stateTransl,
		const std::string&               params = "");


	void LoadFromAutDesc(
		const VATA::Util::AutDescription&   desc,
		const std::string&                  params = "");


	void LoadFromAutDesc(
		const VATA::Util::AutDescription&   desc,
		StateDict&                          stateDict,
		const std::string&                  params = "");


	void LoadFromAutDesc(
		const VATA::Util::AutDescription&   desc,
		StringToStateTranslWeak&            stateTransl,
		const std::string&                  params = "");


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const std::string&                         params = "") const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StateDict&                           stateDict,
		const std::string&                         params = "") const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&  serializer,
		const StateBackTranslStrict&           stateTransl,
		const std::string&                     params = "") const;


	void SetStateFinal(
		const StateType&               state);


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


	StateDiscontBinaryRelation ComputeSimulation(
		const SimParam&        params) const;


	BDDTopDownTreeAut RemoveUnreachableStates() const;


	BDDTopDownTreeAut RemoveUselessStates() const;


	BDDTopDownTreeAut Complement() const
	{
		throw NotImplementedException(__func__);
	}


	BDDTopDownTreeAut Reduce() const
	{
		throw NotImplementedException(__func__);
	}


	BDDTopDownTreeAut ReindexStates(
		StateToStateTranslWeak&     stateTrans) const;


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
};

#endif
