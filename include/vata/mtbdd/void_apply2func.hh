/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
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
			typename Data2
		>
		class AbstractVoidApply2Functor;
	}
}


template <
	typename Data1,
	typename Data2
>
class VATA::MTBDDPkg::AbstractVoidApply2Functor
{
public:   // Public data types

	typedef Data1 Data1Type;
	typedef Data2 Data2Type;

	typedef OndriksMTBDD<Data1Type> MTBDD1Type;
	typedef OndriksMTBDD<Data2Type> MTBDD2Type;

	typedef typename MTBDD1Type::NodePtrType Node1PtrType;
	typedef typename MTBDD2Type::NodePtrType Node2PtrType;

private:  // Private data types

	typedef std::pair<Node1PtrType, Node2PtrType> CacheAddressType;

	typedef std::tr1::unordered_set<CacheAddressType,
		boost::hash<CacheAddressType> > CacheHashTable;

private:  // Private data members

	const MTBDD1Type* mtbdd1_;
	const MTBDD2Type* mtbdd2_;

	CacheHashTable ht;

	static const char NODE1MASK = 0x01;  // 00000001
	static const char NODE2MASK = 0x02;  // 00000010

private:  // Private methods

	AbstractVoidApply2Functor(const AbstractVoidApply2Functor&);
	AbstractVoidApply2Functor& operator=(const AbstractVoidApply2Functor&);

	void recDescend(const Node1PtrType& node1, const Node2PtrType& node2)
	{
		// Assertions
		assert(!IsNull(node1));
		assert(!IsNull(node2));

		CacheAddressType cacheAddress(node1, node2);
		typename CacheHashTable::iterator itHt;
		if ((itHt = ht.find(cacheAddress)) != ht.end())
		{	// if the result is already known
			assert(!IsNull(itHt->second));
			return;
		}

		char relation = classifyCase2(node1, node2);
		assert((relation & ~(NODE1MASK | NODE2MASK)) == 0x00);

		if (!relation)
		{	// for the terminal case
			ApplyOperation(GetDataFromLeaf(node1), GetDataFromLeaf(node2));

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

public:   // Public methods

	AbstractVoidApply2Functor() :
		mtbdd1_(static_cast<MTBDD1Type*>(0)),
		mtbdd2_(static_cast<MTBDD2Type*>(0)),
		ht()
	{ }

	void operator()(const MTBDD1Type& mtbdd1, const MTBDD2Type& mtbdd2)
	{
		// store the MTBDDs
		mtbdd1_ = &mtbdd1;
		mtbdd2_ = &mtbdd2;

		// clear the cache
		ht.clear();

		// recursively descend the MTBDD
		recDescend(mtbdd1_->getRoot(), mtbdd2_->getRoot());
	}

	virtual void ApplyOperation(
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


	virtual ~AbstractVoidApply2Functor()
	{ }
};

#endif
