/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Binary Apply functor for OndriksMTBDD
 *
 *****************************************************************************/

#ifndef _VATA_APPLY2FUNC_HH_
#define _VATA_APPLY2FUNC_HH_

// VATA headers
#include	<vata/vata.hh>

// Standard library headers
#include  <unordered_set>

// Boost library headers
#include <boost/functional/hash.hpp>

#include "ondriks_mtbdd.hh"
#include "classify_case.hh"

namespace VATA
{
	namespace MTBDDPkg
	{
		template <
			class Base,
			typename Data1,
			typename Data2,
			typename DataOut
		>
		class Apply2Functor;
	}
}


template <
	class Base,
	typename Data1,
	typename Data2,
	typename DataOut
>
class VATA::MTBDDPkg::Apply2Functor
{
public:   // Public data types

	typedef Base BaseClass;

	typedef Data1 Data1Type;
	typedef Data2 Data2Type;
	typedef DataOut DataOutType;

	typedef OndriksMTBDD<Data1Type> MTBDD1Type;
	typedef OndriksMTBDD<Data2Type> MTBDD2Type;
	typedef OndriksMTBDD<DataOutType> MTBDDOutType;

	typedef typename MTBDD1Type::NodePtrType Node1PtrType;
	typedef typename MTBDD2Type::NodePtrType Node2PtrType;
	typedef typename MTBDDOutType::NodePtrType NodeOutPtrType;

	typedef typename MTBDDOutType::VarType VarType;

private:  // Private data types

	typedef std::pair<Node1PtrType, Node2PtrType> CacheAddressType;

	typedef std::unordered_map<CacheAddressType, NodeOutPtrType,
		boost::hash<CacheAddressType>> CacheHashTable;

private:  // Private data members

	const MTBDD1Type* mtbdd1_;
	const MTBDD2Type* mtbdd2_;

	CacheHashTable ht;

private:  // Private methods

	Apply2Functor(const Apply2Functor&);
	Apply2Functor& operator=(const Apply2Functor&);

	NodeOutPtrType recDescend(const Node1PtrType& node1, const Node2PtrType& node2)
	{
		// Assertions
		assert(!IsNull(node1));
		assert(!IsNull(node2));

		CacheAddressType cacheAddress(node1, node2);
		typename CacheHashTable::iterator itHt;
		if ((itHt = ht.find(cacheAddress)) != ht.end())
		{	// if the result is already known
			assert(!IsNull(itHt->second));
			return itHt->second;
		}

		char relation = classifyCase2(node1, node2);
		assert((relation & ~(NODE1MASK | NODE2MASK)) == 0x00);

		if (!relation)
		{	// for the terminal case
			NodeOutPtrType result = MTBDDOutType::spawnLeaf(makeBase().ApplyOperation(
				GetDataFromLeaf(node1), GetDataFromLeaf(node2)));

			ht.insert(std::make_pair(cacheAddress, result));
			return result;
		}

		// in case there is something to be branched
		assert(relation);

		VarType var;
		Node1PtrType low1Tree = 0;
		Node2PtrType low2Tree = 0;
		Node1PtrType high1Tree = 0;
		Node2PtrType high2Tree = 0;

		if (relation & NODE1MASK)
		{	// if node1 is to be branched
			var = GetVarFromInternal(node1);
			low1Tree = GetLowFromInternal(node1);
			high1Tree = GetHighFromInternal(node1);
			assert(low1Tree != high1Tree);
		}
		else
		{	// if node1 is not to be branched
			low1Tree = node1;
			high1Tree = node1;
		}

		if (relation & NODE2MASK)
		{	// if node2 is to be branched
			var = GetVarFromInternal(node2);
			low2Tree = GetLowFromInternal(node2);
			high2Tree = GetHighFromInternal(node2);
			assert(low2Tree != high2Tree);
		}
		else
		{	// if node2 is not to be branched
			low2Tree = node2;
			high2Tree = node2;
		}

		NodeOutPtrType lowOutTree = recDescend(low1Tree, low2Tree);
		NodeOutPtrType highOutTree = recDescend(high1Tree, high2Tree);

		if (lowOutTree == highOutTree)
		{	// in case both trees are isomorphic (when caching is enabled)
			ht.insert(std::make_pair(cacheAddress, lowOutTree));
			return lowOutTree;
		}
		else
		{	// in case both trees are distinct
			NodeOutPtrType result =
				MTBDDOutType::spawnInternal(lowOutTree, highOutTree, var);

			ht.insert(std::make_pair(cacheAddress, result));
			return result;
		}
	}

	inline BaseClass& makeBase()
	{
		return static_cast<BaseClass&>(*this);
	}

public:   // Public methods

	Apply2Functor() :
		mtbdd1_(nullptr),
		mtbdd2_(nullptr),
		ht()
	{ }

	MTBDDOutType operator()(const MTBDD1Type& mtbdd1, const MTBDD2Type& mtbdd2)
	{
		// store the MTBDDs
		mtbdd1_ = &mtbdd1;
		mtbdd2_ = &mtbdd2;

		// clear the cache
		ht.clear();

		// recursively descend the MTBDD and generate a new one
		NodeOutPtrType root = recDescend(mtbdd1_->getRoot(), mtbdd2_->getRoot());
		IncrementRefCnt(root);

		// compute the new default value
		DataOutType defaultValue = makeBase().ApplyOperation(
			mtbdd1_->GetDefaultValue(), mtbdd2_->GetDefaultValue());

		// wrap it all up
		return MTBDDOutType(root, defaultValue);
	}

protected:// Protected methods

	inline const MTBDD1Type& getMTBDD1() const
	{
		assert(mtbdd1_ != nullptr);
		return *mtbdd1_;
	}

	inline const MTBDD2Type& getMTBDD2() const
	{
		assert(mtbdd2_ != nullptr);
		return *mtbdd2_;
	}
};

#endif
