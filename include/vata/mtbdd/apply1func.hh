/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Unary Apply functor for OndriksMTBDD
 *
 *****************************************************************************/

#ifndef _VATA_APPLY1FUNC_HH_
#define _VATA_APPLY1FUNC_HH_

// VATA headers
#include	<vata/vata.hh>
#include	<vata/mtbdd/ondriks_mtbdd.hh>

// Standard library headers
#include  <tr1/unordered_set>

// Boost library headers
#include <boost/functional/hash.hpp>


namespace VATA
{
	namespace MTBDDPkg
	{
		template <
			typename Data1,
			typename DataOut
		>
		class AbstractApply1Functor;
	}
}


template <
	typename Data1,
	typename DataOut
>
class VATA::MTBDDPkg::AbstractApply1Functor
{
public:   // Public data types

	typedef Data1 Data1Type;
	typedef DataOut DataOutType;

	typedef OndriksMTBDD<Data1Type> MTBDD1Type;
	typedef OndriksMTBDD<DataOutType> MTBDDOutType;

	typedef typename MTBDD1Type::NodeType Node1Type;
	typedef typename MTBDDOutType::NodeType NodeOutType;

	typedef typename MTBDDOutType::VarType VarType;

private:  // Private data types

	typedef const Node1Type* CacheAddressType;

	/**
	 * @brief  Hasher structure for a single key
	 *
	 * This class is a hasher for a single key.
	 */
	struct Hasher1
	{
		inline size_t operator()(const CacheAddressType& key) const
		{
			return boost::hash_value(key);
		}
	};

	typedef std::tr1::unordered_map<CacheAddressType, NodeOutType*, Hasher1>
		CacheHashTable;

private:  // Private data members

	const MTBDD1Type* mtbdd1_;

	CacheHashTable ht;


private:  // Private methods

	AbstractApply1Functor(const AbstractApply1Functor&);
	AbstractApply1Functor& operator=(const AbstractApply1Functor&);


	NodeOutType* recDescend(const Node1Type* node1)
	{
		// Assertions
		assert(node1 != static_cast<Node1Type*>(0));

		if (IsLeaf(node1))
		{	// for the terminal case
			CacheAddressType cacheAddress(node1);
			typename CacheHashTable::iterator itHt;
			if ((itHt = ht.find(cacheAddress)) != ht.end())
			{	// if the result is already known
				assert(itHt->second != static_cast<NodeOutType*>(0));
				return itHt->second;
			}
			else
			{	// if the result isn't known
				NodeOutType* result = MTBDDOutType::spawnLeaf(
					ApplyOperation(GetDataFromLeaf(node1)));

				// cache
				ht.insert(std::make_pair(cacheAddress, result));
				return result;
			}
		}
		else
		{	// for internal nodes
			// NB: we don't cache internal nodes, because it is useless as unary
			// apply only performs a DFS on the graph. Given that caching is used
			// for leaves, the condition for reduced MTBDDs is bound to hold.

			assert(IsInternal(node1));

			const Node1Type* low1Tree = GetLowFromInternal(node1);
			const Node1Type* high1Tree = GetHighFromInternal(node1);

			// Assertions for one condition of reduced MTBDDs
			assert(low1Tree != high1Tree);

			VarType var = GetVarFromInternal(node1);
			NodeOutType* lowOutTree = recDescend(low1Tree);
			NodeOutType* highOutTree = recDescend(high1Tree);

			if (lowOutTree == highOutTree)
			{	// in case both trees are isomorphic (when caching is enabled)
				return lowOutTree;
			}
			else
			{	// in case both trees are distinct
				return MTBDDOutType::spawnInternal(lowOutTree, highOutTree, var);
			}
		}
	}

public:   // Public methods

	AbstractApply1Functor()
		: mtbdd1_(static_cast<MTBDD1Type*>(0)),
			ht()
	{ }

	MTBDDOutType operator()(const MTBDD1Type& mtbdd1)
	{
		// store the MTBDD
		mtbdd1_ = &mtbdd1;

		// clear the cache
		ht.clear();

		// recursively descend the MTBDD and generate a new one
		NodeOutType* root = recDescend(mtbdd1_->getRoot());
		RecursivelyIncrementRefCnt(root);

		// compute the new default value
		DataOutType defaultValue = ApplyOperation(mtbdd1_->GetDefaultValue());

		// wrap it all up
		return MTBDDOutType(root, defaultValue, mtbdd1_->GetVarOrdering());
	}

	virtual DataOutType ApplyOperation(const Data1Type& data1) = 0;

protected:// Protected methods

	inline const MTBDD1Type& getMTBDD1() const
	{
		assert(mtbdd1_ != static_cast<MTBDD1Type*>(0));
		return *mtbdd1_;
	}


	virtual ~AbstractApply1Functor()
	{ }
};
#endif
