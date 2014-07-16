/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Ternary Apply functor for OndriksMTBDD
 *
 *****************************************************************************/

#ifndef _VATA_APPLY3FUNC_HH_
#define _VATA_APPLY3FUNC_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/triple.hh>

// Standard library headers
#include <unordered_set>

// Boost library headers
#include <boost/functional/hash.hpp>

#include "ondriks_mtbdd.hh"

namespace VATA
{
	namespace MTBDDPkg
	{
		template <
			class Base,
			typename Data1,
			typename Data2,
			typename Data3,
			typename DataOut
		>
		class Apply3Functor;
	}
}


template <
	class Base,
	typename Data1,
	typename Data2,
	typename Data3,
	typename DataOut
>
class VATA::MTBDDPkg::Apply3Functor
{
public:   // Public data types

	typedef Base BaseClass;

	typedef Data1 Data1Type;
	typedef Data2 Data2Type;
	typedef Data3 Data3Type;
	typedef DataOut DataOutType;

	typedef OndriksMTBDD<Data1Type> MTBDD1Type;
	typedef OndriksMTBDD<Data2Type> MTBDD2Type;
	typedef OndriksMTBDD<Data3Type> MTBDD3Type;
	typedef OndriksMTBDD<DataOutType> MTBDDOutType;

	typedef typename MTBDD1Type::NodePtrType Node1PtrType;
	typedef typename MTBDD2Type::NodePtrType Node2PtrType;
	typedef typename MTBDD3Type::NodePtrType Node3PtrType;
	typedef typename MTBDDOutType::NodePtrType NodeOutPtrType;

	typedef typename MTBDDOutType::VarType VarType;

private:  // Private data types


	typedef VATA::Util::Triple<Node1PtrType, Node2PtrType, Node3PtrType>
		CacheAddressType;

	typedef std::unordered_map<CacheAddressType, NodeOutPtrType,
		boost::hash<CacheAddressType>> CacheHashTable;


private:  // Private data members

	const MTBDD1Type* mtbdd1_;
	const MTBDD2Type* mtbdd2_;
	const MTBDD3Type* mtbdd3_;

	CacheHashTable ht;

	static const char NODE1MASK = 0x01;  // 00000001
	static const char NODE2MASK = 0x02;  // 00000010
	static const char NODE3MASK = 0x04;  // 00000100


private:  // Private methods

	Apply3Functor(const Apply3Functor&);
	Apply3Functor& operator=(const Apply3Functor&);


	inline static char classifyCase(Node1PtrType node1,
		Node2PtrType node2, Node3PtrType node3)
	{
		// Assertions
		assert(!IsNull(node1));
		assert(!IsNull(node2));
		assert(!IsNull(node3));

		char result = 0x00;

		if (IsInternal(node1))
		{	// node1 is internal
			if ((IsLeaf(node2) ||
				(GetVarFromInternal(node1) >= GetVarFromInternal(node2))) &&
				(IsLeaf(node3) ||
				(GetVarFromInternal(node1) >= GetVarFromInternal(node3))))
			{
				result |= NODE1MASK;	// branch node1
			}
		}

		if (IsInternal(node2))
		{	// node2 is internal
			if ((IsLeaf(node1) ||
				(GetVarFromInternal(node2) >= GetVarFromInternal(node1))) &&
				(IsLeaf(node3) ||
				(GetVarFromInternal(node2) >= GetVarFromInternal(node3))))
			{
				result |= NODE2MASK;	// branch node2
			}
		}

		if (IsInternal(node3))
		{	// node3 is internal
			if ((IsLeaf(node1) ||
				(GetVarFromInternal(node3) >= GetVarFromInternal(node1))) &&
				(IsLeaf(node2) ||
				(GetVarFromInternal(node3) >= GetVarFromInternal(node2))))
			{
				result |= NODE3MASK;	// branch node3
			}
		}

		return result;
	}

	NodeOutPtrType recDescend(Node1PtrType node1, Node2PtrType node2,
		Node3PtrType node3)
	{
		// Assertions
		assert(!IsNull(node1));
		assert(!IsNull(node2));
		assert(!IsNull(node3));

		CacheAddressType cacheAddress(node1, node2, node3);
		typename CacheHashTable::iterator itHt;
		if ((itHt = ht.find(cacheAddress)) != ht.end())
		{	// if the result is already known
			assert(!IsNull(itHt->second));
			return itHt->second;
		}

		char relation = classifyCase(node1, node2, node3);
		assert((relation & ~(NODE1MASK | NODE2MASK | NODE3MASK)) == 0x00);

		if (!relation)
		{	// for the terminal case
			NodeOutPtrType result = MTBDDOutType::spawnLeaf(makeBase().ApplyOperation(
				GetDataFromLeaf(node1), GetDataFromLeaf(node2), GetDataFromLeaf(node3)));

			ht.insert(std::make_pair(cacheAddress, result));
			return result;
		}

		// in case there is something to be branched
		assert(relation);

		VarType var;
		Node1PtrType low1Tree = 0;
		Node2PtrType low2Tree = 0;
		Node3PtrType low3Tree = 0;
		Node1PtrType high1Tree = 0;
		Node2PtrType high2Tree = 0;
		Node3PtrType high3Tree = 0;

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

		if (relation & NODE3MASK)
		{	// if node3 is to be branched
			var = GetVarFromInternal(node3);
			low3Tree = GetLowFromInternal(node3);
			high3Tree = GetHighFromInternal(node3);
			assert(low3Tree != high3Tree);
		}
		else
		{	// if node3 is not to be branched
			low3Tree = node3;
			high3Tree = node3;
		}

		NodeOutPtrType lowOutTree = recDescend(low1Tree, low2Tree, low3Tree);
		NodeOutPtrType highOutTree = recDescend(high1Tree, high2Tree, high3Tree);

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

	Apply3Functor() :
		mtbdd1_(nullptr),
		mtbdd2_(nullptr),
		mtbdd3_(nullptr),
		ht()
	{ }

	MTBDDOutType operator()(const MTBDD1Type& mtbdd1,
		const MTBDD2Type& mtbdd2, const MTBDD3Type& mtbdd3)
	{
		// store the MTBDDs
		mtbdd1_ = &mtbdd1;
		mtbdd2_ = &mtbdd2;
		mtbdd3_ = &mtbdd3;

		// clear the cache
		ht.clear();

		// recursively descend the MTBDD and generate a new one
		NodeOutPtrType root = recDescend(mtbdd1_->getRoot(), mtbdd2_->getRoot(),
			mtbdd3_->getRoot());
		IncrementRefCnt(root);

		// compute the new default value
		DataOutType defaultValue = makeBase().ApplyOperation(mtbdd1_->GetDefaultValue(),
			mtbdd2_->GetDefaultValue(), mtbdd3_->GetDefaultValue());

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

	inline const MTBDD3Type& getMTBDD3() const
	{
		assert(mtbdd3_ != nullptr);
		return *mtbdd3_;
	}
};

// TODO:
//template <
//	typename T1,
//	typename T2,
//	typename T3,
//	typename TOut
//>
//inline static bool operator==(
//	const typename VATA::MTBDDPkg::AbstractApply3Functor<T1, T2, T3, TOut>::triple& lhs,
//	const typename VATA::MTBDDPkg::AbstractApply3Functor<T1, T2, T3, TOut>::triple& rhs)
//{
//	return ((lhs.first == rhs.first) && (lhs.second == rhs.second) &&
//		(lhs.third == rhs.third));
//}

#endif
