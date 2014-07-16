/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a bottom-up BDD transition table.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_BU_TRANS_TABLE_HH_
#define _VATA_BDD_BU_TRANS_TABLE_HH_

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
		class BDDBottomUpTransTable;
	}
}


template <
	typename State,
	typename Leaf
>
class VATA::Util::BDDBottomUpTransTable
{
public:   // data types

	typedef State StateType;
	typedef std::vector<StateType> StateTuple;
	typedef Leaf LeafType;
	typedef VATA::MTBDDPkg::OndriksMTBDD<LeafType> MTBDD;
	typedef std::unordered_map<StateTuple, MTBDD, boost::hash<StateTuple>>
		TupleMap;

private:  // data members

	MTBDD defaultMtbdd_;

	TupleMap mtbddMap_;

public:   // methods

	BDDBottomUpTransTable() :
		defaultMtbdd_(LeafType()),
		mtbddMap_()
	{ }

	inline const MTBDD& GetMtbdd(const StateTuple& tuple) const
	{
		typename TupleMap::const_iterator itHt;
		if ((itHt = mtbddMap_.find(tuple)) == mtbddMap_.end())
		{	// in case we are trying to access some nonsense
			return defaultMtbdd_;
		}

		return itHt->second;
	}

	inline void SetMtbdd(const StateTuple& tuple, const MTBDD& bdd)
	{
		auto itBoolPair = mtbddMap_.insert(std::make_pair(tuple, bdd));
		if (!itBoolPair.second)
		{	// in case there already is something
			(itBoolPair.first)->second = bdd;
		}
	}

	inline void RemoveMtbdd(const StateTuple& tuple)
	{
		if (mtbddMap_.erase(tuple) != 1)
		{
			assert(false);
		}
	}

	inline const TupleMap& GetTupleMap() const
	{
		return mtbddMap_;
	}

	inline size_t size() const
	{
		return mtbddMap_.size();
	}
};

#endif
