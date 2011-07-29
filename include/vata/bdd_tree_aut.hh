/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a BDD-based tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TREE_AUT_HH_
#define _VATA_BDD_TREE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/abstr_aut.hh>
#include <vata/mtbdd/apply2func.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/util/ord_vector.hh>
#include <vata/util/td_bdd_trans_table.hh>
#include <vata/util/vector_map.hh>

// Standard library headers
#include <stdint.h>

namespace VATA { class BDDTreeAut; }

class VATA::BDDTreeAut
	: public AbstrAut
{
public:   // public data types

	typedef VATA::MTBDDPkg::VariableAssignment SymbolType;

private:  // private data types

	typedef std::vector<StateType> StateTuple;
	typedef VATA::Util::OrdVector<StateTuple> StateTupleSet;

	typedef VATA::MTBDDPkg::OndriksMTBDD<StateTupleSet> MTBDD;

	typedef std::vector<StateType> StateVector;

	typedef VATA::Util::TDBDDTransTable< StateType, VATA::Util::OrdVector>
		TransitionTable;

	typedef std::tr1::shared_ptr<TransitionTable> TransitionTablePtr;

//	struct StateContainer
//	{
//	private:  // data members
//
//		MTBDD bdd_;
//		size_t refcnt_;
//
//	private:  // methods
//
//		StateContainer()
//			: bdd_(StateSet()),
//				refcnt_(1)
//		{ }
//
//		static inline StateType ToStateType(StateContainer* sc)
//		{
//			// Assertions
//			assert(sc != static_cast<StateContainer*>(0));
//
//			return reinterpret_cast<StateType>(sc);
//		}
//
//		static inline StateContainer* ToContainer(StateType st)
//		{
//			// Assertions
//			assert(st != 0);
//
//			return reinterpret_cast<StateContainer*>(st);
//		}
//
//	public:   // methods
//
//		static inline StateContainer* Create()
//		{
//			return new StateContainer();
//		}
//
//		static inline void IncrementRefCnt(StateType st)
//		{
//			// Assertions
//			assert(st != 0);
//			assert(ToContainer(st)->refcnt_ > 0);
//
//			++(ToContainer(st)->refcnt_);
//		}
//
//		static inline void DecrementRefCnt(StateType st)
//		{
//			// Assertions
//			assert(st != 0);
//			assert(ToContainer(st)->refcnt_ > 0);
//
//			if (--(ToContainer(st)->refcnt_) == 0)
//			{	// in case there is no one else having this state
//				delete ToContainer(st);
//			}
//		}
//	};

private:  // private data members

	StateVector states_;
	StateVector finalStates_;
	TransitionTablePtr transTable_;

private:  // private methods

	inline bool isValid() const
	{
		if (!std::includes(states_.begin(), states_.end(),
			finalStates_.begin(), finalStates_.end()))
		{	// in case the set of final states is not a subset of the set of states
			return false;
		}

		if (transTable_.get() == static_cast<TransitionTable*>(0))
		{	// in case the transition table pointer is bad
			return false;
		}

		return true;
	}

	void copyStates(const BDDTreeAut& src);

	inline const MTBDD& getMtbdd(const StateType& state) const
	{
		// Assertions
		assert(isValid());

		return transTable_->GetMtbdd(state);
	}

	inline void setMtbdd(const StateType& state, const MTBDD& mtbdd)
	{
		// Assertions
		assert(isValid());

		transTable_->SetMtbdd(state, mtbdd);
	}

	inline bool hasEmptyStateSet() const
	{
		// Assertions
		assert(isValid());

		return states_.empty();
	}

	static bool haveDisjointStateSets(const BDDTreeAut& lhs, const BDDTreeAut& rhs);

	// TODO: put this somewhere else
//	static BDDTreeAut* makeUnionBU(const BDDTreeAut& lhs, const BDDTreeAut& rhs,
//			const std::string&)
//	{
//		// Assertions
//		assert(lhs.isValid());
//		assert(rhs.isValid());
//		assert(haveDisjointStateSets(lhs, rhs));
//
//		class UnionApplyFunctor
//			: public VATA::MTBDDPkg::AbstractApply2Functor<StateTupleSet, StateTupleSet, StateTupleSet>
//		{
//		public:
//			virtual StateTupleSet ApplyOperation(const StateTupleSet& lhs, const StateTupleSet& rhs)
//			{
//				return lhs.Union(rhs);
//			}
//		};
//
//		BDDTreeAut* result = new BDDTreeAut(lhs);
//		result->copyStates(rhs);
//
//		const MTBDD& lhsMtbdd = lhs.getMtbdd(StateTuple());
//		const MTBDD& rhsMtbdd = rhs.getMtbdd(StateTuple());
//
//		UnionApplyFunctor unionFunc;
//		MTBDD resultMtbdd = unionFunc(lhsMtbdd, rhsMtbdd);
//
//		result->setMtbdd(StateTuple(), resultMtbdd);
//
//		return result;
//	}


public:   // public methods

	BDDTreeAut() :
		states_(),
		finalStates_(),
		transTable_(new TransitionTable)
	{
		// Assertions
		assert(isValid());
	}

	BDDTreeAut(TransitionTablePtr transTable) :
		states_(),
		finalStates_(),
		transTable_(transTable)
	{
		// Assertions
		assert(isValid());
	}

	BDDTreeAut(const BDDTreeAut& aut) :
		states_(aut.states_),
		finalStates_(aut.finalStates_),
		transTable_(aut.transTable_)
	{
		// Assertions
		assert(isValid());
	}

	virtual void LoadFromString(VATA::Parsing::AbstrParser& parser,
		const std::string& str,
		StringToStateDict* pStateDict = static_cast<StringToStateDict*>(0));

	virtual std::string DumpToString(VATA::Serialization::AbstrSerializer& serializer,
		StringToStateDict* pStateDict = static_cast<StringToStateDict*>(0));

	inline StateType AddState()
	{
		// Assertions
		assert(isValid());

		return transTable_->AddState();
	}

	void AddTransition(const StateTuple& children, const SymbolType& sym,
		const StateType& state)
	{
		assert(isValid());

		assert(false);
	}

	~BDDTreeAut();
};

#endif
