/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Unary void Apply functor for OndriksMTBDD
 *
 *****************************************************************************/

#ifndef _VATA_VOID_APPLY1FUNC_HH_
#define _VATA_VOID_APPLY1FUNC_HH_

// VATA headers
#include <vata/vata.hh>

#include "ondriks_mtbdd.hh"


namespace VATA
{
	namespace MTBDDPkg
	{
		template <
			class Base,
			typename Data1
		>
		class VoidApply1Functor;
	}
}

template <
	class Base,
	typename Data1
>
class VATA::MTBDDPkg::VoidApply1Functor
{
public:   // Public data types

	typedef Base BaseClass;

	typedef Data1 Data1Type;

	typedef OndriksMTBDD<Data1Type> MTBDD1Type;

	typedef typename MTBDD1Type::NodePtrType Node1PtrType;

private:  // Private data types

	typedef Node1PtrType CacheAddressType;

	// TODO: add some description of the use of boost::hash
	//   from http://www.boost.org/doc/libs/1_34_0/doc/html/hash/custom.html
	//
	//boost::hash is implemented by calling the function hash_value. The
	//namespace isn't specified so that it can detect overloads via argument
	//dependant lookup. So if there is a free function hash_value in the same
	//namespace as a custom type, it will get called. 

	typedef std::unordered_set<CacheAddressType,
		boost::hash<CacheAddressType>> CacheHashTable;

private:  // Private data members

	const MTBDD1Type* mtbdd1_;

	CacheHashTable ht;


private:  // Private methods

	VoidApply1Functor(const VoidApply1Functor&);
	VoidApply1Functor& operator=(const VoidApply1Functor&);


	void recDescend(const Node1PtrType& node1)
	{
		// Assertions
		assert(!IsNull(node1));

		typename CacheHashTable::iterator itHt;
		if ((itHt = ht.find(node1)) != ht.end())
		{	// if the result is already known
			return;
		}

		if (IsLeaf(node1))
		{	// for the terminal case
			makeBase().ApplyOperation(GetDataFromLeaf(node1));
		}
		else
		{	// for internal nodes
			assert(IsInternal(node1));

			Node1PtrType low1Tree = GetLowFromInternal(node1);
			Node1PtrType high1Tree = GetHighFromInternal(node1);

			// Assertions for one condition of reduced MTBDDs
			assert(low1Tree != high1Tree);

			recDescend(low1Tree);
			recDescend(high1Tree);
		}

		// cache
		ht.insert(node1);
	}

	inline BaseClass& makeBase()
	{
		return static_cast<BaseClass&>(*this);
	}

public:   // Public methods

	VoidApply1Functor()
		: mtbdd1_(nullptr),
			ht()
	{ }

	void operator()(const MTBDD1Type& mtbdd1)
	{
		// store the MTBDD
		mtbdd1_ = &mtbdd1;

		// clear the cache
		ht.clear();

		// recursively descend the MTBDD and generate a new one
		recDescend(mtbdd1_->getRoot());
	}

protected:// Protected methods

	inline const MTBDD1Type& getMTBDD1() const
	{
		assert(mtbdd1_ != nullptr);
		return *mtbdd1_;
	}
};
#endif
