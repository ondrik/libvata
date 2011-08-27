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
#include <vata/aut_base.hh>
#include <vata/aut_op.hh>
#include <vata/mtbdd/apply2func.hh>
#include <vata/mtbdd/void_apply2func.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/util/ord_vector.hh>
#include <vata/util/td_bdd_trans_table.hh>
#include <vata/util/vector_map.hh>
#include <vata/util/util.hh>

// Standard library headers
#include <cstdint>
#include <unordered_set>

namespace VATA { class BDDTreeAut; }

GCC_DIAG_OFF(effc++)
class VATA::BDDTreeAut
	: public AutBase
{
GCC_DIAG_ON(effc++)

	template <class Aut>
	friend Aut Union(const Aut&, const Aut&, AutBase::StateToStateMap*);

	template <class Aut>
	friend Aut Intersection(const Aut&, const Aut&,
		AutBase::ProductTranslMap*);

	template <class Aut>
	friend Aut RemoveUnreachableStates(const Aut&,
		AutBase::StateToStateMap* pTranslMap);

	template <class Aut>
	friend bool CheckInclusion(const Aut&, const Aut&);

public:   // public data types

	typedef VATA::MTBDDPkg::VarAsgn SymbolType;
	typedef std::vector<StateType> StateTuple;
	typedef VATA::Util::OrdVector<StateTuple> StateTupleSet;
	typedef std::unordered_set<StateType> StateSet;
	typedef VATA::Util::OrdVector<StateType> StateSetLight;

private:  // private data types


	typedef VATA::MTBDDPkg::OndriksMTBDD<StateTupleSet> TransMTBDD;
	typedef VATA::MTBDDPkg::OndriksMTBDD<bool> BDD;

	typedef VATA::Util::TDBDDTransTable< StateType, VATA::Util::OrdVector>
		TransTable;

	typedef std::shared_ptr<TransTable> TransTablePtr;

	typedef VATA::Util::AutDescription AutDescription;

	typedef VATA::Util::TwoWayDict<std::string, SymbolType> StringToSymbolDict;

	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class UnionApplyFunctor :
		public VATA::MTBDDPkg::Apply2Functor<UnionApplyFunctor, StateTupleSet,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_ON(effc++)
	public:   // methods

		inline StateTupleSet ApplyOperation(const StateTupleSet& lhs,
			const StateTupleSet& rhs)
		{
			return lhs.Union(rhs);
		}
	};


private:  // constants

	static const size_t SYMBOL_VALUE_LENGTH = 16;

	static const size_t SYMBOL_ARITY_LENGTH = 6;
	static const size_t MAX_SYMBOL_ARITY =
		VATA::Util::IntExp2(SYMBOL_ARITY_LENGTH) - 1;

	static const size_t SYMBOL_SIZE = SYMBOL_VALUE_LENGTH + SYMBOL_ARITY_LENGTH;


private:  // data members


	StateSet states_;
	StateSet finalStates_;
	TransTablePtr transTable_;

	static StringToSymbolDict symbolDict_;
	static SymbolType nextBaseSymbol_;

private:  // methods

	bool isValid() const;

	void copyStates(const BDDTreeAut& src);

	static SymbolType addBaseSymbol()
	{
		return nextBaseSymbol_++;
	}

	inline void deallocateStates()
	{
		// Assertions
		assert(isValid());

		for (StateSet::iterator itSt = states_.begin();
			itSt != states_.end(); ++itSt)
		{	// release all states
			transTable_->DecrementStateRefCnt(*itSt);
		}
	}

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

		return states_.empty() && finalStates_.empty();
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

	inline bool isStateLocal(const StateType& state) const
	{
		// Assertions
		assert(isValid());

		return (states_.find(state) != states_.end());
	}

	template <typename T, class Container>
	inline StateType safelyTranslateToState(const T& value, Container& dict)
	{
		// Assertions
		assert(isValid());

		StateType state;
		typename Container::const_iterator itHt;
		if ((itHt = dict.find(value)) != dict.end())
		{	// in case the state is known
			state = itHt->second;
		}
		else
		{	// in case there is no translation for the state
			state = AddState();
			dict.insert(std::make_pair(value, state));
		}

		return state;
	}

	inline void addArityToSymbol(SymbolType& symbol, size_t arity) const
	{
		// Assertions
		assert(arity <= MAX_SYMBOL_ARITY);

		SymbolType prefix(SYMBOL_ARITY_LENGTH, arity);

		symbol.append(prefix);
	}


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

	BDDTreeAut(const BDDTreeAut& aut);
	BDDTreeAut& operator=(const BDDTreeAut& rhs);

	inline const StateSet& GetStates() const
	{
		return states_;
	}

	inline const StateSet& GetFinalStates() const
	{
		return finalStates_;
	}

	inline TransTablePtr& GetTransTable() const
	{
		return const_cast<TransTablePtr&>(transTable_);
	}

	void LoadFromString(VATA::Parsing::AbstrParser& parser,
		const std::string& str,
		StringToStateDict* pStateDict = nullptr,
		const std::string& params = "");

	std::string DumpToString(VATA::Serialization::AbstrSerializer& serializer,
		const StringToStateDict* pStateDict = nullptr,
		const std::string& params = "") const;

	inline StateType AddState()
	{
		// Assertions
		assert(isValid());

		StateType newState = transTable_->AddState();
		states_.insert(newState);

		assert(isValid());

		return newState;
	}

	inline void SetStateFinal(const StateType& state)
	{
		// Assertions
		assert(isValid());
		assert(isStateLocal(state));

		finalStates_.insert(state);
	}

	inline static const SymbolType& TranslateStringToSymbol(const std::string& str)
	{
		return symbolDict_.TranslateFwd(str);
	}

	inline static const SymbolType& SafelyTranslateStringToSymbol(
		const std::string& str)
	{
		StringToSymbolDict::ConstIteratorFwd itSym;
		if ((itSym = symbolDict_.FindFwd(str)) != symbolDict_.EndFwd())
		{	// in case the state name is known
			return itSym->second;
		}
		else
		{	// in case there is no translation for the state name
			SymbolType symbol = addBaseSymbol();
			return symbolDict_.insert(std::make_pair(str, symbol)).first->second;
		}
	}

	void AddTransition(const StateTuple& children, const SymbolType& symbol,
		const StateType& state);

	void AddSimplyTransition(const StateTuple& children, SymbolType symbol,
		const StateType& parent);

	template <class OperationFunc>
	static void ForeachDownSymbolFromStateAndStateSetDo(const BDDTreeAut& lhs,
		const BDDTreeAut& rhs, const StateType& lhsState,
		const StateSetLight& rhsSet, OperationFunc& opFunc)
	{
		// Assertions
		assert(lhs.isValid());
		assert(rhs.isValid());

		GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
		class OperationApplyFunctor :
			public VATA::MTBDDPkg::VoidApply2Functor<OperationApplyFunctor,
			StateTupleSet, StateTupleSet>
		{
		GCC_DIAG_ON(effc++)

		private:  // data members

			OperationFunc& opFunc_;

		public:   // methods

			OperationApplyFunctor(OperationFunc& opFunc) :
				opFunc_(opFunc)
			{ }

			inline void ApplyOperation(const StateTupleSet& lhs,
				const StateTupleSet& rhs)
			{
				opFunc_(lhs, rhs);

				if (opFunc_.IsProcessingStopped())
				{	// in case the operator wants to stop processing
					this->stopProcessing();
				}
			}
		};

		UnionApplyFunctor unioner;
		TransMTBDD rhsUnionMtbdd((StateTupleSet()));

		// collect the RHS's MTBDDs leaves
		for (auto itRhsSt = rhsSet.cbegin(); itRhsSt != rhsSet.cend(); ++itRhsSt)
		{
			rhsUnionMtbdd = unioner(rhsUnionMtbdd, rhs.getMtbdd(*itRhsSt));
		}

		// create apply functor
		OperationApplyFunctor opApplyFunc(opFunc);

		// perform the apply operation
		opApplyFunc(lhs.getMtbdd(lhsState), rhsUnionMtbdd);
	}

	~BDDTreeAut();
};

#endif
