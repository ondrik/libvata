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

#include <vata/explicit_tree_aut.hh>

namespace VATA {

	class ExplicitUpwardInclusion;

}

class VATA::ExplicitUpwardInclusion {

	class Transition {

		Explicit::TuplePtr children_;
		size_t symbol_;
		Explicit::StateType state_;

	public:

		Transition(const Explicit::TuplePtr& children, const size_t& symbol,
			const Explicit::StateType& state) : children_(children), symbol_(symbol), state_(state)
			{}

		const Explicit::StateTuple& children() const { return *this->children_; }
		const size_t& symbol() const { return this->symbol_; }
		const Explicit::StateType& state() const { return this->state_; }

		friend std::ostream& operator<<(std::ostream& os, const Transition& t) {

			return os << t.symbol_ << Util::Convert::ToString(*t.children_) << "->" << t.state_;

		}

	};

	typedef std::shared_ptr<Transition> TransitionPtr;
	typedef std::vector<TransitionPtr> TransitionList;
	typedef std::vector<TransitionList> IndexedTransitionList;
	typedef std::vector<IndexedTransitionList> DoubleIndexedTransitionList;
	typedef std::vector<TransitionList> SymbolToTransitionListMap;
	typedef std::vector<IndexedTransitionList> SymbolToIndexedTransitionListMap;
	typedef std::vector<DoubleIndexedTransitionList> SymbolToDoubleIndexedTransitionListMap;
	typedef std::vector<SymbolToIndexedTransitionListMap> IndexedSymbolToIndexedTransitionListMap;

	template <class Aut, class SymbolIndex>
	static void bottomUpIndex(const Aut& aut,
		IndexedSymbolToIndexedTransitionListMap& bottomUpIndex,
		SymbolToTransitionListMap& leaves, SymbolIndex& symbolIndex) {

		for (auto& stateClusterPair : *aut.transitions_) {

			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);
				assert(symbolTupleSetPair.second->size());

				auto& symbol = symbolIndex[symbolTupleSetPair.first];

				auto& first = *symbolTupleSetPair.second->begin();

				assert(first);

				if (first->empty()) {

					if (leaves.size() <= symbol)
						leaves.resize(symbol + 1);

					auto& transitionList = leaves[symbol];

					for (auto& tuple : *symbolTupleSetPair.second) {

						assert(tuple);
						assert(tuple->empty());

						transitionList.push_back(
							TransitionPtr(new Transition(tuple, symbol, stateClusterPair.first))
						);

					}

					continue;

				}

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);
					assert(tuple->size());

					TransitionPtr transition(
						new Transition(tuple, symbol, stateClusterPair.first)
					);

					size_t i = 0;

					for (auto& state : *tuple) {

						if (bottomUpIndex.size() <= state)
							bottomUpIndex.resize(state + 1);

						auto& symbolIndexedTransitionList = bottomUpIndex[state];

						if (symbolIndexedTransitionList.size() <= symbol)
							symbolIndexedTransitionList.resize(symbol + 1);

						auto& indexedTransitionList = symbolIndexedTransitionList[symbol];

						if (indexedTransitionList.size() <= i)
							indexedTransitionList.resize(i + 1);

						indexedTransitionList[i].push_back(transition);

						++i;

					}

				}

			}

		}

	}

	template <class Aut, class SymbolIndex>
	static void bottomUpIndex2(const Aut& aut,
		SymbolToDoubleIndexedTransitionListMap& bottomUpIndex,
		SymbolToTransitionListMap& leaves, SymbolIndex& symbolIndex) {

		for (auto& stateClusterPair : *aut.transitions_) {

			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);
				assert(symbolTupleSetPair.second->size());

				auto& symbol = symbolIndex[symbolTupleSetPair.first];

				auto& first = *symbolTupleSetPair.second->begin();

				assert(first);

				if (first->empty()) {

					if (leaves.size() <= symbol)
						leaves.resize(symbol + 1);

					auto& transitionList = leaves[symbol];

					for (auto& tuple : *symbolTupleSetPair.second) {

						assert(tuple);
						assert(tuple->empty());

						transitionList.push_back(
							TransitionPtr(new Transition(tuple, symbol, stateClusterPair.first))
						);

					}

					continue;

				}

				if (bottomUpIndex.size() <= symbol)
					bottomUpIndex.resize(symbol + 1);

				auto& doubleIndexedTransitionList = bottomUpIndex[symbol];

				if (doubleIndexedTransitionList.size() < first->size())
					doubleIndexedTransitionList.resize(first->size());

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);
					assert(tuple->size());

					TransitionPtr transition(
						new Transition(tuple, symbol, stateClusterPair.first)
					);

					size_t i = 0;

					for (auto& state : *tuple) {

						assert(i < doubleIndexedTransitionList.size());

						if (doubleIndexedTransitionList[i].size() <= state)
							doubleIndexedTransitionList[i].resize(state + 1);

						doubleIndexedTransitionList[i][state].push_back(transition);

						++i;

					}

				}

			}

		}

	}

public:

	template <class Aut, class States, class Rel>
	static bool Check(const Aut& smaller, const States& sStates, const Aut& bigger,
		const States& bStates, const Rel& preorder) {

		IndexedSymbolToIndexedTransitionListMap sIndex;
		SymbolToDoubleIndexedTransitionListMap bIndex;
		SymbolToTransitionListMap sLeaves, bLeaves;

		size_t symbolCnt = 0;
		std::unordered_map<typename Aut::SymbolType, size_t> symbolMap;
		Util::TranslatorWeak2<std::unordered_map<typename Aut::SymbolType, size_t>>
			symbolTranslator(
				symbolMap,
				[&symbolCnt](const typename Aut::SymbolType&){ return symbolCnt++; }
			);

		ExplicitUpwardInclusion::bottomUpIndex(
			smaller, sIndex, sLeaves, symbolTranslator
		);

		ExplicitUpwardInclusion::bottomUpIndex2(
			bigger, bIndex, bLeaves, symbolTranslator
		);

		std::vector<std::vector<size_t>> ind, inv;

		preorder.buildIndex(ind, inv);

		return ExplicitUpwardInclusion::checkInternal(
			sLeaves, sIndex, sStates, bLeaves, bIndex, bStates, ind, inv
		);

	}

private:

	static bool checkInternal(
		const SymbolToTransitionListMap& smallerLeaves,
		const IndexedSymbolToIndexedTransitionListMap& smallerIndex,
		const Explicit::StateSet& smallerFinalStates,
		const SymbolToTransitionListMap& biggerLeaves,
		const SymbolToDoubleIndexedTransitionListMap& biggerIndex,
		const Explicit::StateSet& biggerFinalStates,
		const std::vector<std::vector<size_t>>& ind,
		const std::vector<std::vector<size_t>>& inv
	);

};

#endif
