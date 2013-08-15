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

#include "bdd_td_tree_aut_core.hh"
#include "mtbdd/apply1func.hh"

using VATA::AutBase;
using VATA::BDDTDTreeAutCore;
using VATA::Util::Convert;

// Standard library headers
#include <stack>
#include <unordered_map>


BDDTDTreeAutCore BDDTDTreeAutCore::RemoveUnreachableStates() const
{
	typedef BDDTDTreeAutCore::StateType StateType;
	typedef BDDTDTreeAutCore::StateTuple StateTuple;
	typedef BDDTDTreeAutCore::StateTupleSet StateTupleSet;
	typedef std::stack<StateType, std::list<StateType>> WorkSetType;
	typedef BDDTDTreeAutCore::StateSet StateHT;


	GCC_DIAG_OFF(effc++)   // suppress missing virtual destructor warning
	class UnreachableApplyFunctor : public VATA::MTBDDPkg::Apply1Functor<
		UnreachableApplyFunctor,
		StateTupleSet,
		StateTupleSet>
	{
	GCC_DIAG_OFF(effc++)
	private:  // data members

		WorkSetType& workset_;
		StateHT& processed_;

	public:   // methods

		UnreachableApplyFunctor(
			WorkSetType&           workset,
			StateHT&               processed) :
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

	BDDTDTreeAutCore result;
	WorkSetType workset;
	StateHT processed;

	UnreachableApplyFunctor unreach(workset, processed);

	for (auto fst : this->GetFinalStates())
	{	// start from all final states of the original automaton
		result.SetStateFinal(fst);
		workset.push(fst);
	}

	while (!workset.empty())
	{	// while there is something in the workset
		StateType state = workset.top();
		workset.pop();

		result.SetMtbdd(state, unreach(this->GetMtbdd(state)));
	}

	return result;
}
