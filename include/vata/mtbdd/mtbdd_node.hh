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

// Standard library headers
#include	<cassert>
#include	<stdint.h>

// Boost headers
#include <boost/functional/hash.hpp>


namespace
{
	template <
		typename Data
	>
	struct InternalNode;

	template <
		typename Data
	>
	struct LeafNode;
}

namespace VATA
{
	namespace MTBDDPkg
	{
		template <
			typename Data
		>
		struct MTBDDNodePtr;
	}
}



template <
	typename Data
>
struct VATA::MTBDDPkg::MTBDDNodePtr
{
public:   // public data types

	typedef Data DataType;
	typedef uintptr_t RefCntType;
	typedef uintptr_t VarType;

private:  // private data types

	typedef InternalNode<DataType> InternalType;
	typedef LeafNode<DataType> LeafType;

private:  // private data members

	uintptr_t addr_;

private:  // private methods

	static inline MTBDDNodePtr makeLeaf(LeafType* node)
	{
		// Assertions
		assert(node != static_cast<LeafType*>(0));

		return (reinterpret_cast<uintptr_t>(node) | 1);
	}

	static inline MTBDDNodePtr makeInternal(InternalType* node)
	{
		// Assertions
		assert(node != static_cast<InternalType*>(0));

		return static_cast<MTBDDNodePtr>(node);
	}

	static inline LeafType* nodeToLeaf(MTBDDNodePtr& node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsLeaf(node));

		return reinterpret_cast<LeafType*>(node.addr_ ^ 1);
	}

	static inline const LeafType* nodeToLeaf(const MTBDDNodePtr& node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsLeaf(node));

		return nodeToLeaf(const_cast<MTBDDNodePtr&>(node));
	}

	static inline InternalType* nodeToInternal(MTBDDNodePtr& node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsInternal(node));

		return reinterpret_cast<InternalType*>(node.addr_);
	}

	static inline const InternalType* nodeToInternal(const MTBDDNodePtr& node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsInternal(node));

		return nodeToInternal(const_cast<MTBDDNodePtr&>(node));
	}

	static inline const RefCntType& getInternalRefCnt(const MTBDDNodePtr& node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsInternal(node));

		return nodeToInternal(node)->GetRefCnt();
	}

	static inline void incrementLeafRefCnt(MTBDDNodePtr node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsLeaf(node));

		nodeToLeaf(node)->IncrementRefCnt();
	}

	static inline void incrementInternalRefCnt(MTBDDNodePtr node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsInternal(node));

		nodeToInternal(node)->IncrementRefCnt();
	}

public:

	MTBDDNodePtr(const uintptr_t addr)
		: addr_(addr)
	{ }

	explicit MTBDDNodePtr(const LeafType* leaf)
		: addr_(static_cast<uintptr_t>(leaf))
	{ }

	explicit MTBDDNodePtr(const InternalType* internal)
		: addr_(reinterpret_cast<uintptr_t>(internal))
	{ }

	inline bool operator==(const MTBDDNodePtr& rhs) const
	{
		return addr_ == rhs.addr_;
	}

	inline bool operator!=(const MTBDDNodePtr& rhs) const
	{
		return !operator==(rhs);
	}

	template <typename DataType>
	friend MTBDDNodePtr<DataType> CreateLeaf(const DataType& data);

	template <typename NodePtrType>
	friend NodePtrType CreateInternal(NodePtrType low, NodePtrType high,
		const typename NodePtrType::VarType& var);

	template <typename NodePtrType>
	friend void DeleteLeafNode(NodePtrType node);

	template <typename NodePtrType>
	friend void DeleteInternalNode(NodePtrType node);

	template <typename NodePtrType>
	friend bool IsNull(NodePtrType node);

	template <typename NodePtrType>
	friend bool IsLeaf(const NodePtrType& node);

	template <typename NodePtrType>
	friend bool IsInternal(const NodePtrType& node);

	template <typename NodePtrType>
	friend const typename NodePtrType::VarType& GetVarFromInternal(
		NodePtrType& node);

	template <typename NodePtrType>
	friend NodePtrType GetLowFromInternal(NodePtrType& node);

	template <typename NodePtrType>
	friend NodePtrType GetLowFromInternal(const NodePtrType& node);

	template <typename NodePtrType>
	friend NodePtrType GetHighFromInternal(NodePtrType& node);

	template <typename NodePtrType>
	friend NodePtrType GetHighFromInternal(const NodePtrType& node);

	template <typename NodePtrType>
	friend const typename NodePtrType::DataType& GetDataFromLeaf(
		const NodePtrType& node);

	template <typename NodePtrType>
	friend const typename NodePtrType::VarType& GetLeafRefCnt(
		const NodePtrType node);

	template <typename NodePtrType>
	friend void IncrementRefCnt(NodePtrType node);

	template <typename NodePtrType>
	friend const typename NodePtrType::RefCntType& DecrementLeafRefCnt(
		NodePtrType node);

	template <typename NodePtrType>
	friend const typename NodePtrType::RefCntType& DecrementInternalRefCnt(
		NodePtrType node);

	template <typename NodePtrType>
	friend size_t hash_value(const NodePtrType& node);
};


namespace
{
	template <
		typename Data
	>
	struct InternalNode
	{
	public:   // public data types

		typedef Data DataType;
		typedef VATA::MTBDDPkg::MTBDDNodePtr<DataType> NodePtrType;
		typedef typename NodePtrType::RefCntType RefCntType;
		typedef typename NodePtrType::VarType VarType;

	private:  // private data members

		NodePtrType low_;
		NodePtrType high_;
		VarType var_;
		RefCntType refcnt_;

	public:   // public methods

		// I know what I'm doing!!
		GCC_DIAG_OFF(uninitialized)
		InternalNode(NodePtrType low, NodePtrType high, const VarType& var,
			const RefCntType& refcnt)
			: low_(low),
				high_(high),
				var_(var),
				refcnt_(refcnt)
		{
			// Assertions
			assert(!IsNull(low));
			assert(!IsNull(high));
		}
		GCC_DIAG_ON(uninitialized)

		inline const VarType& GetVar() const
		{
			return var_;
		}

		inline const NodePtrType& GetLow() const
		{
			return low_;
		}

		inline const NodePtrType& GetHigh() const
		{
			return high_;
		}

		inline const RefCntType& GetRefCnt() const
		{
			return refcnt_;
		}

		inline void IncrementRefCnt()
		{
			++refcnt_;
		}

		inline const RefCntType& DecrementRefCnt()
		{
			// Assertions
			assert(refcnt_ > 0);

			return --refcnt_;
		}
	};


	template <
		typename Data
	>
	struct LeafNode
	{
	public:   // public data members

		typedef Data DataType;
		typedef VATA::MTBDDPkg::MTBDDNodePtr<DataType> NodePtr;
		typedef typename NodePtr::RefCntType RefCntType;

	private:  // private data members

		DataType data_;
		RefCntType refcnt_;

	public:   // public methods

		LeafNode(const DataType& data, const RefCntType& refcnt)
			: data_(data),
				refcnt_(refcnt)
		{ }

		inline const DataType& GetData() const
		{
			return data_;
		}

		inline const RefCntType& GetRefCnt() const
		{
			return refcnt_;
		}

		inline void IncrementRefCnt()
		{
			++refcnt_;
		}

		inline const RefCntType& DecrementRefCnt()
		{
			// Assertions
			assert(refcnt_ > 0);

			return --refcnt_;
		}
	};
}


namespace VATA
{
	namespace MTBDDPkg
	{
		template <typename NodePtrType>
		inline bool IsLeaf(const NodePtrType& node)
		{
			assert(node.addr_ != 0);
			return (node.addr_ % 2);
		}

		template <typename NodePtrType>
		inline bool IsInternal(const NodePtrType& node)
		{
			assert(node.addr_ != 0);
			return !(IsLeaf(node));
		}

		template <typename NodePtrType>
		inline const typename NodePtrType::DataType& GetDataFromLeaf(
			const NodePtrType& node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsLeaf(node));

			return NodePtrType::nodeToLeaf(node)->GetData();
		}

		template <typename NodePtrType>
		inline const typename NodePtrType::VarType& GetVarFromInternal(
			NodePtrType& node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsInternal(node));

			return NodePtrType::nodeToInternal(node)->GetVar();
		}

		template <typename NodePtrType>
		inline NodePtrType GetLowFromInternal(const NodePtrType& node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsInternal(node));

			return NodePtrType::nodeToInternal(node)->GetLow();
		}

		template <typename NodePtrType>
		inline NodePtrType GetLowFromInternal(NodePtrType& node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsInternal(node));

			return NodePtrType::nodeToInternal(node)->GetLow();
		}

		template <typename NodePtrType>
		inline NodePtrType GetHighFromInternal(const NodePtrType& node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsInternal(node));

			return NodePtrType::nodeToInternal(node)->GetHigh();
		}

		template <typename NodePtrType>
		inline NodePtrType GetHighFromInternal(NodePtrType& node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsInternal(node));

			return NodePtrType::nodeToInternal(node)->GetHigh();
		}

		template <typename DataType>
		inline MTBDDNodePtr<DataType> CreateLeaf(const DataType& data)
		{
			typedef MTBDDNodePtr<DataType> NodePtrType;
			typedef typename NodePtrType::LeafType LeafType;

			// TODO: create allocator						
			LeafType* newNode = new LeafType(data, 0);

			return NodePtrType::makeLeaf(newNode);
		}

		template <typename NodePtrType>
		inline NodePtrType CreateInternal(NodePtrType low, NodePtrType high,
			const typename NodePtrType::VarType& var)
		{
			// Assertions
			assert(!IsNull(low));
			assert(!IsNull(high));

			typedef typename NodePtrType::InternalType InternalType;

			// TODO: create allocator
			InternalType* newNode = new InternalType(low, high, var, 0);

			return NodePtrType::makeInternal(newNode);
		}

		template <typename NodePtrType>
		inline void IncrementRefCnt(NodePtrType node)
		{
			// Assertions
			assert(!IsNull(node));

			if (IsLeaf(node))
			{
				NodePtrType::incrementLeafRefCnt(node);
			}
			else
			{
				assert(IsInternal(node));
				NodePtrType::incrementInternalRefCnt(node);
			}
		}


		template <typename NodePtrType>
		inline const typename NodePtrType::VarType& GetLeafRefCnt(
			const NodePtrType node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsLeaf(node));

			return NodePtrType::nodeToLeaf(node)->GetRefCnt();
		}


		template <typename NodePtrType>
		inline const typename NodePtrType::RefCntType& DecrementLeafRefCnt(
			NodePtrType node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsLeaf(node));
			assert(GetLeafRefCnt(node) > 0);

			return NodePtrType::nodeToLeaf(node)->DecrementRefCnt();
		}


		template <typename NodePtrType>
		inline const typename NodePtrType::RefCntType& DecrementInternalRefCnt(
			NodePtrType node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsInternal(node));
			assert(NodePtrType::getInternalRefCnt(node) > 0);

			return NodePtrType::nodeToInternal(node)->DecrementRefCnt();
		}


		template <typename NodePtrType>
		inline void DeleteLeafNode(NodePtrType node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsLeaf(node));
			assert(GetLeafRefCnt(node) == 0);

			delete NodePtrType::nodeToLeaf(node);
		}

		template <typename NodePtrType>
		inline void DeleteInternalNode(NodePtrType node)
		{
			// Assertions
			assert(!IsNull(node));
			assert(IsInternal(node));
			assert(NodePtrType::getInternalRefCnt(node) == 0);

			delete NodePtrType::nodeToInternal(node);
		}

		template <typename NodePtrType>
		inline bool IsNull(NodePtrType node)
		{
			return node.addr_ == 0;
		}

		template <typename NodePtrType>
		inline size_t hash_value(const NodePtrType& node)
		{
			// Assertions
			assert(!IsNull(node));

			boost::hash<uintptr_t> hasher;
			return hasher(node.addr_);
		}
	}
}


#endif
