/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of intersection on BDD top-down tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

#include "bdd_td_tree_aut_core.hh"

using VATA::AutBase;
using VATA::BDDTDTreeAutCore;

// Standard library headers
#include <unordered_map>

BDDTDTreeAutCore BDDTDTreeAutCore::Intersection(
	const BDDTDTreeAutCore&       lhs,
	const BDDTDTreeAutCore&       rhs,
	AutBase::ProductTranslMap*    pTranslMap)
{
	typedef BDDTDTreeAutCore::StateType StateType;
	typedef BDDTDTreeAutCore::StateTuple StateTuple;
	typedef BDDTDTreeAutCore::StateTupleSet StateTupleSet;
	typedef std::pair<StateType, StateType> StatePair;
	typedef std::map<StateType, StatePair> WorkSetType;
	typedef VATA::Util::TranslatorWeak<AutBase::ProductTranslMap> StateTranslator;

	GCC_DIAG_OFF(effc++)  // suppress non-virtual destructor warning
	class IntersectionApplyFunctor :
		public VATA::MTBDDPkg::Apply2Functor<IntersectionApplyFunctor, StateTupleSet,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // data members

		StateTranslator& transl_;

	public:   // methods

		IntersectionApplyFunctor(StateTranslator& transl) :
			transl_(transl)
		{ }

		StateTupleSet ApplyOperation(const StateTupleSet& lhs,
			const StateTupleSet& rhs)
		{
			StateTupleSet result;

			for (auto lhsTuple : lhs)
			{
				for (auto rhsTuple : rhs)
				{
					assert(lhsTuple.size() == rhsTuple.size());

					StateTuple resultTuple;
					for (size_t i = 0; i < lhsTuple.size(); ++i)
					{
						resultTuple.push_back(transl_(
							std::make_pair(lhsTuple[i], rhsTuple[i])));
					}

					result.insert(resultTuple);
				}
			}

			return result;
		}
	};

	AutBase::ProductTranslMap translMap;
	if (pTranslMap == nullptr)
	{	// if no translation map was given
		pTranslMap = &translMap;
	}

	BDDTDTreeAutCore result;
	WorkSetType workset;
	StateType stateCnt;

	StateTranslator stateTransl(*pTranslMap,
		[&workset,&stateCnt](const StatePair& newPair) -> StateType
		{
			workset.insert(std::make_pair(stateCnt, newPair));
			return stateCnt++;
		});

	for (auto lhsFst : lhs.GetFinalStates())
	{	// iterate over LHS's final states
		for (auto rhsFst : rhs.GetFinalStates())
		{	// iterate over RHS's final states
			StatePair origStates = std::make_pair(lhsFst, rhsFst);

			StateType newState = stateTransl(origStates);
			result.SetStateFinal(newState);
		}
	}

	IntersectionApplyFunctor isect(stateTransl);

	while (!workset.empty())
	{	// while there is something in the workset
		WorkSetType::iterator itWs = workset.begin();
		const StatePair& procPair  = itWs->second;
		const StateType& procState = itWs->first;

		BDDTDTreeAutCore::TransMTBDD mtbdd = isect(lhs.GetMtbdd(procPair.first),
			rhs.GetMtbdd(procPair.second));

		result.SetMtbdd(procState, mtbdd);

		// remove the processed state from the workset
		workset.erase(itWs);
	}

	return result;
}
