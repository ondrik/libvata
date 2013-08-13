/*****************************************************************************
 *  VATA Tree Automata Library
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
#include <vata/util/convert.hh>

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


/**
 * @brief   Pointer to an MTBDD node
 *
 * This is a pointer to an MTBDD node. It can point either to an internal node
 * or to a leaf node. Methods of this structure are optimised so that they can
 * be inlined thus yielding minimum overhead.
 *
 * @li  Internal nodes contain left and right child pointers and a variable. 
 *
 * @li  Leaf nodes contain value of the given data type.
 *
 * Both internal and leaf nodes contain a counter of references. The value of
 * this counter denotes <em>how many times given node is referenced</em>
 * either by other MTBDD nodes or from other places (e.g., as an MTBDD root).
 * Note that it <b>does not</b> count the <em>number of trees going through
 * the node</em>.
 *
 * @tparam  Data  The data type stored in leaves
 */
template <
	typename Data
>
struct VATA::MTBDDPkg::MTBDDNodePtr
{
public:   // public data types


	/**
	 * @brief  Leaf data type
	 *
	 * The data types used in leaves of the MTBDD.
	 */
	typedef Data DataType;


	/**
	 * @brief  Reference counter type
	 *
	 * The data type used for reference counter of MTBDD nodes.
	 */
	typedef uintptr_t RefCntType;

	/**
	 * @brief  Type of Boolean variables
	 *
	 * The data type of Boolean variables stored in the nodes of the MTBDD.
	 */
	typedef uintptr_t VarType;

private:  // private data types

	/**
	 * @brief  The type of internal nodes
	 *
	 * The type of MTBDD internal nodes.
	 */
	typedef InternalNode<DataType> InternalType;


	/**
	 * @brief  The type of leaf nodes
	 *
	 * The type of MTBDD leaf nodes.
	 */
	typedef LeafNode<DataType> LeafType;


private:  // private data members


	/**
	 * @brief  Address of the pointer
	 *
	 * The address that the pointer is pointing to.
	 */
	uintptr_t addr_;

private:  // private methods


	/**
	 * @brief  Creates MTBDDNodePtr from a leaf pointer
	 *
	 * This static method creates MTBDDNodePtr from a leaf node pointer.
	 *
	 * @param[in]  node  Input leaf node pointer
	 *
	 * @return  MTBDDNodePtr with @p node address
	 */
	static inline MTBDDNodePtr makeLeaf(LeafType* node)
	{
		// Assertions
		assert(node != nullptr);

		return (reinterpret_cast<uintptr_t>(node) | 1);
	}


	/**
	 * @brief  Creates MTBDDNodePtr from an internal node pointer
	 *
	 * This static method creates MTBDDNodePtr from an internal node pointer.
	 *
	 * @param[in]  node  Input internal node pointer
	 *
	 * @return  MTBDDNodePtr with @p node address
	 */
	static inline MTBDDNodePtr makeInternal(InternalType* node)
	{
		// Assertions
		assert(node != nullptr);

		return static_cast<MTBDDNodePtr>(node);
	}


	/**
	 * @brief  Translates MTBDDNodePtr to a leaf
	 *
	 * The static method that translates MTBDDNodePtr to a pointer to a leaf
	 * node.
	 *
	 * @param[in]  node  Input MTBDDNodePtr
	 *
	 * @return  Leaf node pointer
	 */
	static inline LeafType* nodeToLeaf(MTBDDNodePtr& node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsLeaf(node));

		return reinterpret_cast<LeafType*>(node.addr_ ^ 1);
	}

	/**
	 * @copydoc MTBDDNodePtr::nodeToLeaf(MTBDDNodePtr&)
	 */
	static inline const LeafType* nodeToLeaf(const MTBDDNodePtr& node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsLeaf(node));

		return nodeToLeaf(const_cast<MTBDDNodePtr&>(node));
	}

	/**
	 * @brief  Translates MTBDDNodePtr to an internal node
	 *
	 * The static method that translates MTBDDNodePtr to a pointer to an
	 * internal node.
	 *
	 * @param[in]  node  Input MTBDDNodePtr
	 *
	 * @return  Internal node pointer
	 */
	static inline InternalType* nodeToInternal(MTBDDNodePtr& node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsInternal(node));

		return reinterpret_cast<InternalType*>(node.addr_);
	}


	/**
	 * @copydoc MTBDDNodePtr::nodeToInternal(MTBDDNodePtr&)
	 */
	static inline const InternalType* nodeToInternal(const MTBDDNodePtr& node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsInternal(node));

		return nodeToInternal(const_cast<MTBDDNodePtr&>(node));
	}


	/**
	 * @brief  Retrieve internal node's reference counter value
	 *
	 * Static method that retrieves reference counter value of an MTBDDNodePtr
	 * representing an internal node.
	 *
	 * @param[in]  node  MTBDDNodePtr of an internal node
	 *
	 * @return  Reference counter value
	 */
	static inline const RefCntType& getInternalRefCnt(const MTBDDNodePtr& node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsInternal(node));

		return nodeToInternal(node)->GetRefCnt();
	}


	/**
	 * @brief  Increments leaf node's reference counter value
	 *
	 * Static method that increments value of the reference counter of an
	 * MTBDDNodePtr representing a leaf node.
	 *
	 * @param[in]  node  MTBDDNodePtr of a leaf node
	 */
	static inline void incrementLeafRefCnt(MTBDDNodePtr node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsLeaf(node));

		nodeToLeaf(node)->IncrementRefCnt();
	}


	/**
	 * @brief  Increments internal node's reference counter value
	 *
	 * Static method that increments value of the reference counter of an
	 * MTBDDNodePtr representing an internal node.
	 *
	 * @param[in]  node  MTBDDNodePtr of an internal node
	 */
	static inline void incrementInternalRefCnt(MTBDDNodePtr node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsInternal(node));

		nodeToInternal(node)->IncrementRefCnt();
	}

public:


	/**
	 * @brief  Constructs MTBDDNodePtr from an address
	 *
	 * Creates an MTBDDNodePtr from an address (or rather, from @p uintptr_t).
	 *
	 * @param[in]  addr  Address of an MTBDD node given as @p uintptr_t
	 */
	MTBDDNodePtr(const uintptr_t addr) :
		addr_(addr)
	{ }


	/**
	 * @brief  Converts a leaf node pointer to MTBDDNodePtr
	 *
	 * This conversion operator converts a leaf node pointer into MTBDDNodePtr.
	 *
	 * @param[in]  leaf  The leaf node pointer that should be converted
	 */
	explicit MTBDDNodePtr(const LeafType* leaf) :
		addr_(static_cast<uintptr_t>(leaf))
	{ }


	/**
	 * @brief  Converts in internal node pointer to MTBDDNodePtr
	 *
	 * This conversion operator converts an internal node pointer into
	 * MTBDDNodePtr.
	 *
	 * @param[in]  internal	  The internal node pointer that should be converted
	 */
	explicit MTBDDNodePtr(const InternalType* internal) :
		addr_(reinterpret_cast<uintptr_t>(internal))
	{ }


	/**
	 * @brief  Equality comparison operator
	 *
	 * Equality comparison operator, compares stored addreses.
	 *
	 * @param[in]  rhs  Right-hand side of the comparison
	 *
	 * @return  @p true if the object is equal to @p rhs, @p false otherwise
	 */
	inline bool operator==(const MTBDDNodePtr& rhs) const
	{
		return addr_ == rhs.addr_;
	}


	/**
	 * @brief  Unequality comparison operator
	 *
	 * Unequality comparison operator, compares stored addreses.
	 *
	 * @param[in]  rhs  Right-hand side of the comparison
	 *
	 * @return  @p true if the object is not equal to @p rhs, @p false otherwise
	 */
	inline bool operator!=(const MTBDDNodePtr& rhs) const
	{
		return !operator==(rhs);
	}

	// Friends

	/**
	 * @brief  Creates a leaf node
	 *
	 * This function creates a leaf node with given data value and returns the
	 * pointer to the node.
	 *
	 * @param[in]  data  The data value to be stored in the leaf
	 *
	 * @return  Pointer to the created leaf node
	 *
	 * @tparam  DataType  The data type of the leaf
	 */
	template <typename DataType>
	friend MTBDDNodePtr<DataType> CreateLeaf(const DataType& data);

	/**
	 * @brief  Creates an internal node
	 *
	 * This function creates an internal node with given child nodes and
	 * variable, and returns the pointer to the node.
	 *
	 * @param[in]  low   Pointer to the @e low child
	 * @param[in]  high  Pointer to the @e high child
	 * @param[in]  var   Variable of the node
	 *
	 * @return  Pointer to the created internal node
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend NodePtrType CreateInternal(NodePtrType low, NodePtrType high,
		const typename NodePtrType::VarType& var);

	/**
	 * @brief  Deletes a leaf node
	 *
	 * This function deletes the leaf node given by the passed node pointer.
	 *
	 * @param[in]  node  Pointer to the leaf node to be deleted
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend void DeleteLeafNode(NodePtrType node);

	/**
	 * @brief  Deletes an internal node
	 *
	 * This function deletes the internal node given by the passed node
	 * pointer.
	 *
	 * @param[in]  node  Pointer to the internal node to be deleted
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend void DeleteInternalNode(NodePtrType node);

	/**
	 * @brief  Checks if a node pointer is @p NULL
	 *
	 * This function determines whether the passed node pointer points to @p
	 * NULL.
	 *
	 * @param[in]  node  Node pointer 
	 *
	 * @tparam NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend bool IsNull(NodePtrType node);

	/**
	 * @brief  Checks if a node is a leaf
	 *
	 * This function determines whether the node is a leaf node of an MTBDD.
	 *
	 * @param[in]  node  Pointer to the node to be checked 
	 *
	 * @return  @p true if @p node is a leaf, @p false otherwise
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend bool IsLeaf(const NodePtrType& node);

	/**
	 * @brief  Checks if a node is an internal node
	 *
	 * This function determines whether the node is an interanal node of an
	 * MTBDD.
	 *
	 * @param[in]  node  Pointer to the node to be checked 
	 *
	 * @return  @p true if @p node is an internal node, @p false otherwise
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend bool IsInternal(const NodePtrType& node);

	/**
	 * @brief  Gets variable from an internal node
	 *
	 * The function to retrieve variable from the internal node pointed to by
	 * the passed MTBDD node pointer.
	 *
	 * @param[in]  node  Pointer to an internal node
	 *
	 * @return  The variable represented by the internal node
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend const typename NodePtrType::VarType& GetVarFromInternal(
		NodePtrType& node);

	/**
	 * @brief  Gets the @e low child of an internal node
	 *
	 * Retrieves the pointer to the @e low child of the internal node pointed
	 * to by the passed MTBDD node pointer.
	 *
	 * @param[in]  node  Pointer to the internal node
	 *
	 * @return  The @e low child of @p node
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend NodePtrType GetLowFromInternal(NodePtrType& node);

	/**
	 * @copydoc GetLowFromInternal(NodePtrType&)
	 */
	template <typename NodePtrType>
	friend NodePtrType GetLowFromInternal(const NodePtrType& node);

	/**
	 * @brief  Gets the @e high child of an internal node
	 *
	 * Retrieves the pointer to the @e high child of the internal node pointed
	 * to by the passed MTBDD node pointer.
	 *
	 * @param[in]  node  Pointer to the internal node
	 *
	 * @return  The @e high child of @p node
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend NodePtrType GetHighFromInternal(NodePtrType& node);

	/**
	 * @copydoc GetHighFromInternal(NodePtrType&)
	 */
	template <typename NodePtrType>
	friend NodePtrType GetHighFromInternal(const NodePtrType& node);

	/**
	 * @brief  Gets data from a leaf
	 *
	 * The function to retrieve data from the leaf node pointed to by the
	 * passed MTBDD node pointer.
	 *
	 * @param[in]  node  Pointer to a leaf node
	 *
	 * @return  The data value stored in the leaf node
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend const typename NodePtrType::DataType& GetDataFromLeaf(
		const NodePtrType& node);

	/**
	 * @brief  Gets leaf's reference counter
	 *
	 * Retrieves the value of the reference counter of the leaf node pointed
	 * to by given pointer.
	 *
	 * @param[in]  node  Pointer to the leaf node
	 *
	 * @return  The value of the reference counter
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend const typename NodePtrType::VarType& GetLeafRefCnt(
		const NodePtrType node);

	/**
	 * @brief  Increments nodes reference counter
	 *
	 * The function that increments the reference counter of node pointed to
	 * by the passed pointer.
	 *
	 * @param[in]  node  Pointer to the node
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend void IncrementRefCnt(NodePtrType node);

	/**
	 * @brief  Decrements leaf's reference counter
	 *
	 * This function decrements the value of the reference counter of the leaf
	 * of the passed node pointer and returns the new value.
	 *
	 * @param[in]  node  The node for decrementing reference counter
	 *
	 * @return  The new value of the reference counter
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend const typename NodePtrType::RefCntType& DecrementLeafRefCnt(
		NodePtrType node);

	/**
	 * @brief  Decrements internal node's reference counter
	 *
	 * This function decrements the value of the reference counter of the
	 * internal node of the passed node pointer and returns the new value.
	 *
	 * @param[in]  node  The node for decrementing reference counter
	 *
	 * @return  The new value of the reference counter
	 *
	 * @tparam  NodePtrType  Type of node pointer
	 */
	template <typename NodePtrType>
	friend const typename NodePtrType::RefCntType& DecrementInternalRefCnt(
		NodePtrType node);

	/**
	 * @brief  Generates hash of MTBDDNodePtr
	 *
	 * This is a function required by Boost to be able to use MTBDDNodePtr in
	 * hash tables, etc. The function generates the hash of a node pointer.
	 *
	 * @param[in]  node  Pointer to the node
	 *
	 * @return  Hash value of the pointer
	 *
	 * @tparam  Data  Data type of leaf nodes
	 */
	template <typename DataType>
	friend size_t hash_value(const MTBDDNodePtr<DataType>& node);

	/**
	 * @brief  Overloaded operator << for std::ostream
	 *
	 * This is the overloaded operator << for std::ostream that enables easy
	 * conversion of the pointer into a string.
	 *
	 * @param[inout]  os    std::ostream object
	 * @param[in]     node  Pointer to the node
	 *
	 * @return  Modified std::ostream object
	 *
	 * @tparam  Data  Data type of leaf nodes
	 */
	template <typename DataType>
	friend std::ostream& operator<<(std::ostream& os,
		const MTBDDNodePtr<DataType>& node);
};


namespace
{
	/**
	 * @brief  Internal MTBDD node
	 *
	 * The data type for an internal MTBDD node. The node contains 2 children
	 * pointers, a variable and a reference counter.
	 *
	 * @tparam  Data  The data type of MTBDD's leaves
	 */
	template <
		typename Data
	>
	struct InternalNode
	{
	public:   // public data types

		/**
		 * @brief  Leaves data type
		 *
		 * The data type of leaf nodes.
		 */
		typedef Data DataType;

		/**
		 * @brief  MTBDD node pointer type
		 *
		 * The data type of MTBDD node pointer (internal or leaf).
		 */
		typedef VATA::MTBDDPkg::MTBDDNodePtr<DataType> NodePtrType;

		/**
		 * @brief  Reference counter data type
		 *
		 * The data type of the reference counter.
		 */
		typedef typename NodePtrType::RefCntType RefCntType;

		/**
		 * @brief  Variable data type
		 *
		 * The data type of the varible in the node.
		 */
		typedef typename NodePtrType::VarType VarType;

	private:  // private data members

		/**
		 * @brief  The @e low child
		 *
		 * The child for value @p 0 of the node's variable.
		 */
		NodePtrType low_;

		/**
		 * @brief  The @e high child
		 *
		 * The child for value @p 1 of the node's variable.
		 */
		NodePtrType high_;

		/**
		 * @brief  Boolean variable
		 *
		 * The Boolean variable represented by the node
		 */
		VarType var_;

		/**
		 * @brief  Reference counter
		 *
		 * The counter of references to the node.
		 */
		RefCntType refcnt_;

	public:   // public methods

		/**
		 * @brief  Constructor
		 *
		 * Constructs InternalNode from components.
		 *
		 * @param[in]  low     The node's @e low child pointer
		 * @param[in]  high    The node's @e high child pointer
		 * @param[in]  var     The node's Boolean variable
		 * @param[in]  refcnt  Value of reference counter
		 */
		GCC_DIAG_OFF(maybe-uninitialized)     // I know what I'm doing!!!
		InternalNode(NodePtrType low, NodePtrType high, const VarType& var,
			const RefCntType& refcnt) :
			low_(low),
			high_(high),
			var_(var),
			refcnt_(refcnt)
		{
			// Assertions
			assert(!IsNull(low));
			assert(!IsNull(high));
		}
		GCC_DIAG_ON(maybe-uninitialized)

		/**
		 * @brief  Gets the node's variable
		 *
		 * Returns the Boolean variable of the node.
		 *
		 * @return  Boolean variable of the node
		 */
		inline const VarType& GetVar() const
		{
			return var_;
		}

		/**
		 * @brief  Gets the @e low child
		 *
		 * Returns the @e low child of the node, i.e., the child node
		 * corresponding to assigning @p 0 to the variable.
		 *
		 * @return  The @e low child of the node
		 */
		inline const NodePtrType& GetLow() const
		{
			return low_;
		}

		/**
		 * @brief  Gets the @e high child
		 *
		 * Returns the @e high child of the node, i.e., the child node
		 * corresponding to assigning @p 1 to the variable.
		 *
		 * @return  The @e high child of the node
		 */
		inline const NodePtrType& GetHigh() const
		{
			return high_;
		}

		/**
		 * @brief  Gets the reference counter value
		 *
		 * Returns the value of the node's reference counter.
		 *
		 * @return  Node's reference counter value
		 */
		inline const RefCntType& GetRefCnt() const
		{
			return refcnt_;
		}

		/**
		 * @brief  Increments the reference counter value
		 *
		 * Increments the value of the node's reference counter.
		 */
		inline void IncrementRefCnt()
		{
			++refcnt_;
		}

		/**
		 * @brief  Decrements the reference counter value
		 *
		 * Decrements the value of the node's reference counter and returns the
		 * new value.
		 *
		 * @return  Decremented reference counter value
		 */
		inline const RefCntType& DecrementRefCnt()
		{
			// Assertions
			assert(refcnt_ > 0);

			return --refcnt_;
		}
	};


	/**
	 * @brief  Leaf MTBDD node
	 *
	 * The data type for a leaf MTBDD node. The node contains a value from the
	 * domain of the MTBDD.
	 *
	 * @tparam  Data  The data type of value in the leaf
	 */
	template <
		typename Data
	>
	struct LeafNode
	{
	public:   // public data members

		/**
		 * @brief  Data type of the value
		 *
		 * The data type of the value stored in the leaf.
		 */
		typedef Data DataType;

		/**
		 * @brief  Data type of node pointer
		 *
		 * The data type of the MTBDD's node pointer.
		 */
		typedef VATA::MTBDDPkg::MTBDDNodePtr<DataType> NodePtr;

		/**
		 * @brief  Data type of reference counter
		 *
		 * The data type of the reference counter.
		 */
		typedef typename NodePtr::RefCntType RefCntType;

	private:  // private data members

		/**
		 * @brief  Stored data value
		 *
		 * Data value stored in the leaf node.
		 */
		DataType data_;

		/**
		 * @brief  Reference counter
		 *
		 * Counter of references to the leaf node.
		 */
		RefCntType refcnt_;

	public:   // public methods


		/**
		 * @brief  Constructor
		 *
		 * Constructs the leaf from components.
		 *
		 * @param[in]  data    The data value
		 * @param[in]  refcnt  The reference counter
		 */
		LeafNode(const DataType& data, const RefCntType& refcnt)
			: data_(data),
				refcnt_(refcnt)
		{ }

		/**
		 * @brief  Gets data from the leaf
		 *
		 * Returns the data value stored in the leaf.
		 *
		 * @return  Leaf's data value
		 */
		inline const DataType& GetData() const
		{
			return data_;
		}

		/**
		 * @brief  Gets data from the leaf
		 *
		 * Returns the data value stored in the leaf.
		 *
		 * @return  Leaf's data value
		 */
		inline const RefCntType& GetRefCnt() const
		{
			return refcnt_;
		}

		/**
		 * @brief  Increments the reference counter
		 *
		 * Increments the value of the leaf's reference counter.
		 */
		inline void IncrementRefCnt()
		{
			++refcnt_;
		}

		/**
		 * @brief  Decreemtns the reference counter
		 *
		 * Decrements the value of the leaf's reference counter and returns the
		 * new value.
		 *
		 * @return  The decremented value
		 */
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


		template <typename Data>
		inline size_t hash_value(const MTBDDNodePtr<Data>& node)
		{
			// Assertions
			assert(!IsNull(node));

			boost::hash<uintptr_t> hasher;
			return hasher(node.addr_);
		}

		template <typename Data>
		std::ostream& operator<<(std::ostream& os, const MTBDDNodePtr<Data>& node)
		{
			// Assertions
			assert(!IsNull(node));

			os << VATA::Util::Convert::ToString(node.addr_);

			return os;
		}
	}
}


#endif
