/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of pruning useless states of BDD top-down tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

#include "bdd_td_tree_aut_core.hh"

#include "mtbdd/apply1func.hh"
#include "mtbdd/void_apply1func.hh"
#include "util/graph.hh"

// Standard library headers
#include <stack>

using VATA::AutBase;
using VATA::BDDTDTreeAutCore;
using VATA::Util::Convert;
using VATA::Util::Graph;

BDDTDTreeAutCore BDDTDTreeAutCore::RemoveUselessStates() const
{
	typedef AutBase::StateType StateType;
	typedef BDDTDTreeAutCore::StateTuple StateTuple;
	typedef BDDTDTreeAutCore::StateTupleSet StateTupleSet;

	typedef Graph::NodeType NodeType;

	typedef std::pair<NodeType, StateType> NodeStatePair;
	typedef std::stack<NodeStatePair, std::list<NodeStatePair>> WorkSetType;
	typedef std::stack<NodeType, std::list<NodeType>> NodeStack;

	typedef VATA::Util::TwoWayDict<NodeType, StateType,
		std::unordered_map<NodeType, StateType>,
		std::unordered_map<StateType, NodeType>> NodeToStateDict;
	typedef VATA::Util::TwoWayDict<NodeType, StateTuple,
		std::unordered_map<NodeType, StateTuple>,
		std::unordered_map<StateTuple, NodeType, boost::hash<StateTuple>>>
		NodeToTupleDict;
	typedef std::unordered_set<Graph::NodeType> NodeSet;

	typedef std::unordered_set<StateType> StateHT;

	typedef BDDTDTreeAutCore::TransMTBDD TransMTBDD;


	GCC_DIAG_OFF(effc++)   // suppress missing virtual destructor warning
	class AndOrGraphConstrFunctor : public VATA::MTBDDPkg::VoidApply1Functor<
		AndOrGraphConstrFunctor,
		StateTupleSet>
	{
	GCC_DIAG_OFF(effc++)
	private:  // data members

		NodeType& procNode_;

		Graph& graph_;

		WorkSetType& workset_;

		NodeToTupleDict& andNodes_;

		NodeToStateDict& orNodes_;

		NodeSet& termNodes_;

	public:   // methods

		AndOrGraphConstrFunctor(
			NodeType&          procNode,
			Graph&             graph,
			WorkSetType&       workset,
			NodeToTupleDict&   andNodes,
			NodeToStateDict&   orNodes,
			NodeSet&           termNodes) :
			procNode_(procNode),
			graph_(graph),
			workset_(workset),
			andNodes_(andNodes),
			orNodes_(orNodes),
			termNodes_(termNodes)
		{ }

		void ApplyOperation(const StateTupleSet& value)
		{
			for (const StateTuple& tuple : value)
			{
				if (tuple.empty())
				{	// in case this is a nullary transition
					termNodes_.insert(procNode_);
				}
				else
				{
					NodeType tupleNode;
					NodeToTupleDict::ConstIteratorBwd itAndNode;
					if ((itAndNode = andNodes_.FindBwd(tuple)) != andNodes_.EndBwd())
					{	// in case the tuple has already been processed
						tupleNode = itAndNode->second;
					}
					else
					{	// in case the tuple hasn't been processed yet
						tupleNode = graph_.AddNode();
						andNodes_.insert(std::make_pair(tupleNode, tuple));

						for (const StateType& state : tuple)
						{
							NodeType stateNode;
							NodeToStateDict::ConstIteratorBwd itOrNodes;
							if ((itOrNodes = orNodes_.FindBwd(state)) != orNodes_.EndBwd())
							{	// in case there is a so far unexplored state
								stateNode = itOrNodes->second;
							}
							else
							{
								stateNode = graph_.AddNode();
								NodeStatePair translPair(stateNode, state);
								orNodes_.insert(translPair);
								workset_.push(translPair);
							}

							graph_.AddEdge(stateNode, tupleNode);
						}
					}

					graph_.AddEdge(tupleNode, procNode_);
				}
			}
		}
	};


	Graph graph;
	WorkSetType workset;
	NodeToStateDict orNodes;
	NodeToTupleDict andNodes;
	NodeSet termNodes;
	NodeStatePair procPair;

	AndOrGraphConstrFunctor func(procPair.first, graph, workset, andNodes,
		orNodes, termNodes);

	for (const StateType& fst : this->GetFinalStates())
	{
		NodeStatePair translPair(graph.AddNode(), fst);
		orNodes.insert(translPair);
		workset.push(translPair);
	}

	while (!workset.empty())
	{	// until we process all reachable states
		procPair = workset.top();
		workset.pop();

		func(this->GetMtbdd(procPair.second));
	}

	// now perform the analysis of useful states
	NodeStack nodeStack;
	StateHT usefulStates;

	for (const NodeType& node : termNodes)
	{
		nodeStack.push(node);

		NodeToStateDict::ConstIteratorFwd itOrNode;
		if ((itOrNode = orNodes.FindFwd(node)) == orNodes.end())
		{
			assert(false);   // fail gracefully
		}

		usefulStates.insert(itOrNode->second);
	}

	while (!nodeStack.empty())
	{
		NodeType node = nodeStack.top();
		nodeStack.pop();

		for (const NodeType& andNode : Graph::GetIngress(node))
		{	// for each input edge of the node, remove the edge
			if (Graph::GetEgress(andNode).erase(node) != 1)
			{	// in case of an internal error
				assert(false);     // fail gracefully
			}
		}

		for (const NodeType& andNode : Graph::GetEgress(node))
		{	// for each output edge of the node, satisfy edges
			if (Graph::GetIngress(andNode).erase(node) != 1)
			{
				assert(false);     // fail gracefully
			}

			if (Graph::GetIngress(andNode).empty())
			{	// in case all input edges of the AND node are satisfied
				for (const NodeType& orNode : Graph::GetEgress(andNode))
				{	// satisfy all output OR nodes
					NodeToStateDict::ConstIteratorFwd itDict;
					if ((itDict = orNodes.FindFwd(orNode)) == orNodes.EndFwd())
					{
						assert(false);      // fail gracefully
					}

					const StateType& state = itDict->second;

					if (usefulStates.find(state) == usefulStates.end())
					{	// in case the state hasn't been processed yet
						usefulStates.insert(state);
						nodeStack.push(orNode);
					}
				}
			}
		}
	}

	// perform restriction of the states of the automaton to the set of useful
	// states

	class RestrictApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<RestrictApplyFunctor,
		StateTupleSet, StateTupleSet>
	{
	private:  // data members

		StateHT& usefulStates_;

	public:   // methods

		RestrictApplyFunctor(StateHT& usefulStates) :
			usefulStates_(usefulStates)
		{ }

		StateTupleSet ApplyOperation(const StateTupleSet& value)
		{
			StateTupleSet result;

			for (const StateTuple& tuple : value)
			{	// for each tuple from the leaf
				StateTuple resultTuple;
				for (StateTuple::const_iterator itTup = tuple.begin();
					itTup != tuple.end(); ++itTup)
				{	// for each position in the tuple
					if (usefulStates_.find(*itTup) == usefulStates_.end())
					{	// in case the state is not useful
						break;
					}

					resultTuple.push_back(*itTup);
				}

				if (resultTuple.size() == tuple.size())
				{	// in case all states are useful
					result.insert(resultTuple);
				}
			}

			return result;
		}
	};

	BDDTDTreeAutCore result;

	RestrictApplyFunctor restrFunc(usefulStates);

	for (auto fst : this->GetFinalStates())
	{	// start from all final states of the original automaton
		if (usefulStates.find(fst) != usefulStates.end())
		{	// in case the state is useful
			result.SetStateFinal(fst);
		}
	}

	for (auto stateBddPair : this->GetStates())
	{	// for all states
		const StateType& state = stateBddPair.first;
		const TransMTBDD& bdd = stateBddPair.second;

		if (usefulStates.find(state) != usefulStates.end())
		{
			result.SetMtbdd(state, restrFunc(bdd));
		}
	}

	return result.RemoveUnreachableStates();
}
