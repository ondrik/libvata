/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Node of Ondrik's MTBDD
 *
 *****************************************************************************/

#ifndef _VATA_MTBDD_NODE_HH_
#define _VATA_MTBDD_NODE_HH_

// VATA headers
#include <vata/vata.hh>


namespace VATA
{
	namespace MTBDDPkg
	{
		template <
			typename Data
		>
		union MTBDDNode;
	}
}


/**
 * @brief  MTBDD node
 *
 * Union for MTBDD node. It holds either data (for leaf (or sink)
 * nodes), or pointers to low and high nodes together with variable
 * name (for internal nodes). In addition, it contains a counter of the
 * number of references to the node. The type of the node (leaf or internal)
 * is determined using the pointer to the node: if the address is even, then
 * the node is internal, if it is odd, then it is leaf.
 *
 * @tparam  Data  The type of data to be stored into leaf nodes
 */
template
<
	typename Data
>
union VATA::MTBDDPkg::MTBDDNode
{
public:   // Public data types

	typedef Data DataType;
	typedef uintptr_t VarType;
	typedef uintptr_t RefCntType;

private:  // Private disjunct data

	struct
	{
		MTBDDNode* low;
		MTBDDNode* high;
		VarType var;
		RefCntType refcnt;
	} internal;

	struct
	{
		DataType data;
		RefCntType refcnt;
	} leaf;

public:  // Public methods

	MTBDDNode(MTBDDNode* parLow, MTBDDNode* parHigh, const VarType& parVar,
		const RefCntType& parRefcnt)
	{
		internal.low = parLow;
		internal.high = parHigh;
		internal.var = parVar;
		internal.refcnt = parRefcnt;
	}

	MTBDDNode(const DataType& parData, const RefCntType& parRefcnt)
	{
		leaf.data = parData;
		leaf.refcnt = parRefcnt;
	}

private:  // Private methods

	static inline MTBDDNode* makeLeaf(MTBDDNode* node)
	{
		// Assertions
		assert(node != static_cast<MTBDDNode*>(0));

		return reinterpret_cast<MTBDDNode*>(
			reinterpret_cast<uintptr_t>(node) | 1);
	}

	static inline MTBDDNode* makeInternal(MTBDDNode* node)
	{
		// Assertions
		assert(node != static_cast<MTBDDNode*>(0));

		return node;
	}

	static inline MTBDDNode* leafToNode(MTBDDNode* node)
	{
		// Assertions
		assert(node != static_cast<MTBDDNode*>(0));
		assert(IsLeaf(node));

		return reinterpret_cast<MTBDDNode*>(
			reinterpret_cast<uintptr_t>(node) ^ 1);
	}

	static inline const MTBDDNode* leafToNode(const MTBDDNode* node)
	{
		// Assertions
		assert(node != static_cast<MTBDDNode*>(0));
		assert(IsLeaf(node));

		return reinterpret_cast<MTBDDNode*>(
			reinterpret_cast<uintptr_t>(node) ^ 1);
	}

	static inline MTBDDNode* internalToNode(MTBDDNode* node)
	{
		// Assertions
		assert(node != static_cast<MTBDDNode*>(0));
		assert(IsInternal(node));

		return node;
	}

	static inline const MTBDDNode* internalToNode(const MTBDDNode* node)
	{
		// Assertions
		assert(node != static_cast<MTBDDNode*>(0));
		assert(IsInternal(node));

		return node;
	}

	static inline const RefCntType& getLeafRefCnt(const MTBDDNode* node)
	{
		// Assertions
		assert(node != static_cast<MTBDDNode*>(0));
		assert(IsLeaf(node));

		return leafToNode(node)->leaf.refcnt;
	}

	static inline const RefCntType& getInternalRefCnt(const MTBDDNode* node)
	{
		// Assertions
		assert(node != static_cast<MTBDDNode*>(0));
		assert(IsInternal(node));

		return internalToNode(node)->internal.refcnt;
	}

	static inline void incrementLeafRefCnt(MTBDDNode* node)
	{
		// Assertions
		assert(node != static_cast<MTBDDNode*>(0));
		assert(IsLeaf(node));

		++(leafToNode(node)->leaf.refcnt);
	}

	static inline void incrementInternalRefCnt(MTBDDNode* node)
	{
		// Assertions
		assert(node != static_cast<MTBDDNode*>(0));
		assert(IsInternal(node));

		++(internalToNode(node)->internal.refcnt);
	}

	template <typename NodeType>
	friend void DeleteMTBDDdag(NodeType* root);

	template <typename NodeType>
	friend const typename NodeType::VarType& GetVarFromInternal(
		const NodeType* node);

	template <typename NodeType>
	friend NodeType* GetLowFromInternal(NodeType* node);

	template <typename NodeType>
	friend const NodeType* GetLowFromInternal(const NodeType* node);

	template <typename NodeType>
	friend NodeType* GetHighFromInternal(NodeType* node);

	template <typename NodeType>
	friend const NodeType* GetHighFromInternal(const NodeType* node);

	template <typename NodeType>
	friend const typename NodeType::DataType& GetDataFromLeaf(
		const NodeType* node);

	template <typename DataType>
	friend MTBDDNode<DataType>* CreateLeaf(const DataType& data);

	template <typename NodeType>
	friend NodeType* CreateInternal(
		NodeType* low, NodeType* high, const typename NodeType::VarType& var);

	template <typename NodeType>
	friend void DeleteLeafNode(NodeType* node);

	template <typename NodeType>
	friend void DeleteInternalNode(NodeType* node);

	template <typename NodeType>
	friend inline void RecursivelyIncrementRefCnt(NodeType* node);

	template <typename NodeType>
	friend const typename NodeType::RefCntType& DecrementLeafRefCnt(
		NodeType* node);

	template <typename NodeType>
	friend const typename NodeType::RefCntType& DecrementInternalRefCnt(
		NodeType* node);
};

namespace VATA
{
	namespace MTBDDPkg
	{
		template <typename NodeType>
		inline bool IsLeaf(const NodeType* node)
		{
			assert(node != static_cast<NodeType*>(0));
			return (reinterpret_cast<uintptr_t>(node) % 2);
		}

		template <typename NodeType>
		inline bool IsInternal(const NodeType* node)
		{
			assert(node != static_cast<NodeType*>(0));
			return !(IsLeaf(node));
		}

		template <typename NodeType>
		inline const typename NodeType::DataType& GetDataFromLeaf(
			const NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));
			assert(IsLeaf(node));

			return NodeType::leafToNode(node)->leaf.data;
		}

		template <typename NodeType>
		inline const typename NodeType::VarType& GetVarFromInternal(
			const NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));
			assert(IsInternal(node));

			return NodeType::internalToNode(node)->internal.var;
		}

		template <typename NodeType>
		inline const NodeType* GetLowFromInternal(const NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));
			assert(IsInternal(node));

			return NodeType::internalToNode(node)->internal.low;
		}

		template <typename NodeType>
		inline NodeType* GetLowFromInternal(NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));
			assert(IsInternal(node));

			return NodeType::internalToNode(node)->internal.low;
		}

		template <typename NodeType>
		inline const NodeType* GetHighFromInternal(const NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));
			assert(IsInternal(node));

			return NodeType::internalToNode(node)->internal.high;
		}

		template <typename NodeType>
		inline NodeType* GetHighFromInternal(NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));
			assert(IsInternal(node));

			return NodeType::internalToNode(node)->internal.high;
		}

		template <typename DataType>
		inline MTBDDNode<DataType>* CreateLeaf(const DataType& data)
		{
			typedef MTBDDNode<DataType> NodeType;

			// TODO: create allocator						
			NodeType* newNode = new NodeType(data, 0);

			return NodeType::makeLeaf(newNode);
		}

		template <typename NodeType>
		inline NodeType* CreateInternal(
			NodeType* low, NodeType* high, const typename NodeType::VarType& var)
		{
			// Assertions
			assert(low != static_cast<NodeType*>(0));
			assert(high != static_cast<NodeType*>(0));

			// TODO: create allocator
			NodeType* newNode = new NodeType(low, high, var, 0);

			return NodeType::makeInternal(newNode);
		}

		template <typename NodeType>
		inline void RecursivelyIncrementRefCnt(NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));

			if (IsLeaf(node))
			{
				NodeType::incrementLeafRefCnt(node);
			}
			else
			{
				assert(IsInternal(node));

				RecursivelyIncrementRefCnt(GetLowFromInternal(node));
				RecursivelyIncrementRefCnt(GetHighFromInternal(node));

				NodeType::incrementInternalRefCnt(node);
			}
		}


		template <typename NodeType>
		inline const typename NodeType::RefCntType& DecrementLeafRefCnt(
			NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));
			assert(IsLeaf(node));
			assert(NodeType::getLeafRefCnt(node) > 0);

			return --(NodeType::leafToNode(node)->leaf.refcnt);
		}

		template <typename NodeType>
		inline const typename NodeType::RefCntType& DecrementInternalRefCnt(
			NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));
			assert(IsInternal(node));
			assert(NodeType::getInternalRefCnt(node) > 0);

			return --(NodeType::internalToNode(node)->internal.refcnt);
		}

		template <typename NodeType>
		inline void DeleteLeafNode(NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));
			assert(IsLeaf(node));
			assert(NodeType::getLeafRefCnt(node) == 0);

			delete NodeType::leafToNode(node);
		}

		template <typename NodeType>
		inline void DeleteInternalNode(NodeType* node)
		{
			// Assertions
			assert(node != static_cast<NodeType*>(0));
			assert(IsInternal(node));
			assert(NodeType::getInternalRefCnt(node) == 0);

			delete NodeType::internalToNode(node);
		}
	}
}


#endif
