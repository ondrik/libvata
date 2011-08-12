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

using VATA::BDDTreeAut;

// Standard library headers
#include <unordered_map>


template <>
BDDTreeAut VATA::RemoveUnreachableStates<BDDTreeAut>(const BDDTreeAut& aut)
{
	// Assertions
	assert(aut.isValid());

	typedef BDDTreeAut::StateType StateType;
	typedef BDDTreeAut::StateSet StateSet;
	typedef BDDTreeAut::StateTuple StateTuple;
	typedef BDDTreeAut::StateTupleSet StateTupleSet;
	typedef std::unordered_map<StateType, StateType> TranslMap;
	typedef std::map<StateType, StateType> WorkSetType;

	class UnreachableApplyFunctor :
		public VATA::MTBDDPkg::AbstractApply1Functor<StateTupleSet, StateTupleSet>
	{
	private:  // data members

		BDDTreeAut& resultAut_;
		TranslMap& translMap_;
		WorkSetType& workset_;

	private:  // methods

		UnreachableApplyFunctor(const UnreachableApplyFunctor&);
		UnreachableApplyFunctor& operator=(const UnreachableApplyFunctor&);

	public:   // methods

		UnreachableApplyFunctor(BDDTreeAut& resultAut, TranslMap& translMap,
			WorkSetType& workset) :
			resultAut_(resultAut),
			translMap_(translMap),
			workset_(workset)
		{ }

		virtual StateTupleSet ApplyOperation(const StateTupleSet& value)
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
					TranslMap::const_iterator itTransl;
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
	TranslMap translMap;

	UnreachableApplyFunctor unreach(result, translMap, workset);

	for (StateSet::const_iterator itFst = aut.GetFinalStates().begin();
		itFst != aut.GetFinalStates().end(); ++itFst)
	{	// start from all final states of the original automaton
		StateType state = result.AddState();
		result.SetStateFinal(state);
		workset.insert(std::make_pair(state, *itFst));
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

	assert(result.isValid());

	return result;
}
