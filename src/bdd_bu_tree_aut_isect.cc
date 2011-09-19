/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of intersection on BDD bottom-up tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

using VATA::AutBase;
using VATA::BDDBottomUpTreeAut;

BDDBottomUpTreeAut VATA::Intersection(const BDDBottomUpTreeAut& lhs,
	const BDDBottomUpTreeAut& rhs, AutBase::ProductTranslMap* pTranslMap)
{
	// Assertions
	assert(lhs.isValid());
	assert(rhs.isValid());

	typedef BDDBottomUpTreeAut::StateType StateType;
	typedef BDDBottomUpTreeAut::StateTuple StateTuple;
	typedef BDDBottomUpTreeAut::StateSet StateSet;
	typedef std::pair<StateType, StateType> StatePair;
	typedef std::map<StateType, StatePair> WorkSetType;
	typedef AutBase::ProductTranslMap IntersectionTranslMap;
	typedef BDDBottomUpTreeAut::TransMTBDD MTBDD;
	typedef BDDBottomUpTreeAut::MTBDDHandle MTBDDHandle;
	typedef BDDBottomUpTreeAut::IndexValueArray IndexValueArray;

	GCC_DIAG_OFF(effc++)
	class IntersectionApplyFunctor :
		public VATA::MTBDDPkg::Apply2Functor<IntersectionApplyFunctor, StateSet,
		StateSet, StateSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // Private data members

		const BDDBottomUpTreeAut& lhsAut_;
		const BDDBottomUpTreeAut& rhsAut_;
		BDDBottomUpTreeAut& resultAut_;

		IntersectionTranslMap& translMap_;
		WorkSetType& workset_;

	private:  // Private methods

		IntersectionApplyFunctor(const IntersectionApplyFunctor&);
		IntersectionApplyFunctor& operator=(const IntersectionApplyFunctor&);

	public:   // Public methods

		IntersectionApplyFunctor(const BDDBottomUpTreeAut& lhsAut,
			const BDDBottomUpTreeAut& rhsAut, BDDBottomUpTreeAut& resultAut,
			IntersectionTranslMap& translMap, WorkSetType& workset) :
			lhsAut_(lhsAut),
			rhsAut_(rhsAut),
			resultAut_(resultAut),
			translMap_(translMap),
			workset_(workset)
		{ }

		StateSet ApplyOperation(const StateSet& lhs, const StateSet& rhs)
		{
			StateSet result;

			for (auto lhsState : lhs)
			{
				for (auto rhsState : rhs)
				{
					StatePair newPair = std::make_pair(lhsState, rhsState);

					StateType resultState;
					IntersectionTranslMap::const_iterator itTransl;
					if ((itTransl = translMap_.find(newPair)) != translMap_.end())
					{	// if the pair is already known
						resultState = itTransl->second;
					}
					else
					{	// if the pair is new
						resultState = resultAut_.AddState();
						translMap_.insert(std::make_pair(newPair, resultState));
						workset_.insert(std::make_pair(resultState, newPair));

						if (lhsAut_.IsStateFinal(lhsState) && rhsAut_.IsStateFinal(rhsState))
						{	// set the state final if desirable
							resultAut_.SetStateFinal(resultState);
						}
					}

					result.insert(resultState);
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


	BDDBottomUpTreeAut result;
	WorkSetType workset;

	IntersectionApplyFunctor isect(lhs, rhs, result, *pTranslMap, workset);

	// start with leaves
	StateTuple tuple;
	MTBDD mtbdd = isect(lhs.getMtbdd(tuple), rhs.getMtbdd(tuple));
	result.setMtbdd(tuple, mtbdd);

	while (!workset.empty())
	{	// while there is something in the workset
		WorkSetType::iterator itWs = workset.begin();
		const StatePair& procPair  = itWs->second;

		IndexValueArray lhsTuples =
			lhs.GetTuples().GetItemsWith(procPair.first, lhs.GetStates());

		IndexValueArray rhsTuples =
			rhs.GetTuples().GetItemsWith(procPair.second, rhs.GetStates());

		for (size_t arity = 0;
			(arity < lhsTuples.size()) && (arity < rhsTuples.size()); ++arity)
		{	// for each arity of left-hand side in LHS and RHS
			for (size_t lhsIndex = 0; lhsIndex < lhsTuples[arity].size(); ++lhsIndex)
			{	// for each left-hand side of given arity in LHS
				for (size_t rhsIndex = 0; rhsIndex < rhsTuples[arity].size(); ++rhsIndex)
				{	// for each left-hand side of given arity in RHS
					const StateTuple& lhsCandidate = lhsTuples[arity][lhsIndex].first;
					const StateTuple& rhsCandidate = rhsTuples[arity][rhsIndex].first;

					// Assertions
					assert(lhsCandidate.size() == arity);
					assert(rhsCandidate.size() == arity);

					StateTuple tuple;
					for (size_t arityIndex = 0; arityIndex < arity; ++arityIndex)
					{	// check if respective states have product state
						IntersectionTranslMap::const_iterator itTable;
						if ((itTable = pTranslMap->find(std::make_pair(
							lhsCandidate[arityIndex], rhsCandidate[arityIndex]))) !=
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
						const MTBDDHandle& lhsMtbdd = lhsTuples[arity][lhsIndex].second;
						const MTBDDHandle& rhsMtbdd = rhsTuples[arity][rhsIndex].second;

						result.setMtbdd(tuple,
							isect(lhs.getMtbdd(lhsMtbdd), rhs.getMtbdd(rhsMtbdd)));
					}
				}
			}
		}

		// remove the processed state from the workset
		workset.erase(itWs);
	}

	return result;
}
