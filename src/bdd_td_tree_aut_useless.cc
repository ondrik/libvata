/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of pruning useless states of BDD top-down tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_td_tree_aut_op.hh>
#include <vata/mtbdd/apply1func.hh>
#include <vata/mtbdd/void_apply1func.hh>
#include <vata/util/graph.hh>

// Standard library headers
#include <stack>

using VATA::AutBase;
using VATA::BDDTopDownTreeAut;
using VATA::Util::Convert;
using VATA::Util::Graph;

template <>
BDDTopDownTreeAut VATA::RemoveUselessStates<BDDTopDownTreeAut>(
	const BDDTopDownTreeAut& aut, AutBase::StateToStateMap* pTranslMap)
{
	// Assertions
	assert(aut.isValid());

	typedef AutBase::StateType StateType;
	typedef BDDTopDownTreeAut::StateTuple StateTuple;
	typedef BDDTopDownTreeAut::StateTupleSet StateTupleSet;
	typedef AutBase::StateToStateMap StateToStateMap;

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
	typedef std::stack<StateType, std::list<StateType>> StateStack;

	typedef std::pair<StateType, StateType> StatePair;
	typedef std::stack<StatePair, std::list<StatePair>> StatePairStack;



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

		NodeToTupleDict& andNodes_;

		NodeToStateDict& orNodes_;

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
			size_t cnt = Graph::GetIngress(andNode).erase(node);
			assert(cnt == 1);

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

		BDDTopDownTreeAut& resultAut_;
		StateToStateMap& translMap_;
		StatePairStack& workStack_;
		StateHT& usefulStates_;

	public:   // methods

		RestrictApplyFunctor(BDDTopDownTreeAut& resultAut, StateToStateMap& translMap,
			StatePairStack& workStack, StateHT& usefulStates) :
			resultAut_(resultAut),
			translMap_(translMap),
			workStack_(workStack),
			usefulStates_(usefulStates)
		{ }

		StateTupleSet ApplyOperation(const StateTupleSet& value)
		{
			StateTupleSet result;

			for (const StateTuple& tuple : value)
			{	// for each tuple from the leaf
				StateTuple resultTuple;
				for (size_t i = 0; i < tuple.size(); ++i)
				{	// for each position in the tuple
					const StateType& state = tuple[i];

					if (usefulStates_.find(state) == usefulStates_.end())
					{	// in case the state is not useful
						break;
					}

					StateType newState;
					StateToStateMap::const_iterator itTransl;
					if ((itTransl = translMap_.find(state)) != translMap_.end())
					{	// if the pair is already known
						newState = itTransl->second;
					}
					else
					{	// if the pair is new
						newState = resultAut_.AddState();
						translMap_.insert(std::make_pair(state, newState));
						workStack_.push(std::make_pair(newState, state));
					}

					resultTuple.push_back(newState);
				}

				if (resultTuple.size() == tuple.size())
				{	// in case all states are useful
					result.insert(resultTuple);
				}
			}

			return result;
		}
	};

	BDDTopDownTreeAut result(aut.GetTransTable());
	StatePairStack workStack;

	bool deleteTranslMap = false;
	if (pTranslMap == nullptr)
	{	// in case the state translation map was not provided
		pTranslMap = new StateToStateMap();
		deleteTranslMap = true;
	}

	assert(pTranslMap->empty());

	RestrictApplyFunctor restrFunc(result, *pTranslMap, workStack, usefulStates);

	for (auto fst : aut.GetFinalStates())
	{	// start from all final states of the original automaton
		if (usefulStates.find(fst) != usefulStates.end())
		{	// in case the state is useful
			StateType newState = result.AddState();
			result.SetStateFinal(newState);
			workStack.push(std::make_pair(newState, fst));
			pTranslMap->insert(std::make_pair(fst, newState));
		}
	}

	while (!workStack.empty())
	{	// while there is something in the workset
		StatePair stPr = workStack.top();
		workStack.pop();

		BDDTopDownTreeAut::TransMTBDD mtbdd = restrFunc(aut.getMtbdd(stPr.second));

		result.setMtbdd(stPr.first, mtbdd);
	}

	if (deleteTranslMap)
	{	// in case we need to delete the map
		delete pTranslMap;
	}

	assert(result.isValid());

	return result;
}