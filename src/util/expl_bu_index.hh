/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Bottom up transition cache.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_BU_INDEX_HH_
#define _VATA_EXPLICIT_TREE_BU_INDEX_HH_

#include <vector>
#include <unordered_map>

#include "../explicit_tree_aut_core.hh"

namespace VATA
{
	class BUIndexTransition;
}

namespace
{
	using StateType      = VATA::ExplicitTreeAutCore::StateType;
	using SymbolType     = VATA::ExplicitTreeAutCore::SymbolType;
	using StateTuple     = VATA::ExplicitTreeAutCore::StateTuple;
	using Transition     = VATA::BUIndexTransition;
}

namespace VATA
{
	typedef std::shared_ptr<BUIndexTransition> TransitionPtr;
	typedef std::vector<TransitionPtr> TransitionList;
	typedef std::vector<TransitionList> IndexedTransitionList;
	typedef std::vector<IndexedTransitionList> DoubleIndexedTransitionList;
	typedef std::unordered_map<SymbolType, TransitionList> SymbolToTransitionListMap;
	typedef std::unordered_map<SymbolType, IndexedTransitionList> SymbolToIndexedTransitionListMap;
	typedef std::unordered_map<SymbolType, DoubleIndexedTransitionList> SymbolToDoubleIndexedTransitionListMap;
	typedef std::unordered_map<StateType, SymbolToIndexedTransitionListMap> IndexedSymbolToIndexedTransitionListMap;


	template <class Aut, class SymbolIndex>
	static void bottomUpIndex(
		const Aut&                                  aut,
		IndexedSymbolToIndexedTransitionListMap&    bottomUpIndex,
		SymbolToTransitionListMap&                  leaves,
		SymbolIndex&                                symbolIndex);

	template <class Aut, class SymbolIndex>
	static void bottomUpIndex2(
		const Aut&                                  aut,
		SymbolToDoubleIndexedTransitionListMap&     bottomUpIndex,
		SymbolToTransitionListMap&                  leaves,
		SymbolIndex&                                symbolIndex);
}

	class VATA::BUIndexTransition
	{
	private:  // data members

		ExplicitTreeAutCore::TuplePtr children_;
		SymbolType symbol_;
		ExplicitTreeAutCore::StateType state_;

	public:

		BUIndexTransition(
			const ExplicitTreeAutCore::TuplePtr&    children,
			const SymbolType&                       symbol,
			const ExplicitTreeAutCore::StateType&   state) :
			children_(children),
			symbol_(symbol),
			state_(state)
		{ }

		const StateTuple& children() const
		{
			return *children_;
		}

		const SymbolType& symbol() const
		{
			return symbol_;
		}

		const StateType& state() const
		{
			return state_;
		}

		friend std::ostream& operator<<(
			std::ostream&         os,
			const BUIndexTransition&     t)
		{
			return os << t.symbol_
				<< Util::Convert::ToString(*t.children_) << "->" << t.state_;
		}
	};

	// TODO: rewrite using the DownAccessor?
	template <class Aut, class SymbolIndex>
	static void VATA::bottomUpIndex(
		const Aut&                                  aut,
		IndexedSymbolToIndexedTransitionListMap&    bottomUpIndex,
		SymbolToTransitionListMap&                  leaves,
		SymbolIndex&                                symbolIndex)
	{
		for (auto& stateClusterPair : *aut.GetTransitions())
		{
			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second)
			{
				assert(symbolTupleSetPair.second);
				assert(symbolTupleSetPair.second->size());

				SymbolType symbol = symbolIndex[symbolTupleSetPair.first];

				auto& first = *symbolTupleSetPair.second->begin();

				assert(first);

				if (first->empty())
				{
					auto& transitionList = leaves[symbol];

					for (auto& tuple : *symbolTupleSetPair.second)
					{
						assert(tuple);
						assert(tuple->empty());

						transitionList.push_back(
							TransitionPtr(new Transition(tuple, symbol, stateClusterPair.first))
						);
					}

					continue;
				}

				for (auto& tuple : *symbolTupleSetPair.second)
				{
					assert(tuple);
					assert(tuple->size());

					TransitionPtr transition(
						new Transition(tuple, symbol, stateClusterPair.first)
					);

					size_t i = 0;

					for (const StateType& state : *tuple)
					{
						auto& symbolIndexedTransitionList = bottomUpIndex[state];

						auto& indexedTransitionList = symbolIndexedTransitionList[symbol];

						if (indexedTransitionList.size() <= i)
						{
							indexedTransitionList.resize(i + 1);
						}

						indexedTransitionList[i].push_back(transition);

						++i;
					}
				}
			}
		}
	}

	template <class Aut, class SymbolIndex>
	static void VATA::bottomUpIndex2(
		const Aut&                                  aut,
		SymbolToDoubleIndexedTransitionListMap&     bottomUpIndex,
		SymbolToTransitionListMap&                  leaves,
		SymbolIndex&                                symbolIndex)
	{
		for (auto& stateClusterPair : *aut.GetTransitions())
		{
			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second)
			{
				assert(symbolTupleSetPair.second);
				assert(symbolTupleSetPair.second->size());

				SymbolType symbol = symbolIndex[symbolTupleSetPair.first];

				auto& first = *symbolTupleSetPair.second->begin();

				assert(first);

				if (first->empty())
				{
					auto& transitionList = leaves[symbol];

					for (auto& tuple : *symbolTupleSetPair.second)
					{
						assert(tuple);
						assert(tuple->empty());

						transitionList.push_back(
							TransitionPtr(new Transition(tuple, symbol, stateClusterPair.first))
						);
					}

					continue;
				}

				auto& doubleIndexedTransitionList = bottomUpIndex[symbol];

				if (doubleIndexedTransitionList.size() < first->size())
				{
					doubleIndexedTransitionList.resize(first->size());
				}

				for (auto& tuple : *symbolTupleSetPair.second)
				{
					assert(tuple);
					assert(tuple->size());

					TransitionPtr transition(
						new Transition(tuple, symbol, stateClusterPair.first)
					);

					size_t i = 0;

					for (const StateType& state : *tuple)
					{
						assert(i < doubleIndexedTransitionList.size());

						if (doubleIndexedTransitionList[i].size() <= state)
						{
							doubleIndexedTransitionList[i].resize(state + 1);
						}

						doubleIndexedTransitionList[i][state].push_back(transition);

						++i;
					}
				}
			}
		}
	}

#endif
