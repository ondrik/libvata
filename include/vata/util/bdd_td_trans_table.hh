/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a top-down BDD transition function.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TD_TRANS_TABLE_HH_
#define _VATA_BDD_TD_TRANS_TABLE_HH_

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
		class BDDTopDownTransTable;
	}
}


template <
	typename State,
	typename Leaf
>
class VATA::Util::BDDTopDownTransTable
{
public:   // data types

	typedef State StateType;

private:  // data types

	typedef Leaf LeafType;
	typedef VATA::MTBDDPkg::OndriksMTBDD<LeafType> MTBDD;

public:   // data types

	typedef std::unordered_map<StateType, MTBDD> StateMap;

private:  // data members

	MTBDD defaultMtbdd_;

	StateMap mtbddMap_;

public:   // methods

	BDDTopDownTransTable() :
		defaultMtbdd_(LeafType()),
		mtbddMap_()
	{ }

	inline const MTBDD& GetMtbdd(const StateType& state) const
	{
		typename StateMap::const_iterator itHt;
		if ((itHt = mtbddMap_.find(state)) == mtbddMap_.end())
		{	// in case we are trying to access some nonsense
			return defaultMtbdd_;
		}

		return itHt->second;
	}

	inline void SetMtbdd(const StateType& state, const MTBDD& bdd)
	{
		auto itBoolPair = mtbddMap_.insert(std::make_pair(state, bdd));
		if (!itBoolPair.second)
		{	// in case there already is something
			(itBoolPair.first)->second = bdd;
		}
	}

	inline void RemoveMtbdd(const StateType& state)
	{
		if (mtbddMap_.erase(state) != 1)
		{
			assert(false);
		}
	}

	inline const StateMap& GetStateMap() const
	{
		return mtbddMap_;
	}
};

#endif
