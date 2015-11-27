/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Upward inclusion for explicitly represented tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_INCL_UP_HH_
#define _VATA_EXPLICIT_TREE_INCL_UP_HH_

#include "explicit_tree_aut_core.hh"
#include "util/expl_bu_index.hh"

namespace VATA { class ExplicitUpwardInclusion; }


class VATA::ExplicitUpwardInclusion
{
	using StateType      = ExplicitTreeAutCore::StateType;
	using SymbolType     = ExplicitTreeAutCore::SymbolType;
	using StateTuple     = ExplicitTreeAutCore::StateTuple;
	using StateDiscontBinaryRelation = ExplicitTreeAutCore::StateDiscontBinaryRelation;

public:

	template <
		class Aut,
		class Rel>
	static bool Check(
		const Aut&        smaller,
		const Aut&        bigger,
		const Rel&        preorder)
	{
		IndexedSymbolToIndexedTransitionListMap smallerIndex;
		SymbolToDoubleIndexedTransitionListMap biggerIndex;
		SymbolToTransitionListMap smallerLeaves, biggerLeaves;

		size_t symbolCnt = 0;
		std::unordered_map<typename Aut::SymbolType, size_t> symbolMap;
		Util::TranslatorWeak2<std::unordered_map<typename Aut::SymbolType, size_t>>
			symbolTranslator(
				symbolMap,
				[&symbolCnt](const typename Aut::SymbolType&){ return symbolCnt++; }
			);

		bottomUpIndex(
			smaller, smallerIndex, smallerLeaves, symbolTranslator
		);

		bottomUpIndex2(
			bigger, biggerIndex, biggerLeaves, symbolTranslator
		);


		typename Rel::IndexType ind, inv;

		preorder.buildIndex(ind, inv);

		return ExplicitUpwardInclusion::checkInternal(
			smallerLeaves,
			smallerIndex,
			smaller.GetFinalStates(),
			biggerLeaves,
			biggerIndex,
			bigger.GetFinalStates(),
			ind,
			inv
		);
	}

private:

	static bool checkInternal(
		const SymbolToTransitionListMap&                  smallerLeaves,
		const IndexedSymbolToIndexedTransitionListMap&    smallerIndex,
		const ExplicitTreeAutCore::FinalStateSet&         smallerFinalStates,
		const SymbolToTransitionListMap&                  biggerLeaves,
		const SymbolToDoubleIndexedTransitionListMap&     biggerIndex,
		const ExplicitTreeAutCore::FinalStateSet&         biggerFinalStates,
		const StateDiscontBinaryRelation::IndexType&      ind,
		const StateDiscontBinaryRelation::IndexType&      inv
	);
};

#endif
