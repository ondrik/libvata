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
#include	<vata/mtbdd/mtbdd_node.hh>
#include	<vata/mtbdd/var_asgn.hh>
#include	<vata/vata.hh>

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
	typedef MTBDDNode<DataType> NodeType;
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
		if (value == defaultValue)
		{	// in case an MTBDD with a single leaf is desired
			return CreateLeaf(value);
		}

		// bottom leaves
		NodeType* leaf = CreateLeaf(value);
		NodeType* sink = CreateLeaf(defaultValue);

		// working node
		NodeType* node = leaf;

		for (size_t i = 0; i < varOrdering->size(); ++i)
		{	// construct the MTBDD according to the variable ordering
			VarType var =	(*varOrdering)[i];
			if (asgn.GetIthVariableValue(var) == VariableAssignment::ONE)
			{	// in case the variable is 1
				node = CreateInternal(sink, node, var);
			}
			else if (asgn.GetIthVariableValue(var) == VariableAssignment::ZERO)
			{	// in case the variable is 0
				node = CreateInternal(node, sink, var);
			}
			// otherwise don't care about the variable
		}

		if (node == leaf)
		{	// in case the MTBDD is with a single leaf
			DeleteNode(sink);
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

		DeleteMTBDDdag(root_);

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
		if (root_ != static_cast<NodeType*>(0))
		{
			DeleteMTBDDdag(root_);
		}
	}
};

#endif
