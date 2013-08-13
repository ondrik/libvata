/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of intersection on BDD bottom-up tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

#include "bdd_bu_tree_aut_core.hh"

using VATA::AutBase;
using VATA::BDDBUTreeAutCore;
using VATA::Util::Convert;

BDDBUTreeAutCore BDDBUTreeAutCore::Intersection(
	const BDDBUTreeAutCore&        lhs,
	const BDDBUTreeAutCore&        rhs,
	AutBase::ProductTranslMap*     pTranslMap)
{
	typedef BDDBUTreeAutCore::StateType StateType;
	typedef BDDBUTreeAutCore::StateTuple StateTuple;
	typedef BDDBUTreeAutCore::StateSet StateSet;
	typedef std::pair<StateType, StateType> StatePair;
	typedef std::map<StateType, StatePair> WorkSetType;
	typedef AutBase::ProductTranslMap IntersectionTranslMap;
	typedef BDDBUTreeAutCore::TransMTBDD MTBDD;
	typedef VATA::Util::TranslatorWeak<IntersectionTranslMap> StateTranslator;

	GCC_DIAG_OFF(effc++)
	class IntersectionApplyFunctor :
		public VATA::MTBDDPkg::Apply2Functor<IntersectionApplyFunctor, StateSet,
		StateSet, StateSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // Private data members

		StateTranslator& transl_;

	public:   // Public methods

		IntersectionApplyFunctor(StateTranslator& transl) :
			transl_(transl)
		{ }

		StateSet ApplyOperation(const StateSet& lhs, const StateSet& rhs)
		{
			StateSet result;

			for (const StateType& lhsState : lhs)
			{
				for (const StateType& rhsState : rhs)
				{
					result.insert(transl_(std::make_pair(lhsState, rhsState)));
				}
			}

			return result;
		}
	};

	AutBase::ProductTranslMap translMap;
	if (pTranslMap == nullptr)
	{
		pTranslMap = &translMap;
	}

	BDDBUTreeAutCore result;
	WorkSetType workset;
	StateType stateCnt;

	StateTranslator stateTransl(*pTranslMap,
		[&workset,&stateCnt](const StatePair& newPair) -> StateType
		{
			workset.insert(std::make_pair(stateCnt, newPair));
			return stateCnt++;
		});

	IntersectionApplyFunctor isect(stateTransl);

	// start with leaves
	StateTuple tuple;
	MTBDD mtbdd = isect(lhs.GetMtbdd(tuple), rhs.GetMtbdd(tuple));
	result.SetMtbdd(tuple, mtbdd);

	while (!workset.empty())
	{	// while there is something in the workset
		WorkSetType::iterator itWs = workset.begin();
		const StateType& newState  = itWs->first;
		const StatePair& procPair  = itWs->second;
		const StateType& lhsState  = procPair.first;
		const StateType& rhsState  = procPair.second;

		if (lhs.IsStateFinal(lhsState) && rhs.IsStateFinal(rhsState))
		{	// set the state final if desirable
			result.SetStateFinal(newState);
		}

		for (auto lhsTupleBddPair : lhs.GetTransTable())
		{
			const StateTuple& lhsTuple = lhsTupleBddPair.first;
			const size_t& arity = lhsTuple.size();

			size_t firstMatch;
			for (firstMatch = 0; firstMatch < arity; ++firstMatch)
			{
				if (lhsTuple[firstMatch] == procPair.first)
				{
					break;
				}
			}

			if (firstMatch == arity)
			{	// if the first processed state is not present
				continue;
			}

			for (auto rhsTupleBddPair : rhs.GetTransTable())
			{
				const StateTuple& rhsTuple = rhsTupleBddPair.first;
				if (rhsTuple.size() != arity)
				{	// skip tuples of different size
					continue;
				}

				size_t i;
				for (i = firstMatch; i < arity; ++i)
				{
					if ((lhsTuple[i] == procPair.first) && (rhsTuple[i] == procPair.second))
					{
						break;
					}
				}

				if (i == arity)
				{	// if there was no match for the pair
					continue;
				}

				StateTuple tuple;
				for (size_t arityIndex = 0; arityIndex < arity; ++arityIndex)
				{	// check if respective states have product state
					if (arityIndex == i)
					{
						tuple.push_back(newState);
						continue;
					}

					IntersectionTranslMap::const_iterator itTable;
					if ((itTable = pTranslMap->find(std::make_pair(
						lhsTuple[arityIndex], rhsTuple[arityIndex]))) !=
						pTranslMap->end())
					{
						tuple.push_back(itTable->second);
					}
					else
					{
						break;
					}
				}

				if (tuple.size() == arity)
				{	// in case all positions match
					result.SetMtbdd(tuple,
						isect(lhsTupleBddPair.second, rhsTupleBddPair.second));
				}
			}
		}

		// remove the processed state from the workset
		workset.erase(itWs);
	}

	return result;
}
