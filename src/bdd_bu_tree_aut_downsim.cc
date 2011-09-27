/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Source file of computation of downward inclusion on bottom-up
 *    represented tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

using VATA::BDDBottomUpTreeAut;
using VATA::BDDTopDownTreeAut;
using VATA::Util::Convert;

typedef VATA::AutBase::StateBinaryRelation StateBinaryRelation;
typedef VATA::AutBase::StateType StateType;
typedef VATA::AutBase::StateToStateMap StateToStateMap;
typedef VATA::AutBase::StateToStateTranslator StateToStateTranslator;

typedef BDDTopDownTreeAut::StateTuple StateTuple;
typedef BDDTopDownTreeAut::StateTupleSet StateTupleSet;

typedef std::map<StateType, size_t> CounterElementMap;
typedef VATA::MTBDDPkg::OndriksMTBDD<CounterElementMap> CounterMTBDD;
typedef BDDTopDownTreeAut::TransMTBDD TopDownMTBDD;


StateBinaryRelation VATA::ComputeDownwardSimulation(
	const BDDBottomUpTreeAut& aut)
{
	BDDBottomUpTreeAut newAut;

	StateType stateCnt = 0;
	StateToStateMap stateMap;
	StateToStateTranslator stateTrans(stateMap,
		[&stateCnt](const StateType&){return stateCnt++;});

	aut.ReindexStates(newAut, stateTrans);

	return ComputeDownwardSimulation(newAut, stateCnt);
}

StateBinaryRelation VATA::ComputeDownwardSimulation(
	const BDDBottomUpTreeAut& aut, const size_t& states)
{
	GCC_DIAG_OFF(effc++)
	class InitCntApplyFctor :
		public VATA::MTBDDPkg::Apply2Functor<InitCntApplyFctor, StateTupleSet,
		CounterElementMap, CounterElementMap>
	{
	GCC_DIAG_ON(effc++)

	private:  // data members

		const StateType& state_;

	public:   // methods

		InitCntApplyFctor(const StateType& state) :
			state_(state)
		{ }

		CounterElementMap ApplyOperation(const StateTupleSet& lhs,
			const CounterElementMap& rhs)
		{
			CounterElementMap result = rhs;
			if (!result.insert(std::make_pair(state_, lhs.size())).second)
			{
				assert(false);
			}

			return result;
		}
	};

	class InitRefineApplyFctor :
		public VATA::MTBDDPkg::VoidApply2Functor<InitRefineApplyFctor, StateTupleSet,
		StateTupleSet>
	{

	private:  // data members

		bool& isSim_;

	public:   // methods

		InitRefineApplyFctor(bool& isSim) :
			isSim_(isSim)
		{ }

		void ApplyOperation(const StateTupleSet& lhs, const StateTupleSet& rhs)
		{
			if (!lhs.empty() && rhs.empty())
			{
				isSim_ = false;
				stopProcessing();
			}
		}
	};

	StateBinaryRelation result(states);

	BDDTopDownTreeAut topDownAut = aut.GetTopDownAut();

	CounterMTBDD initCnt((CounterElementMap()));

	StateType firstState;
	InitCntApplyFctor initCntFctor(firstState);

	bool isSim;
	InitRefineApplyFctor initRefFctor(isSim);

	for (auto firstStateBddPair : topDownAut.GetStates())
	{
		firstState = firstStateBddPair.first;
		const TopDownMTBDD& firstBdd = firstStateBddPair.second;

		initCnt = initCntFctor(firstBdd, initCnt);

		for (auto secondStateBddPair : topDownAut.GetStates())
		{
			const StateType& secondState = secondStateBddPair.first;
			const TopDownMTBDD& secondBdd = secondStateBddPair.second;
			isSim = true;
			initRefFctor(firstBdd, secondBdd);
			if (isSim)
			{
				assert(false);
			}
			else
			{
				assert(false);
			}
		}
	}

	return result;
}

