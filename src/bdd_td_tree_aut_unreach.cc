/*****************************************************************************
 *  VATA Tree Automata Library
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
using VATA::Util::Convert;

// Standard library headers
#include <stack>
#include <unordered_map>


BDDTopDownTreeAut VATA::RemoveUnreachableStates(const BDDTopDownTreeAut& aut)
{
	typedef BDDTopDownTreeAut::StateType StateType;
	typedef BDDTopDownTreeAut::StateSet StateSet;
	typedef BDDTopDownTreeAut::StateTuple StateTuple;
	typedef BDDTopDownTreeAut::StateTupleSet StateTupleSet;
	typedef AutBase::StateToStateMap StateToStateMap;
	typedef std::stack<StateType, std::list<StateType>> WorkSetType;
	typedef BDDTopDownTreeAut::StateSet StateHT;


	GCC_DIAG_OFF(effc++)   // suppress missing virtual destructor warning
	class UnreachableApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<UnreachableApplyFunctor,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_OFF(effc++)
	private:  // data members

		WorkSetType& workset_;
		StateHT& processed_;

	public:   // methods

		UnreachableApplyFunctor(WorkSetType& workset, StateHT& processed) :
			workset_(workset),
			processed_(processed)
		{ }

		StateTupleSet ApplyOperation(const StateTupleSet& value)
		{
			for (const StateTuple& tuple : value)
			{	// for each tuple from the leaf
				for (const StateType& state : tuple)
				{	// for each position in the tuple
					if (processed_.insert(state).second)
					{
						workset_.push(state);
					}
				}
			}

			return value;
		}
	};

	BDDTopDownTreeAut result;
	WorkSetType workset;
	StateHT processed;

	UnreachableApplyFunctor unreach(workset, processed);

	for (auto fst : aut.GetFinalStates())
	{	// start from all final states of the original automaton
		result.SetStateFinal(fst);
		workset.push(fst);
	}

	while (!workset.empty())
	{	// while there is something in the workset
		StateType state = workset.top();
		workset.pop();

		result.SetMtbdd(state, unreach(aut.GetMtbdd(state)));
	}

	return result;
}
