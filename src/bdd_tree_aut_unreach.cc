/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of intersection on BDD tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_tree_aut_op.hh>
#include <vata/mtbdd/apply1func.hh>

using VATA::AutBase;
using VATA::BDDTreeAut;

// Standard library headers
#include <unordered_map>


template <>
BDDTreeAut VATA::RemoveUnreachableStates<BDDTreeAut>(const BDDTreeAut& aut,
	AutBase::StateToStateMap* pTranslMap)
{
	// Assertions
	assert(aut.isValid());

	typedef BDDTreeAut::StateType StateType;
	typedef BDDTreeAut::StateSet StateSet;
	typedef BDDTreeAut::StateTuple StateTuple;
	typedef BDDTreeAut::StateTupleSet StateTupleSet;
	typedef AutBase::StateToStateMap StateToStateMap;
	typedef std::map<StateType, StateType> WorkSetType;


	GCC_DIAG_OFF(effc++)   // suppress missing virtual destructor warning
	class UnreachableApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<UnreachableApplyFunctor,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_OFF(effc++)
	private:  // data members

		BDDTreeAut& resultAut_;
		StateToStateMap& translMap_;
		WorkSetType& workset_;

	public:   // methods

		UnreachableApplyFunctor(BDDTreeAut& resultAut, StateToStateMap& translMap,
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

	BDDTreeAut result(aut.GetTransTable());
	WorkSetType workset;

	bool deleteTranslMap = false;
	if (pTranslMap == nullptr)
	{	// in case the state translation map was not provided
		pTranslMap = new StateToStateMap();
		deleteTranslMap = true;
	}

	assert(pTranslMap->empty());

	UnreachableApplyFunctor unreach(result, *pTranslMap, workset);

	for (auto itFst = aut.GetFinalStates().cbegin();
		itFst != aut.GetFinalStates().cend(); ++itFst)
	{	// start from all final states of the original automaton
		StateType newState = result.AddState();
		result.SetStateFinal(newState);
		workset.insert(std::make_pair(newState, *itFst));
		pTranslMap->insert(std::make_pair(*itFst, newState));
	}

	while (!workset.empty())
	{	// while there is something in the workset
		WorkSetType::iterator itWs = workset.begin();
		const StateType& newState = itWs->first;
		const StateType& oldState = itWs->second;

		BDDTreeAut::TransMTBDD mtbdd = unreach(aut.getMtbdd(oldState));

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
