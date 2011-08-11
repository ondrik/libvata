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

using VATA::BDDTreeAut;

template <>
BDDTreeAut VATA::Intersection<BDDTreeAut>(const BDDTreeAut& lhs,
	const BDDTreeAut& rhs)
{
	// Assertions
	assert(lhs.isValid());
	assert(rhs.isValid());

	// TODO: substitute translation map for a two-way dict?

	typedef BDDTreeAut::StateType StateType;
	typedef BDDTreeAut::StateSet StateSet;
	typedef BDDTreeAut::StateTuple StateTuple;
	typedef BDDTreeAut::StateTupleSet StateTupleSet;
	typedef std::pair<StateType, StateType> StatePair;
	typedef std::tr1::unordered_map<StatePair, StateType, boost::hash<StatePair> >
		IntersectionTranslMap;
	typedef std::map<StateType, StatePair> WorkSetType;

	class IntersectionApplyFunctor :
		public VATA::MTBDDPkg::AbstractApply2Functor<StateTupleSet, StateTupleSet,
		StateTupleSet>
	{
	private:  // data members

		BDDTreeAut* pResultAut_;
		IntersectionTranslMap* pTranslMap_;
		WorkSetType* pWorkset_;

	private:  // methods

		IntersectionApplyFunctor(const IntersectionApplyFunctor&);
		IntersectionApplyFunctor& operator=(const IntersectionApplyFunctor&);

	public:   // methods

		IntersectionApplyFunctor(BDDTreeAut* pResultAut,
			IntersectionTranslMap* pTranslMap, WorkSetType* pWorkset) :
			pResultAut_(pResultAut),
			pTranslMap_(pTranslMap),
			pWorkset_(pWorkset)
		{
			// Assertions
			assert(pResultAut != static_cast<BDDTreeAut*>(0));
			assert(pTranslMap_ != static_cast<IntersectionTranslMap*>(0));
			assert(pWorkset_ != static_cast<WorkSetType*>(0));
		}

		virtual StateTupleSet ApplyOperation(const StateTupleSet& lhs,
			const StateTupleSet& rhs)
		{
			// Assertions
			assert(pResultAut_ != static_cast<BDDTreeAut*>(0));
			assert(pTranslMap_ != static_cast<IntersectionTranslMap*>(0));
			assert(pWorkset_ != static_cast<WorkSetType*>(0));

			StateTupleSet result;

			for (StateTupleSet::const_iterator itLhs = lhs.begin();
				itLhs != lhs.end(); ++itLhs)
			{	// for each tuple from LHS
				for (StateTupleSet::const_iterator itRhs = rhs.begin();
					itRhs != rhs.end(); ++itRhs)
				{	// for each tuple from RHS
					assert(itLhs->size() == itRhs->size());

					StateTuple resultTuple;
					for (size_t i = 0; i < itLhs->size(); ++i)
					{	// for each position in the tuples
						StatePair newPair = std::make_pair((*itLhs)[i], (*itRhs)[i]);

						StateType state;
						IntersectionTranslMap::const_iterator itTransl;
						if ((itTransl = pTranslMap_->find(newPair)) != pTranslMap_->end())
						{	// if the pair is already known
							state = itTransl->second;
						}
						else
						{	// if the pair is new
							state = pResultAut_->AddState();
							pTranslMap_->insert(std::make_pair(newPair, state));
							pWorkset_->insert(std::make_pair(state, newPair));
						}

						resultTuple.push_back(state);
					}

					result.insert(resultTuple);
				}
			}

			return result;
		}
	};

	BDDTreeAut result;
	WorkSetType workset;
	IntersectionTranslMap translMap;

	for (StateSet::const_iterator itFstLhs = lhs.GetFinalStates().begin();
		itFstLhs != lhs.GetFinalStates().end(); ++itFstLhs)
	{	// iterate over LHS's final states
		for (StateSet::const_iterator itFstRhs = rhs.GetFinalStates().begin();
			itFstRhs != rhs.GetFinalStates().end(); ++itFstRhs)
		{	// iterate over RHS's final states
			StatePair origStates = std::make_pair(*itFstLhs, *itFstRhs);

			StateType newState = result.AddState();
			result.SetStateFinal(newState);

			workset.insert(std::make_pair(newState, origStates));
			translMap.insert(std::make_pair(origStates, newState));
		}
	}

	IntersectionApplyFunctor isect(&result, &translMap, &workset);

	while (!workset.empty())
	{	// while there is something in the workset
		WorkSetType::iterator itWs = workset.begin();
		const StatePair& procPair  = itWs->second;
		const StateType& procState = itWs->first;

		BDDTreeAut::TransMTBDD mtbdd = isect(lhs.getMtbdd(procPair.first),
			rhs.getMtbdd(procPair.second));

		result.setMtbdd(procState, mtbdd);

		// remove the processed state from the workset
		workset.erase(itWs);
	}

	assert(result.isValid());

	return result;
}
