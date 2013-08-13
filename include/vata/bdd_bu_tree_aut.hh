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
#include <vata/notimpl_except.hh>

// utilities
#include <vata/util/ord_vector.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>


namespace VATA
{
	class BDDBottomUpTreeAut;

	class BDDBUTreeAutCore;
}

GCC_DIAG_OFF(effc++)
class VATA::BDDBottomUpTreeAut
	: public SymbolicAutBase
{
GCC_DIAG_ON(effc++)

public:   // data types

	using StateBackTranslatorStrict =
		VATA::Util::TranslatorStrict<AutBase::StringToStateDict::MapBwdType>;

	using SymbolBackTranslatorStrict  =
		VATA::Util::TranslatorStrict<StringToSymbolDict::MapBwdType>;

	typedef std::unordered_set<StateType> StateHT;
	typedef std::vector<StateType> StateTuple;

	typedef VATA::Util::TranslatorStrict<StringToSymbolDict> SymbolTranslatorStrict;

	// FIXME: only stub
	typedef std::vector<SymbolType> AlphabetType;

private:  // data types

	typedef VATA::Util::AutDescription AutDescription;


	typedef VATA::Util::Convert Convert;


private:  // methods

	explicit BDDBottomUpTreeAut(
		const BDDBUTreeAutCore&        core);

	explicit BDDBottomUpTreeAut(
		BDDBUTreeAutCore&&             core);

	void SetStateFinal(
		const StateType&               state);


	bool IsStateFinal(
		const StateType&               state) const;


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


	void AddTransition(
		const StateTuple&      children,
		SymbolType             symbol,
		const StateType&       parent);



	void LoadFromString(
		VATA::Parsing::AbstrParser&     parser,
		const std::string&              str,
		StringToStateDict&              stateDict);


	template <
		class StateTransFunc,
		class SymbolTransFunc
		>
	void LoadFromString(
		VATA::Parsing::AbstrParser&       parser,
		const std::string&                str,
		StateTransFunc                    stateTranslator,
		SymbolTransFunc                   symbolTranslator,
		const std::string&                params = "")
	{
		LoadFromAutDesc(parser.ParseString(str), stateTranslator,
				symbolTranslator, params);
	}

	void LoadFromAutDesc(
		const AutDescription&         desc,
		StringToStateDict&            stateDict);


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer) const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StringToStateDict&                   stateDict) const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StateBackTranslatorStrict&           stateTrans,
		const SymbolBackTranslatorStrict&          symbolTrans) const;


	BDDBottomUpTreeAut ReindexStates(
		StateToStateTranslator&       trans);


	std::string DumpToDot() const;


	static inline AlphabetType GetAlphabet()
	{
		throw NotImplementedException(__func__);
	}

	static bool CheckInclusion(
		const BDDBottomUpTreeAut&    smaller,
		const BDDBottomUpTreeAut&    bigger,
		const VATA::InclParam&       params);

	BDDBottomUpTreeAut RemoveUnreachableStates() const;

	BDDBottomUpTreeAut RemoveUselessStates() const;

	BDDBottomUpTreeAut GetCandidateTree() const
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
