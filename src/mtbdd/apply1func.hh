/*****************************************************************************
 *  VATA Tree Automata Library
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

// Standard library headers
#include  <unordered_set>

// Boost library headers
#include <boost/functional/hash.hpp>

#include	"ondriks_mtbdd.hh"

namespace VATA
{
	namespace MTBDDPkg
	{
		template <
			class Base,
			typename Data1,
			typename DataOut
		>
		class Apply1Functor;
	}
}


template <
	class Base,
	typename Data1,
	typename DataOut
>
class VATA::MTBDDPkg::Apply1Functor
{
public:   // Public data types

	typedef Base BaseClass;

	typedef Data1 Data1Type;
	typedef DataOut DataOutType;

	typedef OndriksMTBDD<Data1Type> MTBDD1Type;
	typedef OndriksMTBDD<DataOutType> MTBDDOutType;

	typedef typename MTBDD1Type::NodePtrType Node1PtrType;
	typedef typename MTBDDOutType::NodePtrType NodeOutPtrType;

	typedef typename MTBDDOutType::VarType VarType;

private:  // Private data types

	typedef Node1PtrType CacheAddressType;

	// TODO: add some description of the use of boost::hash
	//   from http://www.boost.org/doc/libs/1_34_0/doc/html/hash/custom.html
	//
	//boost::hash is implemented by calling the function hash_value. The
	//namespace isn't specified so that it can detect overloads via argument
	//dependant lookup. So if there is a free function hash_value in the same
	//namespace as a custom type, it will get called. 

	typedef std::unordered_map<CacheAddressType, NodeOutPtrType,
		boost::hash<CacheAddressType>> CacheHashTable;

private:  // Private data members

	const MTBDD1Type* mtbdd1_;

	CacheHashTable ht;


private:  // Private methods

	Apply1Functor(const Apply1Functor&);
	Apply1Functor& operator=(const Apply1Functor&);


	NodeOutPtrType recDescend(const Node1PtrType& node1)
	{
		// Assertions
		assert(!IsNull(node1));

		if (IsLeaf(node1))
		{	// for the terminal case
			CacheAddressType cacheAddress(node1);
			typename CacheHashTable::iterator itHt;
			if ((itHt = ht.find(cacheAddress)) != ht.end())
			{	// if the result is already known
				assert(!IsNull(itHt->second));
				return itHt->second;
			}
			else
			{	// if the result isn't known
				NodeOutPtrType result = MTBDDOutType::spawnLeaf(
					makeBase().ApplyOperation(GetDataFromLeaf(node1)));

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

			Node1PtrType low1Tree = GetLowFromInternal(node1);
			Node1PtrType high1Tree = GetHighFromInternal(node1);

			// Assertions for one condition of reduced MTBDDs
			assert(low1Tree != high1Tree);

			VarType var = GetVarFromInternal(node1);
			NodeOutPtrType lowOutTree = recDescend(low1Tree);
			NodeOutPtrType highOutTree = recDescend(high1Tree);

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

	inline BaseClass& makeBase()
	{
		return static_cast<BaseClass&>(*this);
	}

public:   // Public methods

	Apply1Functor()
		: mtbdd1_(nullptr),
			ht()
	{ }

	MTBDDOutType operator()(const MTBDD1Type& mtbdd1)
	{
		// store the MTBDD
		mtbdd1_ = &mtbdd1;

		// clear the cache
		ht.clear();

		// recursively descend the MTBDD and generate a new one
		NodeOutPtrType root = recDescend(mtbdd1_->getRoot());
		IncrementRefCnt(root);

		// compute the new default value
		DataOutType defaultValue = makeBase().ApplyOperation(
			mtbdd1_->GetDefaultValue());

		// wrap it all up
		return MTBDDOutType(root, defaultValue);
	}

protected:// Protected methods

	inline const MTBDD1Type& getMTBDD1() const
	{
		assert(mtbdd1_ != nullptr);
		return *mtbdd1_;
	}
};
#endif
