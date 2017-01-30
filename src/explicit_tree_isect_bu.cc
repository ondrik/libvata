/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2015 Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of Intersection() on explicit tree automata
 *    in the bottom up way.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/util/transl_weak.hh>

// Standard library headers
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "explicit_tree_aut_core.hh"
#include "util/expl_bu_index.hh"

using VATA::ExplicitTreeAutCore;

ExplicitTreeAutCore ExplicitTreeAutCore::IntersectionBU(
	const ExplicitTreeAutCore&           lhs,
	const ExplicitTreeAutCore&           rhs,
	VATA::AutBase::ProductTranslMap*     pTranslMap)
{

	ExplicitTreeAutCore res(lhs.cache_);
	std::vector<const ProductTranslMap::value_type*> stack;

	VATA::AutBase::ProductTranslMap translMap;

	if (nullptr == pTranslMap)
	{
		pTranslMap = &translMap;
	}

	// Init auxiliary data structures
	IndexedSymbolToIndexedTransitionListMap lhsIndex, rhsIndex;
	SymbolToTransitionListMap lhsLeaves, rhsLeaves;
	size_t symbolCnt = 0;
	std::unordered_map<SymbolType, size_t> symbolMap;
	Util::TranslatorWeak2<std::unordered_map<SymbolType, size_t>>
		symbolTranslator(
			symbolMap,
			[&symbolCnt](const SymbolType& sym){ return sym; }
		);
	bottomUpIndex(
		lhs, lhsIndex, lhsLeaves, symbolTranslator
	);
	bottomUpIndex(
		rhs, rhsIndex, rhsLeaves, symbolTranslator
	);

	std::unordered_set<size_t> newStates;

	// process empty leaves
	for (const auto& lhsSymPair : lhsLeaves)
	{
		const SymbolType lhsSym = lhsSymPair.first;
		for (const auto& transLhs : lhsSymPair.second)
		{
			if (!rhsLeaves.count(lhsSym))
			{
				continue;
			}

			const auto& parentLhs = transLhs->state();
			
			for (const auto& transRhs : rhsLeaves.at(lhsSym))
			{
				const auto& parentRhs = transRhs->state();
				const auto productState = pTranslMap->insert(
					std::make_pair(std::make_pair(parentLhs, parentRhs), pTranslMap->size())
					).first;

				if (lhs.IsStateFinal(parentLhs) && rhs.IsStateFinal(parentRhs))
				{
					res.SetStateFinal(productState->second);
				}
				res.AddTransition(std::vector<size_t>(), symbolTranslator.at(lhsSym), productState->second);
				stack.push_back(&*productState);
			}
		}
	}
	

	while (!stack.empty())
	{
		const auto p = stack.back();
		stack.pop_back();

		if (newStates.count(p->second))
		{
			continue;
		}
		else
		{
			newStates.insert(p->second);
		}

		if (lhs.IsStateFinal(p->first.first) && rhs.IsStateFinal(p->first.second))
		{
			res.SetStateFinal(p->second);
		}

		const auto& leftCluster = ExplicitTreeAutCore::genericLookup(
			*lhs.transitions_, p->first.first);

		if (!leftCluster)
		{
			continue;
		}

		const auto& rightCluster = ExplicitTreeAutCore::genericLookup(
			*rhs.transitions_, p->first.second);

		if (!rightCluster)
		{
			continue;
		}

		assert(leftCluster);
		assert(rightCluster);

		const std::pair<size_t, size_t>& productState = p->first;
		if (!lhsIndex.count(productState.first)
				|| !rhsIndex.count(productState.second))
		{
			continue;
		}

		for (const auto& lhsSymPair : lhsIndex.at(productState.first))
		{
			const SymbolType lhsSym = lhsSymPair.first;
			for (size_t lhsStateIndex = 0;
					lhsStateIndex < lhsSymPair.second.size();
					++lhsStateIndex)
			{
				if (!rhsIndex.at(productState.second).count(lhsSym) 
						|| lhsStateIndex >= rhsIndex.at(productState.second).at(lhsSym).size())
				{ // left symbol or index of a state is not in the right TA
					continue;
				}

				const auto& rhsTransitions = rhsIndex.at(productState.second).at(
						lhsSym).at(lhsStateIndex);

				for (const auto& lhsTransPtr : lhsSymPair.second.at(lhsStateIndex))
				{
					const BUIndexTransition& lhsTrans = *lhsTransPtr;

					for (const auto& rhsTransPtr : rhsTransitions)
					{
						const BUIndexTransition& rhsTrans = *rhsTransPtr;
						
						const auto productInsertResult = pTranslMap->insert(
							std::make_pair(std::make_pair(lhsTrans.state(), rhsTrans.state()),
								pTranslMap->size()));
						const auto newProduct = productInsertResult.first;
						const auto isNewState = productInsertResult.second;

						assert(rhsTrans.children().size() == lhsTrans.children().size());
						
						bool allTupleInProduct = true;
						std::vector<size_t> newTuple;
						for (size_t tupleIndex = 0; tupleIndex < rhsTrans.children().size(); ++tupleIndex)
						{
							assert(lhsTrans.children().size() > 0 && rhsTrans.children().sze() > 0);
							const auto statePair = std::make_pair(lhsTrans.children().at(tupleIndex), rhsTrans.children().at(tupleIndex));
							const auto findResult = pTranslMap->find(statePair);

							const bool isSelfLoopToNewState =
									findResult != pTranslMap->end() &&
									isNewState && lhsTrans.children().at(tupleIndex) == lhsTrans.state()
									&& rhsTrans.children().at(tupleIndex) == rhsTrans.state();
							
							if (findResult == pTranslMap->end() || isSelfLoopToNewState)
							{
								allTupleInProduct = false;
								break;
							}
							else
							{
								newTuple.push_back(findResult->second);
							}
							assert(pTranslMap->at(statePair));
						}

						if (!allTupleInProduct)
						{
							if (isNewState)
							{
								pTranslMap->erase(
										std::make_pair(
												lhsTrans.state(),
												rhsTrans.state()));
							}
							continue;
						}

						res.AddTransition(newTuple, symbolTranslator.at(lhsSym), newProduct->second);
						stack.push_back(&*newProduct);
					}
				}
			}
		}
	}

	return res;
}
