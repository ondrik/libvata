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
#include	<vata/variable_assignment.hh>

// Loki headers
#include	<loki/SmartPtr.h>


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

			template <
				typename Data1,
				typename Data2,
				typename DataOut
			>
			class AbstractApply2Functor;

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
	template <typename, typename, typename>
	friend class AbstractApply2Functor;

public:   // public data types

	typedef Data DataType;
	typedef MTBDDPkg::MTBDDNodePkg::MTBDDNode<DataType> NodeType;
	typedef typename NodeType::VarType VarType;
	typedef std::vector<typename NodeType::VarType> PermutationTable;
	typedef Loki::SmartPtr<PermutationTable> PermutationTablePtr;

private: // private data members

	// TODO: add something different than simple pointer (e.g. smart pointer
	// with destructive copy
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
		: root_(),
			defaultValue_(defaultValue),
			varOrdering_()
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
		:	root_(),
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

template <
	typename Data1,
	typename Data2,
	typename DataOut
>
class VATA::Private::MTBDDPkg::AbstractApply2Functor
{
public:   // Public data types

	typedef Data1 Data1Type;
	typedef Data2 Data2Type;
	typedef DataOut DataOutType;

	typedef OndriksMTBDD<Data1Type> MTBDD1Type;
	typedef OndriksMTBDD<Data2Type> MTBDD2Type;
	typedef OndriksMTBDD<DataOutType> MTBDDOutType;

	typedef typename MTBDD1Type::NodeType Node1Type;
	typedef typename MTBDD2Type::NodeType Node2Type;
	typedef typename MTBDDOutType::NodeType NodeOutType;

	typedef typename MTBDDOutType::VarType VarType;

private:  // Private data members

	const MTBDD1Type* mtbdd1_;
	const MTBDD2Type* mtbdd2_;

private:  // Private methods

	AbstractApply2Functor(const AbstractApply2Functor&);
	AbstractApply2Functor& operator=(const AbstractApply2Functor&);


	inline static char classifyCase(const Node1Type* node1, const Node2Type* node2)
	{
		if (isInternal(node1))
		{	// node1 is internal
			VarType var1 = getVarFromInternal(node1);

			if (isInternal(node2))
			{	// node2 is also internal
				VarType var2 = getVarFromInternal(node2);

				if (var1 == var2)
				{	// in case the variables match
					return 'E';  // "Equal" variables
				}
				if (var1 > var2)
				{
					return 'B';  // node1 is "Bigger" than node2
				}
				else
				{
					assert(var1 < var2);
					return 'S';  // node1 is "Smaller" than node2
				}
			}
			else
			{	// node2 is a leaf
				assert(isLeaf(node2));
				return 'B';  // node1 is "Bigger" than node2
			}
		}
		else if (isInternal(node2))
		{	// node1 is a leaf
			assert(isLeaf(node1));
			return 'S';  // node1 is "Smaller" than node2
		}
		else
		{	// for the terminal case
			assert(isLeaf(node1) && isLeaf(node2));
			return 'T';  // "Terminal" case
		}
	}

	typename MTBDDOutType::NodeType* recDescend(
		const Node1Type* node1, const Node2Type* node2)
	{
		// Assertions
		assert(node1 != static_cast<Node1Type*>(0));
		assert(node2 != static_cast<Node2Type*>(0));

		using namespace VATA::Private::MTBDDPkg::MTBDDNodePkg;

		// TODO: caching!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		VarType var;
		NodeOutType* lowTree = static_cast<NodeOutType*>(0);
		NodeOutType* highTree = static_cast<NodeOutType*>(0);

		char relation = classifyCase(node1, node2);
		switch (relation)
		{
			// for both leaves
			case 'T': {
					NodeOutType* result = createLeaf(DataOperation(
						getDataFromLeaf(node1), getDataFromLeaf(node2)));

					// TODO: cache!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					return result;
				}

			// for internal nodes with the same variable
			case 'E': {
					var = getVarFromInternal(node1);
					lowTree = recDescend(getLowFromInternal(node1), getLowFromInternal(node2));
					highTree = recDescend(getHighFromInternal(node1), getHighFromInternal(node2));
					break;
				}

			// for internal nodes with node1 above node2
			case 'B': {
					var = getVarFromInternal(node1);
					lowTree = recDescend(getLowFromInternal(node1), node2);
					highTree = recDescend(getHighFromInternal(node1), node2);
					break;
				}

			// for internal nodes with node1 below node2
			case 'S': {
					var = getVarFromInternal(node2);
					lowTree = recDescend(node1, getLowFromInternal(node2));
					highTree = recDescend(node1, getHighFromInternal(node2));
					break;
				}

			// this should never happen
			default: {
					assert(false);
					break;
				}
		}

		if (lowTree == highTree)
		{	// in case both trees are isomorphic (when caching is enabled)
			// TODO: cache!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			return lowTree;
		}
		else
		{	// in case both trees are distinct
			NodeOutType* result = createInternal(lowTree, highTree, var);

			// TODO: cache!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			return result;
		}
	}

public:   // Public methods

	AbstractApply2Functor()
		: mtbdd1_(static_cast<MTBDD1Type*>(0)),
			mtbdd2_(static_cast<MTBDD2Type*>(0))
	{ }

	MTBDDOutType operator()(const MTBDD1Type& mtbdd1, const MTBDD2Type& mtbdd2)
	{
		// store the MTBDDs
		mtbdd1_ = &mtbdd1;
		mtbdd2_ = &mtbdd2;

		if (mtbdd1_->GetVarOrdering() != mtbdd2_->GetVarOrdering())
		{	// in case the MTBDDs have a different variable ordering
			assert(false);
			// TODO
		}

		// recursively descend the MTBDD and generate a new one
		typename MTBDDOutType::NodeType* root = recDescend(mtbdd1_->getRoot(),
			mtbdd2_->getRoot());

		// compute the new default value
		DataOutType defaultValue = DataOperation(mtbdd1_->GetDefaultValue(),
			mtbdd2_->GetDefaultValue());

		// wrap it all up
		return MTBDDOutType(root, defaultValue, mtbdd1_->GetVarOrdering());
	}

	virtual DataOutType DataOperation(
		const Data1Type& data1, const Data2Type& data2) = 0;

protected:// Protected methods

	inline const MTBDD1Type& getMTBDD1() const
	{
		assert(mtbdd1_ != static_cast<MTBDD1Type*>(0));
		return *mtbdd1_;
	}

	inline const MTBDD2Type& getMTBDD2() const
	{
		assert(mtbdd2_ != static_cast<MTBDD2Type*>(0));
		return *mtbdd2_;
	}


	virtual ~AbstractApply2Functor()
	{ }
};

#endif
