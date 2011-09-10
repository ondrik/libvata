/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a BDD transition function.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TRANS_TABLE_HH_
#define _VATA_BDD_TRANS_TABLE_HH_

// VATA headers
#include <vata/vata.hh>

// Standard library headers
#include <unordered_map>


namespace VATA
{
	namespace Util
	{
		template <
			typename State,
			typename Leaf
		>
		class BDDTransTable;
	}
}


template <
	typename State,
	typename Leaf
>
class VATA::Util::BDDTransTable
{
public:   // data types

	typedef State StateType;

	typedef size_t MTBDDHandle;

	typedef size_t RefCntType;

private:  // data types

	typedef Leaf LeafType;
	typedef VATA::MTBDDPkg::OndriksMTBDD<LeafType> MTBDD;


	struct MTBDDCell
	{
	private:  // data members

		MTBDD bdd_;
		RefCntType refcnt_;

	public:   // methods

		explicit MTBDDCell(MTBDD bdd)
			: bdd_(bdd),
				refcnt_(1)
		{ }

		inline void IncrementRefCnt()
		{
			// Assertions
			assert(refcnt_ > 0);

			++refcnt_;
		}

		inline const RefCntType& DecrementRefCnt()
		{
			// Assertions
			assert(refcnt_ > 0);

			return --refcnt_;
		}

		inline const MTBDD& GetMTBDD() const
		{
			return bdd_;
		}

		inline void SetMTBDD(const MTBDD& bdd)
		{
			bdd_ = bdd;
		}

		inline const RefCntType& GetRefCnt() const
		{
			return refcnt_;
		}
	};

	typedef std::unordered_map<MTBDDHandle, MTBDDCell> MTBDDHashTable;

private:  // data members

	MTBDDHandle nextHandleCnt_;
	StateType nextStateCnt_;

	MTBDDHashTable mtbddCellMap_;

public:   // methods

	BDDTransTable()
		: nextHandleCnt_(1),
			nextStateCnt_(1),
			mtbddCellMap_()
	{ }

	inline StateType AddState()
	{
		// Assertions
		assert(nextStateCnt_ != 0);

		return nextStateCnt_++;
	}

	inline MTBDDHandle AddHandle()
	{
		// Assertions
		assert(nextHandleCnt_ != 0);

		mtbddCellMap_.insert(std::make_pair(nextHandleCnt_, MTBDD(Leaf())));

		return nextHandleCnt_++;
	}

	inline const MTBDD& GetMtbdd(const MTBDDHandle& handle) const
	{
		typename MTBDDHashTable::const_iterator itHt;
		if ((itHt = mtbddCellMap_.find(handle)) == mtbddCellMap_.end())
		{	// in case we are trying to access some nonsense
			assert(false);      // fail gracefully
		}

		return (itHt->second).GetMTBDD();
	}

	inline void SetMtbdd(const MTBDDHandle& handle, const MTBDD& bdd)
	{
		typename MTBDDHashTable::iterator itHt;
		if ((itHt = mtbddCellMap_.find(handle)) == mtbddCellMap_.end())
		{	// in case we are trying to access some nonsense
			assert(false);      // fail gracefully
		}

		(itHt->second).SetMTBDD(bdd);
	}

	inline void IncrementHandleRefCnt(const MTBDDHandle& handle)
	{
		typename MTBDDHashTable::iterator itHt;
		if ((itHt = mtbddCellMap_.find(handle)) == mtbddCellMap_.end())
		{	// in case the state is not in the hash table
			assert(false);    // fail gracefully
		}

		(itHt->second).IncrementRefCnt();
	}

	inline void DecrementHandleRefCnt(const MTBDDHandle& handle)
	{
		typename MTBDDHashTable::iterator itHt;
		if ((itHt = mtbddCellMap_.find(handle)) == mtbddCellMap_.end())
		{	// in case the state is not in the hash table
			assert(false);    // fail gracefully
		}

		if ((itHt->second).DecrementRefCnt() == 0)
		{	// in case no one uses the state
			mtbddCellMap_.erase(itHt);
		}
	}

	inline const RefCntType& GetHandleRefCnt(const MTBDDHandle& handle) const
	{
		typename MTBDDHashTable::const_iterator itHt;
		if ((itHt = mtbddCellMap_.find(handle)) == mtbddCellMap_.end())
		{	// in case the state is not in the hash table
			assert(false);    // fail gracefully
		}

		return (itHt->second).GetRefCnt();
	}

	~BDDTransTable()
	{
		// Assertions
		assert(mtbddCellMap_.empty());
	}
};

#endif
