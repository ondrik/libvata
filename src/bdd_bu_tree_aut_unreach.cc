/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of pruning unreachable states of bottom-up BDD tree
 *    automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

using VATA::AutBase;
using VATA::BDDBottomUpTreeAut;
using VATA::Util::Convert;

typedef VATA::AutBase::StateType StateType;
typedef VATA::BDDBottomUpTreeAut::StateSet StateSet;
typedef VATA::BDDBottomUpTreeAut::StateHT StateHT;
typedef VATA::BDDBottomUpTreeAut::StateTuple StateTuple;
typedef VATA::BDDBottomUpTreeAut::TransMTBDD TransMTBDD;

typedef std::unordered_map<StateTuple, TransMTBDD, boost::hash<StateTuple>>
	TupleHT;


GCC_DIAG_OFF(effc++)
class ReachableCollectorFctor :
	public VATA::MTBDDPkg::VoidApply1Functor<ReachableCollectorFctor,
	StateSet>
{
GCC_DIAG_ON(effc++)

private:  // data members

	StateHT& reachable_;
	StateHT& workset_;

public:   // methods

	ReachableCollectorFctor(StateHT& reachable, StateHT& workset) :
		reachable_(reachable),
		workset_(workset)
	{ }

	inline void ApplyOperation(const StateSet& value)
	{
		for (const StateType& state : value)
		{
			if (reachable_.insert(state).second)
			{	// if the value was inserted
				if (!workset_.insert(state).second)
				{	// if it is already in the workset
					assert(false);     // fail gracefully
				}
			}
		}
	}
};


BDDBottomUpTreeAut VATA::RemoveUnreachableStates(const BDDBottomUpTreeAut& aut)
{
	BDDBottomUpTreeAut result;


	StateHT reachable;
	StateHT workset;

	TupleHT tuples;

	for (auto tupleBddPair : aut.GetTransTable())
	{
		tuples.insert(tupleBddPair);
	}

	tuples.erase(StateTuple());

	ReachableCollectorFctor reachFunc(reachable, workset);

	const TransMTBDD& nullaryBdd = aut.GetMtbdd(StateTuple());
	reachFunc(nullaryBdd);
	result.SetMtbdd(StateTuple(), nullaryBdd);

	while (!workset.empty())
	{
		StateType state = *(workset.begin());
		workset.erase(workset.begin());

		TupleHT::const_iterator itTup = tuples.begin();
		while (itTup != tuples.end())
		{
			const StateTuple& tuple = itTup->first;

			if (std::find(tuple.begin(), tuple.end(), state) != tuple.end())
			{	// if the state is there
				size_t i;
				for (i = 0; i < tuple.size(); ++i)
				{
					if (reachable.find(tuple[i]) == reachable.end())
					{
						break;
					}
				}

				if (i == tuple.size())
				{	// in case all states are reachable

					// collect reachable states
					reachFunc(itTup->second);

					result.SetMtbdd(tuple, itTup->second);

					// remove the tuple from the set of tuples
					decltype(itTup) tmpIt = itTup;
					++itTup;
					tuples.erase(tmpIt);
					continue;
				}
			}

			++itTup;
		}
	}

	for (const StateType& fst : aut.GetFinalStates())
	{
		if (reachable.find(fst) != reachable.end())
		{
			result.SetStateFinal(fst);
		}
	}

	return result;
}
