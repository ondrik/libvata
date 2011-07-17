/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Binary Apply functor for OndriksMTBDD
 *
 *****************************************************************************/

#ifndef _VATA_APPLY2FUNC_HH_
#define _VATA_APPLY2FUNC_HH_

// Standard library headers
#include  <tr1/unordered_set>

// VATA headers
#include	<vata/mtbdd/ondriks_mtbdd.hh>
#include	<vata/vata.hh>

// Boost library headers
#include <boost/functional/hash.hpp>


namespace VATA
{
	namespace Private
	{
		namespace MTBDDPkg
		{
			template <
				typename Data1,
				typename Data2,
				typename DataOut
			>
			class AbstractApply2Functor;
		}
	}
}


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

private:  // Private data types

	typedef std::pair<const Node1Type*, const Node2Type*> CacheAddressType;

	/**
	 * @brief  Hasher structure for a pair of keys
	 *
	 * This class is a hasher for a pair of keys.
	 */
	struct Hasher2
	{
		inline size_t operator()(const CacheAddressType& key) const
		{
			size_t seed  = 0;
			boost::hash_combine(seed, key.first);
			boost::hash_combine(seed, key.second);
			return seed;
		}
	};

	typedef std::tr1::unordered_map<CacheAddressType, NodeOutType*, Hasher2>
		CacheHashTable;

private:  // Private data members

	const MTBDD1Type* mtbdd1_;
	const MTBDD2Type* mtbdd2_;

	CacheHashTable ht;


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

		CacheAddressType cacheAddress(node1, node2);
		typename CacheHashTable::iterator itHt;
		if ((itHt = ht.find(cacheAddress)) != ht.end())
		{	// if the result is already known
			assert(itHt->second != static_cast<NodeOutType*>(0));
			return itHt->second;
		}

		VarType var;
		NodeOutType* lowOutTree = static_cast<NodeOutType*>(0);
		NodeOutType* highOutTree = static_cast<NodeOutType*>(0);

		char relation = classifyCase(node1, node2);
		switch (relation)
		{
			// for both leaves
			case 'T': {
					NodeOutType* result = createLeaf(ApplyOperation(
						getDataFromLeaf(node1), getDataFromLeaf(node2)));

					ht.insert(std::make_pair(cacheAddress, result));
					return result;
				}

			// for internal nodes with the same variable
			case 'E': {
					var = getVarFromInternal(node1);
					const Node1Type* low1Tree = getLowFromInternal(node1);
					const Node2Type* low2Tree = getLowFromInternal(node2);
					const Node1Type* high1Tree = getHighFromInternal(node1);
					const Node2Type* high2Tree = getHighFromInternal(node2);

					// Assertions for one condition of reduced MTBDDs
					assert(low1Tree != high1Tree);
					assert(low2Tree != high2Tree);

					lowOutTree = recDescend(low1Tree, low2Tree);
					highOutTree = recDescend(high1Tree, high2Tree);
					break;
				}

			// for internal nodes with node1 above node2
			case 'B': {
					var = getVarFromInternal(node1);
					const Node1Type* low1Tree = getLowFromInternal(node1);
					const Node1Type* high1Tree = getHighFromInternal(node1);

					// Assertion for one condition of reduced MTBDDs
					assert(low1Tree != high1Tree);

					lowOutTree = recDescend(low1Tree, node2);
					highOutTree = recDescend(high1Tree, node2);
					break;
				}

			// for internal nodes with node1 below node2
			case 'S': {
					var = getVarFromInternal(node2);
					const Node2Type* low2Tree = getLowFromInternal(node2);
					const Node2Type* high2Tree = getHighFromInternal(node2);

					// Assertion for one condition of reduced MTBDDs
					assert(low2Tree != high2Tree);

					lowOutTree = recDescend(node1, low2Tree);
					highOutTree = recDescend(node1, high2Tree);
					break;
				}

			// this should never happen
			default: {
					assert(false);
					break;
				}
		}

		if (lowOutTree == highOutTree)
		{	// in case both trees are isomorphic (when caching is enabled)
			ht.insert(std::make_pair(cacheAddress, lowOutTree));
			return lowOutTree;
		}
		else
		{	// in case both trees are distinct
			NodeOutType* result = createInternal(lowOutTree, highOutTree, var);

			ht.insert(std::make_pair(cacheAddress, result));
			return result;
		}
	}

public:   // Public methods

	AbstractApply2Functor()
		: mtbdd1_(static_cast<MTBDD1Type*>(0)),
			mtbdd2_(static_cast<MTBDD2Type*>(0)),
			ht()
	{ }

	MTBDDOutType operator()(const MTBDD1Type& mtbdd1, const MTBDD2Type& mtbdd2)
	{
		// store the MTBDDs
		mtbdd1_ = &mtbdd1;
		mtbdd2_ = &mtbdd2;

		// clear the cache
		ht.clear();

		if (mtbdd1_->GetVarOrdering() != mtbdd2_->GetVarOrdering())
		{	// in case the MTBDDs have a different variable ordering
			assert(false);
			// TODO
		}

		// recursively descend the MTBDD and generate a new one
		typename MTBDDOutType::NodeType* root = recDescend(mtbdd1_->getRoot(),
			mtbdd2_->getRoot());

		// compute the new default value
		DataOutType defaultValue = ApplyOperation(mtbdd1_->GetDefaultValue(),
			mtbdd2_->GetDefaultValue());

		// wrap it all up
		return MTBDDOutType(root, defaultValue, mtbdd1_->GetVarOrdering());
	}

	virtual DataOutType ApplyOperation(
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
