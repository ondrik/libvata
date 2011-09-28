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

	StateHT reachable_;
	StateHT workset_;

public:   // methods

	ReachableCollectorFctor(StateHT& reachable, StateHT& workset) :
		reachable_(reachable),
		workset_(workset)
	{ }

	void ApplyOperation(const StateSet& value)
	{
		assert(&value != nullptr);

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

	reachFunc(aut.GetMtbdd(StateTuple()));

	while (!workset.empty())
	{
		StateType state = *(workset.begin());
		workset.erase(workset.begin());


	}




	return result;
}
