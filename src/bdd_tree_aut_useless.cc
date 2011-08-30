/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of pruning useless states of BDD tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_tree_aut_op.hh>
#include <vata/mtbdd/void_apply1func.hh>
#include <vata/util/graph.hh>

// Standard library headers
#include <stack>

using VATA::AutBase;
using VATA::BDDTreeAut;
using VATA::Util::Convert;
using VATA::Util::Graph;

template <>
BDDTreeAut VATA::RemoveUselessStates<BDDTreeAut>(const BDDTreeAut& aut,
	AutBase::StateToStateMap* pTranslMap)
{
	// Assertions
	assert(aut.isValid());

	typedef AutBase::StateType StateType;
	typedef BDDTreeAut::StateTuple StateTuple;
	typedef BDDTreeAut::StateTupleSet StateTupleSet;

	typedef Graph::NodeType NodeType;

	typedef std::pair<NodeType, StateType> NodeStatePair;
	typedef std::stack<NodeStatePair, std::list<NodeStatePair>> WorkSetType;

	typedef VATA::Util::TwoWayDict<NodeType, StateType,
		std::unordered_map<NodeType, StateType>,
		std::unordered_map<StateType, NodeType>> NodeToStateDict;
	typedef VATA::Util::TwoWayDict<NodeType, StateTuple,
		std::unordered_map<NodeType, StateTuple>,
		std::unordered_map<StateTuple, NodeType, boost::hash<StateTuple>>>
		NodeToTupleDict;
	typedef std::unordered_set<Graph::NodeType> NodeSet;


	GCC_DIAG_OFF(effc++)   // suppress missing virtual destructor warning
	class AndOrGraphConstrFunctor :
		public VATA::MTBDDPkg::VoidApply1Functor<AndOrGraphConstrFunctor,
		StateTupleSet>
	{
	GCC_DIAG_OFF(effc++)
	private:  // data members

		NodeType& procNode_;

		Graph& graph_;

		WorkSetType& workset_;

		NodeToTupleDict andNodes_;

		NodeToStateDict orNodes_;

		NodeSet& termNodes_;

	public:   // methods

		AndOrGraphConstrFunctor(NodeType& procNode, Graph& graph,
			WorkSetType& workset, NodeToTupleDict& andNodes, NodeToStateDict& orNodes,
			NodeSet& termNodes) :
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

							graph_.AddEdge(tupleNode, stateNode);
						}
					}

					graph_.AddEdge(procNode_, tupleNode);
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

	for (auto fst : aut.GetFinalStates())
	{
		NodeStatePair translPair(graph.AddNode(), fst);
		orNodes.insert(translPair);
		workset.push(translPair);
	}

	while (!workset.empty())
	{	// until we process all reachable states
		procPair = workset.top();
		workset.pop();

		func(aut.getMtbdd(procPair.second));
	}

	assert(false);

	return aut;
}
