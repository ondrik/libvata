/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Downward inclusion for explicitly represented tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_INCL_DOWN_HH_
#define _VATA_EXPLICIT_TREE_INCL_DOWN_HH_

#include <vata/explicit_tree_aut.hh>

namespace VATA {

	class ExplicitDownwardInclusion;

}

class VATA::ExplicitDownwardInclusion {

public:

	typedef std::vector<const Explicit::StateTuple*> TupleList;
	typedef std::vector<TupleList> IndexedTupleList;
	typedef std::vector<IndexedTupleList> DoubleIndexedTupleList;

private:

	template <class Aut, class SymbolIndex>
	static void topDownIndex(const Aut& aut, DoubleIndexedTupleList& topDownIndex,
		SymbolIndex& symbolIndex) {

		for (auto& stateClusterPair : *aut.transitions_) {

			assert(stateClusterPair.second);

			if (stateClusterPair.first >= topDownIndex.size())
				topDownIndex.resize(stateClusterPair.first + 1);

			auto& indexedTupleList = topDownIndex[stateClusterPair.first];

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);
				assert(symbolTupleSetPair.second->size());

				auto& symbol = symbolIndex[symbolTupleSetPair.first];

				if (symbol >= indexedTupleList.size())
					indexedTupleList.resize(symbol + 1);

				auto& tupleList = indexedTupleList[symbol];

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);

					tupleList.push_back(tuple.get());

				}

			}

		}

	}

public:

	template <class Aut, class Rel>
	static bool Check(const Aut& smaller, const Aut& bigger, const Rel& preorder) {

		DoubleIndexedTupleList smallerIndex, biggerIndex;

		size_t symbolCnt = 0;
		std::unordered_map<typename Aut::SymbolType, size_t> symbolMap;
		Util::TranslatorWeak2<std::unordered_map<typename Aut::SymbolType, size_t>>
			symbolTranslator(
				symbolMap,
				[&symbolCnt](const typename Aut::SymbolType&){ return symbolCnt++; }
			);

		ExplicitDownwardInclusion::topDownIndex(smaller, smallerIndex, symbolTranslator);
		ExplicitDownwardInclusion::topDownIndex(bigger, biggerIndex, symbolTranslator);

		std::vector<std::vector<size_t>> ind, inv;

		preorder.buildIndex(ind, inv);

		return ExplicitDownwardInclusion::checkInternal(
			smallerIndex, smaller.GetFinalStates(), biggerIndex, bigger.GetFinalStates(), ind, inv
		);

	}
/*
	template <class Aut, class Rel>
	static bool CheckOpt(const Aut& smaller, const Aut& bigger, const Rel& preorder) {

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

		ExplicitUpwardInclusion::bottomUpIndex(
			smaller, smallerIndex, smallerLeaves, symbolTranslator
		);

		ExplicitUpwardInclusion::bottomUpIndex2(
			bigger, biggerIndex, biggerLeaves, symbolTranslator
		);

		std::vector<std::vector<size_t>> ind, inv;

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
*/
private:

	static bool checkInternal(
		const DoubleIndexedTupleList& smallerIndex,
		const Explicit::StateSet& smallerFinalStates,
		const DoubleIndexedTupleList& biggerIndex,
		const Explicit::StateSet& biggerFinalStates,
		const std::vector<std::vector<size_t>>& ind,
		const std::vector<std::vector<size_t>>& inv
	);
/*
	static bool checkInternalOpt(
		const SymbolToTransitionListMap& smallerLeaves,
		const IndexedSymbolToIndexedTransitionListMap& smallerIndex,
		const Explicit::StateSet& smallerFinalStates,
		const SymbolToTransitionListMap& biggerLeaves,
		const SymbolToDoubleIndexedTransitionListMap& biggerIndex,
		const Explicit::StateSet& biggerFinalStates,
		const std::vector<std::vector<size_t>>& ind,
		const std::vector<std::vector<size_t>>& inv
	);
*/
};

#endif
