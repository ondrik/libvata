/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    My own implementation of multi-terminal binary decision diagrams (MTBDDs)
 *
 *****************************************************************************/

#ifndef _VATA_ONDRIKS_MTBDD_HH_
#define _VATA_ONDRIKS_MTBDD_HH_


// Standard library headers
#include	<cassert>
#include	<stdint.h>
#include	<stdexcept>
#include	<vector>
#include  <tr1/unordered_set>

// VATA headers
#include	<vata/mtbdd/var_asgn.hh>
#include	<vata/vata.hh>

// Loki headers
#include	<loki/SmartPtr.h>

// Boost library headers
#include <boost/functional/hash.hpp>

namespace VATA
{
	namespace Private
	{
		namespace MTBDDPkg
		{
			template <
				typename Data
			>
			class OndriksMTBDD;

			namespace MTBDDNodePkg
			{
				/**
				 * @brief  MTBDD node
				 *
				 * Union for MTBDD node. It holds either data (for leaf (or sink)
				 * nodes), or pointers to low and high nodes together with variable
				 * name (for internal nodes). The type of the node (leaf or internal) is
				 * determined using the pointer to the node: if the address is even, then
				 * the node is internal, if it is odd, then it is leaf.
				 *
				 * @tparam  Data  The type of data to be stored into leaf nodes
				 */
				template
				<
					typename Data
				>
				union MTBDDNode
				{
					// Data types
					typedef Data DataType;
					typedef uintptr_t VarType;

					// Disjunct data
					struct
					{
						MTBDDNode* low;
						MTBDDNode* high;
						VarType var;
					} internal;

					DataType data;
				};


				template <class NodeType>
				inline NodeType* makeLeaf(NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));

					return reinterpret_cast<NodeType*>(
						reinterpret_cast<uintptr_t>(node) | 1);
				}

				template <class NodeType>
				inline NodeType* makeInternal(NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));

					return node;
				}

				template <class NodeType>
				inline bool isLeaf(const NodeType* node)
				{
					assert(node != static_cast<NodeType*>(0));
					return (reinterpret_cast<uintptr_t>(node) % 2);
				}

				template <class NodeType>
				inline bool isInternal(const NodeType* node)
				{
					assert(node != static_cast<NodeType*>(0));
					return !(isLeaf(node));
				}

				template <class NodeType>
				inline NodeType* leafToNode(NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));
					assert(isLeaf(node));

					return reinterpret_cast<NodeType*>(
						reinterpret_cast<uintptr_t>(node) ^ 1);
				}

				template <class NodeType>
				inline NodeType* internalToNode(NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));
					assert(isInternal(node));

					return node;
				}

				template <class NodeType>
				inline const typename NodeType::DataType& getDataFromLeaf(
					const NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));
					assert(isLeaf(node));

					return leafToNode(node)->data;
				}

				template <class NodeType>
				inline const typename NodeType::VarType& getVarFromInternal(
					const NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));
					assert(isInternal(node));

					return internalToNode(node)->internal.var;
				}

				template <class NodeType>
				inline const NodeType* getLowFromInternal(const NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));
					assert(isInternal(node));

					return internalToNode(node)->internal.low;
				}

				template <class NodeType>
				inline NodeType* getLowFromInternal(NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));
					assert(isInternal(node));

					return internalToNode(node)->internal.low;
				}

				template <class NodeType>
				inline const NodeType* getHighFromInternal(const NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));
					assert(isInternal(node));

					return internalToNode(node)->internal.high;
				}

				template <class NodeType>
				inline NodeType* getHighFromInternal(NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));
					assert(isInternal(node));

					return internalToNode(node)->internal.high;
				}

				template <typename DataType>
				MTBDDNode<DataType>* createLeaf(const DataType& data)
				{
					typedef MTBDDNode<DataType> NodeType;

					// TODO: create allocator						
					NodeType* newNode = new NodeType;
					newNode->data = data;

					return makeLeaf(newNode);
				}

				template <class NodeType>
				NodeType* createInternal(
					NodeType* low, NodeType* high, typename NodeType::VarType var)
				{
					// Assertions
					assert(low != static_cast<NodeType*>(0));
					assert(high != static_cast<NodeType*>(0));

					// TODO: create allocator
					NodeType* newNode = new NodeType;
					newNode->internal.low = low;
					newNode->internal.high = high;
					newNode->internal.var = var;

					return makeInternal(newNode);
				}

				template <class NodeType>
				inline void deleteNode(NodeType* node)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));

					if (isLeaf(node))
					{
						delete leafToNode(node);
					}
					else if (isInternal(node))
					{
						delete internalToNode(node);
					}
					else
					{
						throw std::runtime_error("Invalid type of MTBDD node.");
					}
				}

				template <class NodeType>
				void traverseMTBDDdagAndCollectNodes(NodeType* node,
						std::tr1::unordered_set<NodeType*>& ht)
				{
					// Assertions
					assert(node != static_cast<NodeType*>(0));

					ht.insert(node);

					if (isInternal(node))
					{
						traverseMTBDDdagAndCollectNodes(getLowFromInternal(node), ht);
						traverseMTBDDdagAndCollectNodes(getHighFromInternal(node), ht);
					}
				}

				template <class NodeType>
				void deleteMTBDDdag(NodeType* root)
				{
					// Assertions
					assert(root != static_cast<NodeType*>(0));

					typedef std::tr1::unordered_set<NodeType*> NodeHashTable;

					NodeHashTable ht;
					traverseMTBDDdagAndCollectNodes(root, ht);

					for (typename NodeHashTable::iterator iNodes = ht.begin();
						iNodes != ht.end(); ++iNodes)
					{	// delete all nodes
						deleteNode(*iNodes);
					}
				}
			}
		}
	}
}


/**
 * @brief   Class representing MTBDD
 * @author  Ondra Lengal <ilengal@fit.vutbr.cz>
 * @date    2011
 *
 * This class represents a single multi-terminal binary decision diagram
 * (MTBDD).
 *
 * @tparam  Data  The type of leaves
 */
template <
	typename Data
>
class VATA::Private::MTBDDPkg::OndriksMTBDD
{
	template <typename, typename>
	friend class AbstractApply1Functor;

	template <typename, typename, typename>
	friend class AbstractApply2Functor;

public:   // public data types

	typedef Data DataType;
	typedef MTBDDPkg::MTBDDNodePkg::MTBDDNode<DataType> NodeType;
	typedef typename NodeType::VarType VarType;
	typedef std::vector<typename NodeType::VarType> PermutationTable;
	typedef Loki::SmartPtr<PermutationTable> PermutationTablePtr;

private: // private data members

	NodeType* root_;

	DataType defaultValue_;

	/**
	 * @brief  Permutation table representing variable ordering
	 *
	 * The table gives variable ordering. Semantics is that at index @p i there
	 * is the @p i -th variable. The ordering is bottom-up, i.e., MTBDD node
	 * labelled with variable 0 is directly above a leaf.
	 */
	PermutationTablePtr varOrdering_;


private:  // private methods


	/**
	 * @brief  Function for constructing an MTBDD
	 *
	 * This function is used for constructing a new MTBDD according to the given
	 * variable ordering @p varOrdering, such that the variable assignment @p
	 * asgn is set to @p value and all other assignments are set to @p
	 * defaultValue.
	 *
	 * @param  asgn          Variable assignment to be set to @p value
	 * @param  value         Value to be set for assignment @p asgn
	 * @param  defaultValue  Value to be set for all assignments other than @p
	 *                       asgn
	 * @param  varOrdering   Ordering of variables
	 *
	 * @return  The constructed MTBDD
	 */
	static NodeType* constructMTBDD(const VariableAssignment& asgn,
		const DataType& value, const DataType& defaultValue,
		const PermutationTablePtr& varOrdering)
	{
		using namespace VATA::Private::MTBDDPkg::MTBDDNodePkg;

		if (value == defaultValue)
		{	// in case an MTBDD with a single leaf is desired
			return createLeaf(value);
		}

		// bottom leaves
		NodeType* leaf = createLeaf(value);
		NodeType* sink = createLeaf(defaultValue);

		// working node
		NodeType* node = leaf;

		for (size_t i = 0; i < varOrdering->size(); ++i)
		{	// construct the MTBDD according to the variable ordering
			VarType var =	(*varOrdering)[i];
			if (asgn.GetIthVariableValue(var) == VariableAssignment::ONE)
			{	// in case the variable is 1
				node = createInternal(sink, node, var);
			}
			else if (asgn.GetIthVariableValue(var) == VariableAssignment::ZERO)
			{	// in case the variable is 0
				node = createInternal(node, sink, var);
			}
			// otherwise don't care about the variable
		}

		if (node == leaf)
		{	// in case the MTBDD is with a single leaf
			deleteNode(sink);
		}

		return node;
	}

protected:// Protected methods

	OndriksMTBDD(NodeType* root, const DataType& defaultValue,
		const PermutationTablePtr& varOrdering)
		: root_(root),
			defaultValue_(defaultValue),
			varOrdering_(varOrdering)
	{
		// Assertions
		assert(root_ != static_cast<NodeType*>(0));
	}


	const NodeType* getRoot() const
	{
		return root_;
	}


public:   // public methods


	/**
	 * @brief  Constructor with default variable ordering
	 *
	 * This constructor creates a new MTBDD with the default variable ordering,
	 * such that the variable assignment @p asgn is set to @p value and all
	 * other assignments are set to @p defaultValue.
	 *
	 * @param  asgn          Variable assignment to be set to @p value
	 * @param  value         Value to be set for assignment @p asgn
	 * @param  defaultValue  Value to be set for all assignments other than @p
	 *                       asgn
	 */
	OndriksMTBDD(const VariableAssignment& asgn,
		const DataType& value, const DataType& defaultValue)
		: root_(static_cast<NodeType*>(0)),
			defaultValue_(defaultValue),
			varOrdering_(static_cast<PermutationTable*>(0))
	{
		// create the variable permutation table (the variable ordering)
		PermutationTable* varOrd = new PermutationTable(asgn.VariablesCount());

		for (size_t i = 0; i < varOrd->size(); ++i)
		{	// fill the permutation table with the natural ordering
			(*varOrd)[i] = i;
		}

		varOrdering_ = varOrd;

		// create the MTBDD
		root_ = constructMTBDD(asgn, value, defaultValue_, varOrdering_);
	}


	/**
	 * @brief  Constructor with given variable ordering
	 *
	 * This constructor creates a new MTBDD with the variable ordering given as
	 * @p varOrdering, such that the variable assignment @p asgn is set to @p
	 * value and all other assignments are set to @p defaultValue.
	 *
	 * @param  asgn          Variable assignment to be set to @p value
	 * @param  value         Value to be set for assignment @p asgn
	 * @param  defaultValue  Value to be set for all assignments other than @p
	 *                       asgn
	 * @param  varOrdering   Ordering of variables
	 */
	OndriksMTBDD(const VariableAssignment& asgn, const DataType& value,
		const DataType& defaultValue, const PermutationTablePtr& varOrdering)
		:	root_(static_cast<NodeType*>(0)),
			defaultValue_(defaultValue),
			varOrdering_(varOrdering)
	{
		if (asgn.VariablesCount() > varOrdering->size())
		{
			throw std::runtime_error(
				"Variable assignment contains variables with an unknown ordering");
		}

		// create the MTBDD
		root_ = constructMTBDD(asgn, value, defaultValue_, varOrdering_);
	}

	OndriksMTBDD(const OndriksMTBDD& mtbdd)
		: root_(mtbdd.root_),
			defaultValue_(mtbdd.defaultValue_),
			varOrdering_(mtbdd.varOrdering_)
	{
		// Assertions
		assert(root_ != static_cast<NodeType*>(0));

		// destroy the original MTBDD
		const_cast<OndriksMTBDD&>(mtbdd).root_ = static_cast<NodeType*>(0);
	}

	OndriksMTBDD& operator=(const OndriksMTBDD& mtbdd)
	{
		// Assertions
		assert(root_ != static_cast<NodeType*>(0));

		if (&mtbdd == this)
			return *this;

		deleteMTBDDdag(root_);

		root_ = mtbdd.root_;

		// destroy the original MTBDD
		const_cast<OndriksMTBDD&>(mtbdd).root_ = static_cast<NodeType*>(0);

		defaultValue_ = mtbdd.defaultValue_;
		varOrdering_ = mtbdd.varOrdering_;

		return *this;
	}

	inline const DataType& GetDefaultValue() const
	{
		return defaultValue_;
	}

	inline size_t MaxHeight() const
	{
		return varOrdering_.size();
	}

	inline const PermutationTablePtr& GetVarOrdering() const
	{
		return varOrdering_;
	}

	/**
	 * @brief  Returns value for assignment
	 *
	 * Thsi method returns a value in the MTBDD that corresponds to given
	 * variable assignment @p asgn.
	 *
	 * @note  If there are multiple values that correspond to given variable
	 * assignment (e.g., because the assignment is very nondeterministic), an
	 * arbitrary value is returned.
	 *
	 * @param  asgn  Variable assignment
	 *
	 * @return  Value corresponding to variable assignment @p asgn
	 */
	const DataType& GetValue(const VariableAssignment& asgn) const
	{
		NodeType* node = root_;

		while (!isLeaf(node))
		{	// try to proceed according to the assignment
			const VarType& var = getVarFromInternal(node);

			if (asgn.GetIthVariableValue(var) == VariableAssignment::ONE)
			{	// if one
				node = getHighFromInternal(node);
			}
			else
			{	// if zero or don't care
				node = getLowFromInternal(node);
			}
		}

		return getDataFromLeaf(node);
	}

	~OndriksMTBDD()
	{
		if (root_ != static_cast<NodeType*>(0))
		{
			deleteMTBDDdag(root_);
		}
	}
};

#endif
