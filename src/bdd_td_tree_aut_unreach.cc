/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of pruning unreachable states of top-down BDD tree
 *    automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_td_tree_aut_op.hh>
#include <vata/mtbdd/apply1func.hh>

using VATA::AutBase;
using VATA::BDDTopDownTreeAut;

// Standard library headers
#include <unordered_map>


BDDTopDownTreeAut VATA::RemoveUnreachableStates(const BDDTopDownTreeAut& aut,
	AutBase::StateToStateMap* pTranslMap)
{
	// Assertions
	assert(aut.isValid());

	typedef BDDTopDownTreeAut::StateType StateType;
	typedef BDDTopDownTreeAut::StateSet StateSet;
	typedef BDDTopDownTreeAut::StateTuple StateTuple;
	typedef BDDTopDownTreeAut::StateTupleSet StateTupleSet;
	typedef AutBase::StateToStateMap StateToStateMap;
	typedef std::map<StateType, StateType> WorkSetType;
	typedef AutBase::StateToStateTranslator StateTranslator;


	GCC_DIAG_OFF(effc++)   // suppress missing virtual destructor warning
	class UnreachableApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<UnreachableApplyFunctor,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_OFF(effc++)
	private:  // data members

		StateTranslator& trans_;

	public:   // methods

		UnreachableApplyFunctor(StateTranslator& trans) :
			trans_(trans)
		{ }

		StateTupleSet ApplyOperation(const StateTupleSet& value)
		{
			StateTupleSet result;

			for (const StateTuple& tuple : value)
			{	// for each tuple from the leaf
				StateTuple resultTuple;
				for (StateTuple::const_iterator itTup = tuple.begin();
					itTup != tuple.end(); ++itTup)
				{	// for each position in the tuple
					resultTuple.push_back(trans_(*itTup));
				}

				result.insert(resultTuple);
			}

			return result;
		}
	};

	StateToStateMap translMap;
	if (pTranslMap == nullptr)
	{	// in case the state translation map was not provided
		pTranslMap = &translMap;
	}

	assert(pTranslMap->empty());


	BDDTopDownTreeAut result(aut.GetTransTable());
	WorkSetType workset;
	StateType stateCnt = 0;

	StateTranslator stateTransl(*pTranslMap,
		[&workset,&stateCnt](const StateType& newState) -> StateType
		{
			workset.insert(std::make_pair(newState, stateCnt));
			return stateCnt++;
		});

	UnreachableApplyFunctor unreach(stateTransl);

	for (auto fst : aut.GetFinalStates())
	{	// start from all final states of the original automaton
		result.SetStateFinal(stateTransl(fst));
	}

	while (!workset.empty())
	{	// while there is something in the workset
		WorkSetType::iterator itWs = workset.begin();
		const StateType& newState = itWs->first;
		const StateType& oldState = itWs->second;

		result.SetMtbdd(newState, unreach(aut.GetMtbdd(oldState)));

		workset.erase(itWs);
	}

	assert(result.isValid());
	return result;
}
