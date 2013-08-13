/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the graph class.
 *
 *****************************************************************************/

#ifndef _VATA_GRAPH_HH_
#define _VATA_GRAPH_HH_

#include <set>

namespace VATA
{
	namespace Util
	{
		class Graph;
	}
}

class VATA::Util::Graph
{
public:   // data types

	typedef uintptr_t NodeType;

private:   // data types

	typedef std::set<NodeType> EdgeContainer;

	struct InternalNode
	{
		EdgeContainer ingressEdges;
		EdgeContainer egressEdges;

		InternalNode() :
			ingressEdges(),
			egressEdges()
		{ }
	};

	typedef std::set<InternalNode*> NodeSet;

private:   // data members

	NodeSet nodes_;

private:   // methods

	Graph(const Graph&);
	Graph& operator=(const Graph&);

	inline static NodeType internalToNode(const InternalNode* internal)
	{
		return reinterpret_cast<NodeType>(internal);
	}

	inline static InternalNode* nodeToInternal(const NodeType& node)
	{
		return reinterpret_cast<InternalNode*>(node);
	}

public:   // methods

	Graph() :
		nodes_()
	{ }

	inline NodeType AddNode()
	{
		InternalNode* node = new InternalNode;

		nodes_.insert(node);

		return internalToNode(node);
	}

	inline void AddEdge(const NodeType& src, const NodeType& dst)
	{
		nodeToInternal(src)->egressEdges.insert(dst);
		nodeToInternal(dst)->ingressEdges.insert(src);
	}

	// NOTE: the node cannot be pointed by anything, otherwise the graph becomes
	// incosistent
	inline void DeleteNode(const NodeType& node)
	{
		InternalNode* ptrNode = nodeToInternal(node);
		assert(ptrNode != nullptr);

		if (nodes_.erase(ptrNode) != 1)
		{	// in case there was some problem erasing the node
			assert(false);    // fail gracefully
		}
	}

	inline static EdgeContainer& GetIngress(const NodeType& node)
	{
		return nodeToInternal(node)->ingressEdges;
	}

	inline static EdgeContainer& GetEgress(const NodeType& node)
	{
		return nodeToInternal(node)->egressEdges;
	}

	~Graph()
	{
		for (InternalNode* node : nodes_)
		{
			assert(node != nullptr);
			delete node;
		}
	}
};

#endif
