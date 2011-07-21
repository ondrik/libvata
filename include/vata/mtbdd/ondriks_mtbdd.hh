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

// VATA headers
#include	<vata/vata.hh>
#include	<vata/mtbdd/mtbdd_node.hh>
#include	<vata/mtbdd/var_asgn.hh>
#include	<vata/util/triple.hh>

// Standard library headers
#include	<cassert>
#include	<stdint.h>
#include	<stdexcept>
#include	<vector>
#include  <tr1/unordered_set>

// Loki headers
#include	<loki/SmartPtr.h>

// Boost library headers
#include <boost/functional/hash.hpp>

namespace VATA
{
	namespace MTBDDPkg
	{
		template <
			typename Data
		>
		class OndriksMTBDD;
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
class VATA::MTBDDPkg::OndriksMTBDD
{
	template <typename, typename>
	friend class AbstractApply1Functor;

	template <typename, typename, typename>
	friend class AbstractApply2Functor;

	template <typename, typename, typename, typename>
	friend class AbstractApply3Functor;

public:   // public data types

	typedef Data DataType;

private:  // private data types

	typedef MTBDDNodePtr<DataType> NodePtrType;
	typedef typename NodePtrType::LeafType LeafType;
	typedef typename NodePtrType::InternalType InternalType;

public:   // public data types

	typedef typename NodePtrType::InternalType::VarType VarType;
	typedef std::vector<VarType> PermutationTable;
	typedef Loki::SmartPtr<PermutationTable> PermutationTablePtr;

private:  // private data types

	typedef VATA::Util::Triple<NodePtrType, NodePtrType, VarType>
		InternalAddressType;

	typedef std::tr1::unordered_map<InternalAddressType, NodePtrType,
		typename InternalAddressType::Hasher> InternalCacheType;

	typedef std::tr1::unordered_map<DataType, NodePtrType> LeafCacheType;

private:  // private data members

	NodePtrType root_;

	DataType defaultValue_;

	/**
	 * @brief  Permutation table representing variable ordering
	 *
	 * The table gives variable ordering. Semantics is that at index @p i there
	 * is the @p i -th variable. The ordering is bottom-up, i.e., MTBDD node
	 * labelled with variable 0 is directly above a leaf.
	 */
	PermutationTablePtr varOrdering_;

	static LeafCacheType leafCache_;
	static InternalCacheType internalCache_;


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
	static NodePtrType constructMTBDD(const VariableAssignment& asgn,
		const DataType& value, const DataType& defaultValue,
		const PermutationTablePtr& varOrdering)
	{
		// the leaf with the desired value
		NodePtrType leaf = spawnLeaf(value);

		if (value == defaultValue)
		{	// in case an MTBDD with a single leaf is desired
			IncrementRefCnt(leaf);
			return leaf;
		}

		// the sink leaf
		NodePtrType sink = spawnLeaf(defaultValue);

		// working node
		NodePtrType node = leaf;

		for (size_t i = 0; i < varOrdering->size(); ++i)
		{	// construct the MTBDD according to the variable ordering
			VarType var =	(*varOrdering)[i];
			if (asgn.GetIthVariableValue(var) == VariableAssignment::ONE)
			{	// in case the variable is 1
				node = spawnInternal(sink, node, var);
			}
			else if (asgn.GetIthVariableValue(var) == VariableAssignment::ZERO)
			{	// in case the variable is 0
				node = spawnInternal(node, sink, var);
			}
			// otherwise don't care about the variable
		}

		if (node == leaf)
		{	// in case the MTBDD is with a single leaf
			assert(IsLeaf(leaf));

			if (GetLeafRefCnt(sink) == 0)
			{	// in case there is no one pointing to the sink
				disposeOfLeafNode(sink);
			}
		}

		IncrementRefCnt(node);
		return node;
	}

	OndriksMTBDD(NodePtrType root, const DataType& defaultValue,
		const PermutationTablePtr& varOrdering)
		: root_(root),
			defaultValue_(defaultValue),
			varOrdering_(varOrdering)
	{
		// Assertions
		assert(!IsNull(root_));
	}


	NodePtrType getRoot() const
	{
		return root_;
	}

	static void disposeOfLeafNode(NodePtrType node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsLeaf(node));

		if (leafCache_.erase(GetDataFromLeaf(node)) != 1)
		{	// in case the leaf was not cached
			throw std::runtime_error(
				"Deleting a leaf which is not in the leaf cache!");
		}

		DeleteLeafNode(node);
	}

	static void disposeOfInternalNode(NodePtrType node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsInternal(node));

		InternalAddressType addr(GetLowFromInternal(node),
			GetHighFromInternal(node), GetVarFromInternal(node));
		if (internalCache_.erase(addr) != 1)
		{	// in case the internal was not cached
			throw std::runtime_error(
				"Deleting an internal which is not in the internal cache!");
		}

		recursivelyDeleteMTBDDNode(GetLowFromInternal(node));
		recursivelyDeleteMTBDDNode(GetHighFromInternal(node));

		DeleteInternalNode(node);
	}

	static void recursivelyDeleteMTBDDNode(NodePtrType node)
	{
		// Assertions
		assert(!IsNull(node));

		if (IsLeaf(node))
		{	// for leaves
			if (DecrementLeafRefCnt(node) == 0)
			{	// this reference to node is the last
				disposeOfLeafNode(node);
			}
		}
		else
		{	// for internal nodes
			assert(IsInternal(node));

			if (DecrementInternalRefCnt(node) == 0)
			{	// this reference to node is the last
				disposeOfInternalNode(node);
			}
		}
	}

	inline void deleteMTBDD()
	{
		if (!IsNull(root_))
		{
			recursivelyDeleteMTBDDNode(root_);
			root_ = 0;
		}
	}

	static inline NodePtrType spawnLeaf(const DataType& data)
	{
		NodePtrType result = 0;

		typename LeafCacheType::const_iterator itLC;
		if ((itLC = leafCache_.find(data)) != leafCache_.end())
		{	// in case given leaf is already cached
			result = itLC->second;
		}
		else
		{	// if the leaf doesn't exist
			result = CreateLeaf(data);
			leafCache_.insert(std::make_pair(data, result));
		}

		assert(!IsNull(result));
		return result;
	}

	static inline NodePtrType spawnInternal(
		NodePtrType low, NodePtrType high, const VarType& var)
	{
		NodePtrType result = 0;

		InternalAddressType addr(low, high, var);
		typename InternalCacheType::const_iterator itIC;
		if ((itIC = internalCache_.find(addr)) != internalCache_.end())
		{	// in case given internal is already cached
			result = itIC->second;
		}
		else
		{	// if the internal doesn't exist
			result = CreateInternal(low, high, var);
			IncrementRefCnt(low);
			IncrementRefCnt(high);
			internalCache_.insert(std::make_pair(addr, result));
		}

		assert(!IsNull(result));
		return result;
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
		: root_(static_cast<InternalType*>(0)),
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
		:	root_(static_cast<InternalType*>(0)),
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
		assert(!IsNull(root_));

		IncrementRefCnt(root_);
	}

	OndriksMTBDD& operator=(const OndriksMTBDD& mtbdd)
	{
		// Assertions
		assert(!IsNull(root_));

		if (&mtbdd == this)
			return *this;

		deleteMTBDD();

		root_ = mtbdd.root_;
		IncrementRefCnt(root_);

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
		NodePtrType node = root_;

		while (!IsLeaf(node))
		{	// try to proceed according to the assignment
			const VarType& var = GetVarFromInternal(node);

			if (asgn.GetIthVariableValue(var) == VariableAssignment::ONE)
			{	// if one
				node = GetHighFromInternal(node);
			}
			else
			{	// if zero or don't care
				node = GetLowFromInternal(node);
			}
		}

		return GetDataFromLeaf(node);
	}

	~OndriksMTBDD()
	{
		deleteMTBDD();
	}
};

template <typename Data>
typename VATA::MTBDDPkg::OndriksMTBDD<Data>::LeafCacheType
	VATA::MTBDDPkg::OndriksMTBDD<Data>::leafCache_;

template <typename Data>
typename VATA::MTBDDPkg::OndriksMTBDD<Data>::InternalCacheType
	VATA::MTBDDPkg::OndriksMTBDD<Data>::internalCache_;

#endif
