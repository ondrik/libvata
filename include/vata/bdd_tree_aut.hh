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
#include <tr1/cstdint>

namespace VATA { class BDDTreeAut; }

class VATA::BDDTreeAut
	: public AbstrAut
{
public:   // public data types

	typedef VATA::MTBDDPkg::VarAsgn SymbolType;
	typedef std::vector<StateType> StateTuple;

private:  // private data types

	typedef VATA::Util::OrdVector<StateTuple> StateTupleSet;

	typedef VATA::MTBDDPkg::OndriksMTBDD<StateTupleSet> TransMTBDD;
	typedef VATA::MTBDDPkg::OndriksMTBDD<bool> BDD;

	typedef std::vector<StateType> StateVector;

	typedef VATA::Util::TDBDDTransTable< StateType, VATA::Util::OrdVector>
		TransTable;

	typedef std::tr1::shared_ptr<TransTable> TransTablePtr;

	typedef VATA::Util::AutDescription AutDescription;

	typedef VATA::Util::TwoWayDict<std::string, SymbolType> StringToSymbolDict;

	class UnionApplyFunctor :
		public VATA::MTBDDPkg::AbstractApply2Functor<StateTupleSet,
		StateTupleSet, StateTupleSet>
	{
	public:

		virtual StateTupleSet ApplyOperation(const StateTupleSet& lhs,
			const StateTupleSet& rhs)
		{
			return lhs.Union(rhs);
		}
	};


private:  // private data members

	StateVector states_;
	StateVector finalStates_;
	TransTablePtr transTable_;

	static StringToSymbolDict symbolDict_;

private:  // private methods

	bool isValid() const;

	void copyStates(const BDDTreeAut& src);

	static SymbolType addSymbol();

	inline const TransMTBDD& getMtbdd(const StateType& state) const
	{
		// Assertions
		assert(isValid());

		return transTable_->GetMtbdd(state);
	}

	inline void setMtbdd(const StateType& state, const TransMTBDD& mtbdd)
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

	void loadFromAutDescExplicit(const AutDescription& desc,
		StringToStateDict* pStateDict);

	void loadFromAutDescSymbolic(const AutDescription& desc,
		StringToStateDict* pStateDict);

	AutDescription dumpToAutDescExplicit(
		const StringToStateDict* pStateDict) const;

	AutDescription dumpToAutDescSymbolic(
		const StringToStateDict* pStateDict) const;

	bool isStandAlone() const;

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
		transTable_(new TransTable)
	{
		// Assertions
		assert(isValid());
	}

	BDDTreeAut(TransTablePtr transTable) :
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

	inline const StateVector& GetFinalStates() const
	{
		return finalStates_;
	}

	virtual void LoadFromString(VATA::Parsing::AbstrParser& parser,
		const std::string& str,
		StringToStateDict* pStateDict = static_cast<StringToStateDict*>(0),
		const std::string& params = "");

	virtual std::string DumpToString(VATA::Serialization::AbstrSerializer& serializer,
		const StringToStateDict* pStateDict = static_cast<StringToStateDict*>(0),
		const std::string& params = "") const;

	inline StateType AddState()
	{
		// Assertions
		assert(isValid());

		StateType newState = transTable_->AddState();
		states_.push_back(newState);

		assert(isValid());

		return newState;
	}

	inline static const SymbolType& TranslateStringToSymbol(const std::string& str)
	{
		return symbolDict_.TranslateFwd(str);
	}

	void AddTransition(const StateTuple& children, const SymbolType& symbol,
		const StateType& state);

	void AddSimplyTransition(const StateTuple& children, const SymbolType& symbol,
		const StateType& parent);

	~BDDTreeAut();
};

#endif
