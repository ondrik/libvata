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


	GCC_DIAG_OFF(effc++)   // suppress missing virtual destructor warning
	class UnreachableApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<UnreachableApplyFunctor,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_OFF(effc++)
	private:  // data members

		BDDTopDownTreeAut& resultAut_;
		StateToStateMap& translMap_;
		WorkSetType& workset_;

	public:   // methods

		UnreachableApplyFunctor(BDDTopDownTreeAut& resultAut, StateToStateMap& translMap,
			WorkSetType& workset) :
			resultAut_(resultAut),
			translMap_(translMap),
			workset_(workset)
		{ }

		StateTupleSet ApplyOperation(const StateTupleSet& value)
		{
			StateTupleSet result;

			for (StateTupleSet::const_iterator itVal = value.begin();
				itVal != value.end(); ++itVal)
			{	// for each tuple from the leaf
				StateTuple resultTuple;
				for (size_t i = 0; i < itVal->size(); ++i)
				{	// for each position in the tuple
					const StateType& state = (*itVal)[i];

					StateType newState;
					StateToStateMap::const_iterator itTransl;
					if ((itTransl = translMap_.find(state)) != translMap_.end())
					{	// if the pair is already known
						newState = itTransl->second;
					}
					else
					{	// if the pair is new
						newState = resultAut_.AddState();
						translMap_.insert(std::make_pair(state, newState));
						workset_.insert(std::make_pair(newState, state));
					}

					resultTuple.push_back(newState);
				}

				result.insert(resultTuple);
			}

			return result;
		}
	};

	BDDTopDownTreeAut result(aut.GetTransTable());
	WorkSetType workset;

	bool deleteTranslMap = false;
	if (pTranslMap == nullptr)
	{	// in case the state translation map was not provided
		pTranslMap = new StateToStateMap();
		deleteTranslMap = true;
	}

	assert(pTranslMap->empty());

	UnreachableApplyFunctor unreach(result, *pTranslMap, workset);

	for (auto fst : aut.GetFinalStates())
	{	// start from all final states of the original automaton
		StateType newState = result.AddState();
		result.SetStateFinal(newState);
		workset.insert(std::make_pair(newState, fst));
		pTranslMap->insert(std::make_pair(fst, newState));
	}

	while (!workset.empty())
	{	// while there is something in the workset
		WorkSetType::iterator itWs = workset.begin();
		const StateType& newState = itWs->first;
		const StateType& oldState = itWs->second;

		BDDTopDownTreeAut::TransMTBDD mtbdd = unreach(aut.getMtbdd(oldState));

		result.setMtbdd(newState, mtbdd);

		workset.erase(itWs);
	}

	if (deleteTranslMap)
	{	// in case we need to delete the map
		delete pTranslMap;
	}

	assert(result.isValid());

	return result;
}
