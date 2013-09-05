/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a BDD-based bottom-up tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_BU_TREE_AUT_HH_
#define _VATA_BDD_BU_TREE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/symbolic.hh>
#include <vata/notimpl_except.hh>
#include <vata/incl_param.hh>

// utilities
#include <vata/util/ord_vector.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>


namespace VATA
{
	class BDDBottomUpTreeAut;

	class BDDTopDownTreeAut;

	class BDDBUTreeAutCore;
}

GCC_DIAG_OFF(effc++)
class VATA::BDDBottomUpTreeAut : public SymbolicTreeAutBase
{
GCC_DIAG_ON(effc++)

public:   // data types

	using StateTuple = TreeAutBase::StateTuple;
	using SymbolType = Symbolic::SymbolType;

//	using StateBackTranslatorStrict =
//		VATA::Util::TranslatorStrict<AutBase::StringToStateDict::MapBwdType>;
//
//	using SymbolBackTranslatorStrict =
//		VATA::Util::TranslatorStrict<StringRankToSymbolDict::MapBwdType>;
//	using SymbolTranslatorStrict     =
//		VATA::Util::TranslatorStrict<StringRankToSymbolDict>;

private:  // data types

	typedef VATA::Util::AutDescription AutDescription;

private:  // methods

	explicit BDDBottomUpTreeAut(
		const BDDBUTreeAutCore&        core);

	explicit BDDBottomUpTreeAut(
		BDDBUTreeAutCore&&             core);

public:   // methods

	BDDBottomUpTreeAut();

	BDDBottomUpTreeAut(
		const BDDBottomUpTreeAut&         aut);

	BDDBottomUpTreeAut(
		BDDBottomUpTreeAut&&              aut);

	BDDBottomUpTreeAut& operator=(
		const BDDBottomUpTreeAut&         rhs);

	BDDBottomUpTreeAut& operator=(
		BDDBottomUpTreeAut&&              rhs);

	~BDDBottomUpTreeAut();


	void SetStateFinal(
		const StateType&               state);


	bool IsStateFinal(
		const StateType&               state) const;



	void AddTransition(
		const StateTuple&      children,
		const SymbolType&      symbol,
		const StateType&       parent);


	void LoadFromString(
		VATA::Parsing::AbstrParser&     parser,
		const std::string&              str,
		const std::string&              params = "");


	void LoadFromString(
		VATA::Parsing::AbstrParser&     parser,
		const std::string&              str,
		StateDict&                      stateDict,
		const std::string&              params = "");


	void LoadFromString(
		VATA::Parsing::AbstrParser&     parser,
		const std::string&              str,
		StringToStateTranslWeak&        stateTransl,
		const std::string&              params = "");


	void LoadFromAutDesc(
		const AutDescription&           desc,
		const std::string&              params = "");


	void LoadFromAutDesc(
		const AutDescription&           desc,
		StateDict&                      stateDict,
		const std::string&              params = "");


	void LoadFromAutDesc(
		const AutDescription&           desc,
		StringToStateTranslWeak&        stateTransl,
		const std::string&              params = "");


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const std::string&                         params = "") const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StateDict&                           stateDict,
		const std::string&                         params = "") const;


	template <
		class TranslIndex,
		class SanitizeIndex>
	std::string PrintSimulationMapping(
		TranslIndex          /*index*/,
		SanitizeIndex        /*sanitizeIndex*/)
	{
		throw NotImplementedException(__func__);
	}


	AlphabetType& GetAlphabet() const;


	BDDBottomUpTreeAut ReindexStates(
		StateToStateTranslWeak&       trans);


	std::string DumpToDot() const;


	static bool CheckInclusion(
		const BDDBottomUpTreeAut&    smaller,
		const BDDBottomUpTreeAut&    bigger,
		const VATA::InclParam&       params);

	BDDBottomUpTreeAut RemoveUnreachableStates() const;

	BDDBottomUpTreeAut RemoveUselessStates() const;

	BDDBottomUpTreeAut GetCandidateTree() const;


	template <class Dict>
	BDDBottomUpTreeAut Complement(
		const Dict&                  /*alphabet*/) const
	{
		throw NotImplementedException(__func__);
	}


	static BDDBottomUpTreeAut Union(
		const BDDBottomUpTreeAut&         lhs,
		const BDDBottomUpTreeAut&         rhs,
		AutBase::StateToStateMap*         pTranslMapLhs = nullptr,
		AutBase::StateToStateMap*         pTranslMapRhs = nullptr);

	static BDDBottomUpTreeAut UnionDisjointStates(
		const BDDBottomUpTreeAut&         lhs,
		const BDDBottomUpTreeAut&         rhs);

	static BDDBottomUpTreeAut Intersection(
		const BDDBottomUpTreeAut&         lhs,
		const BDDBottomUpTreeAut&         rhs,
		AutBase::ProductTranslMap*        pTranslMap = nullptr);


	BDDTopDownTreeAut GetTopDownAut() const;

	StateBinaryRelation ComputeDownwardSimulation() const;

	StateBinaryRelation ComputeDownwardSimulation(
		size_t                    size) const;

	StateBinaryRelation ComputeUpwardSimulation() const;

	StateBinaryRelation ComputeUpwardSimulation(
		size_t                    size) const;

private:  // data members

	std::unique_ptr<BDDBUTreeAutCore> core_;
};

#endif
