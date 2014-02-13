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
#include <vata/sim_param.hh>

// utilities
#include <vata/util/ord_vector.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>


namespace VATA
{
	class BDDBottomUpTreeAut;

	class BDDTopDownTreeAut;

	template <
		class>
	class LoadableAut;

	class BDDBUTreeAutCore;
}

GCC_DIAG_OFF(effc++)
class VATA::BDDBottomUpTreeAut : public SymbolicTreeAutBase
{
GCC_DIAG_ON(effc++)

private:  // data types

	using CoreAut        = VATA::LoadableAut<BDDBUTreeAutCore>;

private:  // data members

	std::unique_ptr<CoreAut> core_;

private:  // methods

	explicit BDDBottomUpTreeAut(
		CoreAut&&                      core);

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


	const FinalStateSet& GetFinalStates() const;


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


	/**
	 * @brief  Checks inclusion using default parameters
	 *
	 * This static method checks language inclusion of a pair of automata (@p
	 * smaller, @p bigger) using default parameters.
	 *
	 * @param[in]  smaller  The smaller automaton
	 * @param[in]  bigger   The bigger automaton
	 *
	 * @returns  @p true if the language of @p smaller is a subset of the language
	 *           of @p bigger, @p false otherwise
	 */
	static bool CheckInclusion(
		const BDDBottomUpTreeAut&    smaller,
		const BDDBottomUpTreeAut&    bigger);


	/**
	 * @brief  Dispatcher for calling correct inclusion checking function
	 *
	 * This function is a dispatcher that calls a proper inclusion checking
	 * function between @p smaller and @p bigger according to the parameters in @p
	 * params.
	 *
	 * @param[in]  smaller  The smaller automaton
	 * @param[in]  bigger   The bigger automaton
	 * @param[in]  params   Parameters for the inclusion
	 *
	 * @returns  @p true if the language of @p smaller is a subset of the language
	 *           of @p bigger, @p false otherwise
	 */
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


	BDDBottomUpTreeAut Reduce() const
	{
		throw NotImplementedException(__func__);
	}


	StateBinaryRelation ComputeSimulation(
		const VATA::SimParam&              params) const;
};

#endif
