/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
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
#include	<vata/vata.hh>
#include	<vata/mtbdd/ondriks_mtbdd.hh>
#include	<vata/util/triple.hh>

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
			typename Data2,
			typename Data3,
			typename DataOut
		>
		class AbstractApply3Functor;
	}
}


template <
	typename Data1,
	typename Data2,
	typename Data3,
	typename DataOut
>
class VATA::MTBDDPkg::AbstractApply3Functor
{
public:   // Public data types

	typedef Data1 Data1Type;
	typedef Data2 Data2Type;
	typedef Data3 Data3Type;
	typedef DataOut DataOutType;

	typedef OndriksMTBDD<Data1Type> MTBDD1Type;
	typedef OndriksMTBDD<Data2Type> MTBDD2Type;
	typedef OndriksMTBDD<Data3Type> MTBDD3Type;
	typedef OndriksMTBDD<DataOutType> MTBDDOutType;

	typedef typename MTBDD1Type::NodeType Node1Type;
	typedef typename MTBDD2Type::NodeType Node2Type;
	typedef typename MTBDD3Type::NodeType Node3Type;
	typedef typename MTBDDOutType::NodeType NodeOutType;

	typedef typename MTBDDOutType::VarType VarType;

private:  // Private data types


	typedef VATA::Util::Triple<const Node1Type*, const Node2Type*, const Node3Type*>
		CacheAddressType;

	typedef std::tr1::unordered_map<CacheAddressType, NodeOutType*,
		typename CacheAddressType::Hasher> CacheHashTable;


private:  // Private data members

	const MTBDD1Type* mtbdd1_;
	const MTBDD2Type* mtbdd2_;
	const MTBDD3Type* mtbdd3_;

	CacheHashTable ht;

	static const char NODE1MASK = 0x01;  // 00000001
	static const char NODE2MASK = 0x02;  // 00000010
	static const char NODE3MASK = 0x04;  // 00000100


private:  // Private methods

	AbstractApply3Functor(const AbstractApply3Functor&);
	AbstractApply3Functor& operator=(const AbstractApply3Functor&);


	inline static char classifyCase(const Node1Type* node1,
		const Node2Type* node2, const Node3Type* node3)
	{
		// Assertions
		assert(node1 != static_cast<Node1Type*>(0));
		assert(node2 != static_cast<Node2Type*>(0));
		assert(node3 != static_cast<Node3Type*>(0));

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

	NodeOutType* recDescend(const Node1Type* node1, const Node2Type* node2,
		const Node3Type* node3)
	{
		// Assertions
		assert(node1 != static_cast<Node1Type*>(0));
		assert(node2 != static_cast<Node2Type*>(0));
		assert(node3 != static_cast<Node3Type*>(0));

		CacheAddressType cacheAddress(node1, node2, node3);
		typename CacheHashTable::iterator itHt;
		if ((itHt = ht.find(cacheAddress)) != ht.end())
		{	// if the result is already known
			assert(itHt->second != static_cast<NodeOutType*>(0));
			return itHt->second;
		}

		char relation = classifyCase(node1, node2, node3);
		assert((relation & ~(NODE1MASK | NODE2MASK | NODE3MASK)) == 0x00);

		if (!relation)
		{	// for the terminal case
			NodeOutType* result = CreateLeaf(ApplyOperation(
				GetDataFromLeaf(node1), GetDataFromLeaf(node2), GetDataFromLeaf(node3)));

			ht.insert(std::make_pair(cacheAddress, result));
			return result;
		}

		// in case there is something to be branched
		assert(relation);

		VarType var;
		const Node1Type* low1Tree = static_cast<Node1Type*>(0);
		const Node2Type* low2Tree = static_cast<Node2Type*>(0);
		const Node3Type* low3Tree = static_cast<Node3Type*>(0);
		const Node1Type* high1Tree = static_cast<Node1Type*>(0);
		const Node2Type* high2Tree = static_cast<Node2Type*>(0);
		const Node3Type* high3Tree = static_cast<Node3Type*>(0);

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

		NodeOutType* lowOutTree = recDescend(low1Tree, low2Tree, low3Tree);
		NodeOutType* highOutTree = recDescend(high1Tree, high2Tree, high3Tree);

		if (lowOutTree == highOutTree)
		{	// in case both trees are isomorphic (when caching is enabled)
			ht.insert(std::make_pair(cacheAddress, lowOutTree));
			return lowOutTree;
		}
		else
		{	// in case both trees are distinct
			NodeOutType* result = CreateInternal(lowOutTree, highOutTree, var);

			ht.insert(std::make_pair(cacheAddress, result));
			return result;
		}
	}


public:   // Public methods

	AbstractApply3Functor()
		: mtbdd1_(static_cast<MTBDD1Type*>(0)),
			mtbdd2_(static_cast<MTBDD2Type*>(0)),
			mtbdd3_(static_cast<MTBDD3Type*>(0)),
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

		if ((mtbdd1_->GetVarOrdering() != mtbdd2_->GetVarOrdering()) ||
			(mtbdd1_->GetVarOrdering() != mtbdd3_->GetVarOrdering()))
		{	// in case the MTBDDs have a different variable ordering
			assert(false);
			// TODO
		}

		// recursively descend the MTBDD and generate a new one
		NodeOutType* root = recDescend(mtbdd1_->getRoot(), mtbdd2_->getRoot(),
			mtbdd3_->getRoot());
		RecursivelyIncrementRefCnt(root);

		// compute the new default value
		DataOutType defaultValue = ApplyOperation(mtbdd1_->GetDefaultValue(),
			mtbdd2_->GetDefaultValue(), mtbdd3_->GetDefaultValue());

		// wrap it all up
		return MTBDDOutType(root, defaultValue, mtbdd1_->GetVarOrdering());
	}

	virtual DataOutType ApplyOperation(
		const Data1Type& data1, const Data2Type& data2, const Data3Type& data3) = 0;

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

	inline const MTBDD3Type& getMTBDD3() const
	{
		assert(mtbdd3_ != static_cast<MTBDD3Type*>(0));
		return *mtbdd3_;
	}


	virtual ~AbstractApply3Functor()
	{ }
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
