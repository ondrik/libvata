/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a top-down BDD transition function.
 *
 *****************************************************************************/

#ifndef _VATA_TD_BDD_TRANS_TABLE_HH_
#define _VATA_TD_BDD_TRANS_TABLE_HH_

// VATA headers
#include <vata/vata.hh>

namespace VATA
{
	namespace Util
	{
		template <
			typename State,
			template <typename> class Set
		>
		class TDBDDTransTable;
	}
}


template <
	typename State,
	template <typename> class Set
>
class VATA::Util::TDBDDTransTable
{
public:   // data types

	typedef State StateType;

private:  // data types

	typedef std::vector<StateType> StateTuple;
	typedef Set<StateTuple> StateTupleSet;
	typedef VATA::MTBDDPkg::OndriksMTBDD<StateTupleSet> MTBDD;


	struct StateCell
	{
	public:   // data types

		typedef size_t RefCntType;

	private:  // data members

		MTBDD bdd_;
		RefCntType refcnt_;

	public:   // methods

		explicit StateCell(MTBDD bdd)
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
	};

	typedef std::tr1::unordered_map<StateType, StateCell> StateHashTable;

private:  // data members

	StateType nextStateCnt_;

	StateHashTable stateCellMap_;

public:   // methods

	TDBDDTransTable()
		: nextStateCnt_(1),
			stateCellMap_()
	{ }

	inline StateType AddState()
	{
		// Assertions
		assert(nextStateCnt_ != 0);

		stateCellMap_.insert(std::make_pair(nextStateCnt_,
			StateCell(MTBDD(StateTupleSet()))));

		return nextStateCnt_++;
	}

	inline const MTBDD& GetMtbdd(const StateType& state) const
	{
		assert(false);
	}

	inline void SetMtbdd(const StateType& state, const MTBDD& bdd) const
	{
		assert(false);
	}

	inline void IncrementStateRefCnt(const StateType& state)
	{
		stateCellMap_[state].IncrementRefCnt();
	}

	inline void DecrementStateRefCnt(const StateType& state)
	{
		typename StateHashTable::iterator itStates;
		if ((itStates = stateCellMap_.find(state)) == stateCellMap_.end())
		{	// in case the state is not in the hash table
			assert(false);    // fail gracefully
		}

		if ((itStates->second).DecrementRefCnt() == 0)
		{	// in case no one uses the state
			stateCellMap_.erase(itStates);
		}
	}

	~TDBDDTransTable()
	{
		// Assertions
		assert(stateCellMap_.empty());
	}
};

#endif
