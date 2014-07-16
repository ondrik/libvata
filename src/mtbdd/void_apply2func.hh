/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Binary void Apply functor for OndriksMTBDD
 *
 *****************************************************************************/

#ifndef _VATA_VOID_APPLY2FUNC_HH_
#define _VATA_VOID_APPLY2FUNC_HH_

// VATA headers
#include	<vata/vata.hh>

// Standard library headers
#include  <unordered_set>

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
			typename Data2
		>
		class VoidApply2Functor;
	}
}


template <
	class Base,
	typename Data1,
	typename Data2
>
class VATA::MTBDDPkg::VoidApply2Functor
{
public:   // Public data types

	typedef Base BaseClass;

	typedef Data1 Data1Type;
	typedef Data2 Data2Type;

	typedef OndriksMTBDD<Data1Type> MTBDD1Type;
	typedef OndriksMTBDD<Data2Type> MTBDD2Type;

	typedef typename MTBDD1Type::NodePtrType Node1PtrType;
	typedef typename MTBDD2Type::NodePtrType Node2PtrType;

private:  // Private data types

	typedef std::pair<Node1PtrType, Node2PtrType> CacheAddressType;

	typedef std::unordered_set<CacheAddressType,
		boost::hash<CacheAddressType>> CacheHashTable;

private:  // Private data members

	const MTBDD1Type* mtbdd1_;
	const MTBDD2Type* mtbdd2_;

	CacheHashTable ht;

	bool processingStopped_;

	static const char NODE1MASK = 0x01;  // 00000001
	static const char NODE2MASK = 0x02;  // 00000010

private:  // Private methods

	VoidApply2Functor(const VoidApply2Functor&);
	VoidApply2Functor& operator=(const VoidApply2Functor&);

	void recDescend(const Node1PtrType& node1, const Node2PtrType& node2)
	{
		// Assertions
		assert(!IsNull(node1));
		assert(!IsNull(node2));

		if (processingStopped_)
		{
			return;
		}

		CacheAddressType cacheAddress(node1, node2);
		typename CacheHashTable::iterator itHt;
		if ((itHt = ht.find(cacheAddress)) != ht.end())
		{	// if the result is already known
			return;
		}

		char relation = classifyCase2(node1, node2);
		assert((relation & ~(NODE1MASK | NODE2MASK)) == 0x00);

		if (!relation)
		{	// for the terminal case
			makeBase().ApplyOperation(GetDataFromLeaf(node1), GetDataFromLeaf(node2));

			ht.insert(cacheAddress);
			return;
		}

		// in case there is something to be branched
		assert(relation);

		Node1PtrType low1Tree = 0;
		Node2PtrType low2Tree = 0;
		Node1PtrType high1Tree = 0;
		Node2PtrType high2Tree = 0;

		if (relation & NODE1MASK)
		{	// if node1 is to be branched
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
			low2Tree = GetLowFromInternal(node2);
			high2Tree = GetHighFromInternal(node2);
			assert(low2Tree != high2Tree);
		}
		else
		{	// if node2 is not to be branched
			low2Tree = node2;
			high2Tree = node2;
		}

		recDescend(low1Tree, low2Tree);
		recDescend(high1Tree, high2Tree);

		ht.insert(cacheAddress);
	}

	inline BaseClass& makeBase()
	{
		return static_cast<BaseClass&>(*this);
	}

public:   // Public methods

	VoidApply2Functor() :
		mtbdd1_(nullptr),
		mtbdd2_(nullptr),
		ht(),
		processingStopped_(false)
	{ }

	void operator()(const MTBDD1Type& mtbdd1, const MTBDD2Type& mtbdd2)
	{
		// store the MTBDDs
		mtbdd1_ = &mtbdd1;
		mtbdd2_ = &mtbdd2;

		// clear the cache
		ht.clear();

		// re-enable processing
		processingStopped_ = false;

		// recursively descend the MTBDD
		recDescend(mtbdd1_->getRoot(), mtbdd2_->getRoot());
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

	inline void stopProcessing()
	{
		processingStopped_ = true;
	}
};

#endif
