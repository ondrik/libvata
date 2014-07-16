/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of pruning useless states of BDD bottom-up tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

// Standard library headers
#include <stack>

#include "bdd_bu_tree_aut_core.hh"
#include "mtbdd/void_apply1func.hh"
#include "util/graph.hh"

using VATA::AutBase;
using VATA::BDDBUTreeAutCore;
using VATA::Util::Convert;
using VATA::Util::Graph;

typedef VATA::AutBase::StateType StateType;
typedef VATA::BDDBUTreeAutCore::StateSet StateSet;
typedef VATA::BDDBUTreeAutCore::StateHT StateHT;
typedef VATA::BDDBUTreeAutCore::StateTuple StateTuple;
typedef VATA::BDDBUTreeAutCore::TransMTBDD TransMTBDD;

typedef std::unordered_map<StateTuple, TransMTBDD, boost::hash<StateTuple>>
	TupleHT;

typedef Graph::NodeType NodeType;

typedef VATA::Util::TwoWayDict<NodeType, StateType,
	std::unordered_map<NodeType, StateType>,
	std::unordered_map<StateType, NodeType>> NodeToStateDict;

typedef std::stack<NodeType, std::list<NodeType>> NodeWorkSet;

namespace
{	// anonymous namespace
GCC_DIAG_OFF(effc++)
class ReachableCollectorFctor : public VATA::MTBDDPkg::VoidApply1Functor<
	ReachableCollectorFctor,
	StateSet>
{
GCC_DIAG_ON(effc++)

private:  // data members

	StateHT& reachable_;
	StateHT& workset_;
	const StateTuple& tuple_;
	NodeToStateDict& nodes_;
	Graph& graph_;

public:   // methods

	ReachableCollectorFctor(
		StateHT&              reachable,
		StateHT&              workset,
		const StateTuple&     tuple,
		NodeToStateDict&      nodes,
		Graph&                graph) :
		reachable_(reachable),
		workset_(workset),
		tuple_(tuple),
		nodes_(nodes),
		graph_(graph)
	{ }

	void ApplyOperation(const StateSet& value)
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

			NodeType node;
			NodeToStateDict::ConstIteratorBwd itNode;
			if ((itNode = nodes_.FindBwd(state)) != nodes_.EndBwd())
			{
				node = itNode->second;
			}
			else
			{
				node = graph_.AddNode();
				nodes_.insert(std::make_pair(node, state));
			}

			for (const StateType& tupState : tuple_)
			{
				NodeToStateDict::ConstIteratorBwd itOtherNode;
				if ((itOtherNode = nodes_.FindBwd(tupState)) == nodes_.end())
				{
					assert(false);      // fail gracefully
				}

				graph_.AddEdge(node, itOtherNode->second);
			}
		}
	}
};

GCC_DIAG_OFF(effc++)
class UsefulCheckerFctor : public VATA::MTBDDPkg::Apply1Functor<
	UsefulCheckerFctor,
	StateSet, StateSet>
{
GCC_DIAG_ON(effc++)

private:  // data members

	const StateHT& useful_;

public:   // methods

	UsefulCheckerFctor(const StateHT& useful) :
		useful_(useful)
	{ }

	StateSet ApplyOperation(const StateSet& value)
	{
		StateSet result;

		for (const StateType& state : value)
		{
			if (useful_.find(state) != useful_.end())
			{
				result.insert(state);
			}
		}

		return result;
	}

};

} // namespace



BDDBUTreeAutCore BDDBUTreeAutCore::RemoveUselessStates() const
{
	BDDBUTreeAutCore result;

	StateHT reachable;
	StateHT workset;
	Graph graph;
	NodeToStateDict nodes;

	TupleHT tuples;

	for (auto tupleBddPair : this->GetTransTable())
	{
		tuples.insert(tupleBddPair);
	}

	StateTuple tuple;
	tuples.erase(tuple);

	ReachableCollectorFctor reachFunc(reachable, workset, tuple, nodes, graph);

	const TransMTBDD& nullaryBdd = this->GetMtbdd(StateTuple());
	reachFunc(nullaryBdd);

	while (!workset.empty())
	{
		StateType state = *(workset.begin());
		workset.erase(workset.begin());

		TupleHT::const_iterator itTup = tuples.begin();
		while (itTup != tuples.end())
		{
			tuple = itTup->first;

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

	NodeWorkSet nodeWorkset;
	StateHT useful;

	for (const StateType& fst : this->GetFinalStates())
	{
		NodeToStateDict::ConstIteratorBwd itNodes;
		if ((itNodes = nodes.FindBwd(fst)) != nodes.EndBwd())
		{
			result.SetStateFinal(fst);
			useful.insert(fst);

			nodeWorkset.push(itNodes->second);
		}
	}

	while (!nodeWorkset.empty())
	{
		NodeType node = nodeWorkset.top();
		nodeWorkset.pop();

		for (const NodeType& inNode : Graph::GetIngress(node))
		{	// for each input edge of the node, remove the edge
			if (Graph::GetEgress(inNode).erase(node) != 1)
			{	// in case of an internal error
				assert(false);     // fail gracefully
			}
		}

		for (const NodeType& outNode : Graph::GetEgress(node))
		{	// for each input edge of the node, remove the edge
			NodeToStateDict::const_iterator itNode;
			if ((itNode = nodes.FindFwd(outNode)) == nodes.EndFwd())
			{
				assert(false);      // fail gracefully
			}

			if (useful.insert(itNode->second).second)
			{
				nodeWorkset.push(itNode->first);
			}
		}
	}

	UsefulCheckerFctor usefulFunc(useful);

	for (auto tupleBddPair : this->GetTransTable())
	{
		const StateTuple& tuple = tupleBddPair.first;
		const TransMTBDD& bdd = tupleBddPair.second;
		size_t i;
		for (i = 0; i < tuple.size(); ++i)
		{
			if (useful.find(tuple[i]) == useful.end())
			{
				break;
			}
		}

		if (i != tuple.size())
		{
			continue;
		}

		result.SetMtbdd(tuple, usefulFunc(bdd));
	}

	return result;
}
