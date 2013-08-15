/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the core of a BDD-based top-down tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TD_TREE_AUT_CORE_HH_
#define _VATA_BDD_TD_TREE_AUT_CORE_HH_

// VATA
#include <vata/symbolic_aut_base.hh>
#include <vata/util/ord_vector.hh>
#include <vata/util/util.hh>
#include <vata/util/convert.hh>
#include <vata/incl_param.hh>

// MTBDD
#include "mtbdd/apply1func.hh"
#include "mtbdd/apply2func.hh"
#include "mtbdd/ondriks_mtbdd.hh"
#include "mtbdd/void_apply1func.hh"
#include "mtbdd/void_apply2func.hh"

#include "util/bdd_td_trans_table.hh"
#include "bdd_bu_tree_aut_core.hh"

namespace VATA { class BDDTDTreeAutCore; }


GCC_DIAG_OFF(effc++)
class VATA::BDDTDTreeAutCore : public SymbolicAutBase
{
GCC_DIAG_ON(effc++)

	friend VATA::BDDBUTreeAutCore;

public:   // data types

	using StateTuple                = std::vector<StateType>;
	using StateTupleSet             = VATA::Util::OrdVector<StateTuple>;
	using DownInclStateTupleSet     = StateTupleSet;
	using DownInclStateTupleVector  = std::vector<StateTuple>;

	using TransMTBDD   = VATA::MTBDDPkg::OndriksMTBDD<StateTupleSet>;

private:  // data types

	using StateSet                  = std::unordered_set<StateType>;
	using StateSetLight             = VATA::Util::OrdVector<StateType>;

	using TransTable       = VATA::Util::BDDTopDownTransTable<StateType, StateTupleSet>;
	using TransTablePtr    = std::shared_ptr<TransTable>;
	using StateMap         = TransTable::StateMap;

	using BDD          = VATA::MTBDDPkg::OndriksMTBDD<bool>;

	using AutDescription   = VATA::Util::AutDescription;
	using Convert          = VATA::Util::Convert;

	using SymbolBackTranslatorStrict   =
		VATA::BDDTopDownTreeAut::SymbolBackTranslatorStrict;

	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class UnionApplyFunctor :
		public VATA::MTBDDPkg::Apply2Functor<
			UnionApplyFunctor,
			StateTupleSet,
			StateTupleSet,
			StateTupleSet>
	{
	GCC_DIAG_ON(effc++)
	public:   // methods

		StateTupleSet ApplyOperation(
			const StateTupleSet&    lhs,
			const StateTupleSet&    rhs)
		{
			return lhs.Union(rhs);
		}
	};

private:   // constants

	static const size_t SYMBOL_ARITY_LENGTH = 6;

private:  // constants

	static const size_t MAX_SYMBOL_ARITY =
		VATA::Util::IntExp2(SYMBOL_ARITY_LENGTH) - 1;

	static const size_t SYMBOL_TOTAL_SIZE = SYMBOL_SIZE + SYMBOL_ARITY_LENGTH;


private:  // data members

	StateSet finalStates_;
	TransTablePtr transTable_;


private:  // methods

	template <
		class StateTransFunc,
		class SymbolTransFunc>
	void loadFromAutDescExplicit(
		const AutDescription&       desc,
		StateTransFunc              stateTranslator,
		SymbolTransFunc             symbolTranslator)
	{
		for (const AutDescription::State& fst : desc.finalStates)
		{	// traverse final states
			finalStates_.insert(stateTranslator(fst));
		}

		for (auto tr : desc.transitions)
		{	// traverse the transitions
			const AutDescription::StateTuple& childrenStr = tr.first;
			const std::string& symbolStr = tr.second;
			const AutDescription::State& parentStr = tr.third;

			// translate the parent state
			StateType parent = stateTranslator(parentStr);

			// translate children
			StateTuple children;
			for (const AutDescription::State& tupSt : childrenStr)
			{	// for all children states
				children.push_back(stateTranslator(tupSt));
			}

			// translate the symbol
			SymbolType symbol = symbolTranslator(symbolStr);

			AddTransition(children, symbol, parent);
		}
	}

	template <
		class StateTransFunc,
		class SymbolTransFunc>
	void loadFromAutDescSymbolic(
		const AutDescription&      /* desc */,
		StateTransFunc             /* stateTranslator */,
		SymbolTransFunc            /* symbolTranslator */)
	{
		assert(false);
	}

	template <
		class StateBackTransFunc,
		class SymbolBackTransFunc>
	AutDescription dumpToAutDescExplicit(
		StateBackTransFunc          stateBackTranslator,
		SymbolBackTransFunc         /* symbolTranslator */) const
	{
		GCC_DIAG_OFF(effc++)
		class CondColApplyFunctor :
			public VATA::MTBDDPkg::VoidApply2Functor<CondColApplyFunctor,
			StateTupleSet, bool>
		{
		GCC_DIAG_ON(effc++)

		public:   // data types

			typedef std::list<StateTuple> AccumulatorType;

		private:  // data members

			AccumulatorType accumulator_;

		public:

			CondColApplyFunctor() :
				accumulator_()
			{ }

			const AccumulatorType& GetAccumulator() const
			{
				return accumulator_;
			}

			void Clear()
			{
				accumulator_.clear();
			}

			void ApplyOperation(const StateTupleSet& lhs, const bool& rhs)
			{
				if (rhs)
				{
					accumulator_.insert(accumulator_.end(), lhs.begin(), lhs.end());
				}
			}
		};

		AutDescription desc;

		// copy final states
		for (const StateType& fst : finalStates_)
		{	// copy final states
			desc.finalStates.insert(stateBackTranslator(fst));
		}

		CondColApplyFunctor collector;

		// copy states, transitions and symbols
		for (auto stateBddPair : GetStates())
		{	// for all states
			const StateType& state = stateBddPair.first;

			std::string stateStr = stateBackTranslator(state);

			desc.states.insert(stateStr);

			const TransMTBDD& transMtbdd = GetMtbdd(state);

			for (auto sym : GetSymbolDict())
			{	// iterate over all known symbols
				const std::string& symbol = sym.first;
				BDD symbolBdd(sym.second, true, false);

				collector.Clear();
				collector(transMtbdd, symbolBdd);

				for (auto tuple : collector.GetAccumulator())
				{	// for each state tuple for which there is a transition
					std::vector<std::string> tupleStr;
					for (auto tupState : tuple)
					{	// for each element in the tuple
						tupleStr.push_back(stateBackTranslator(tupState));
					}

					desc.transitions.insert(AutDescription::Transition(tupleStr, symbol,
						stateStr));
				}
			}
		}

		return desc;
	}

	template <
		class StateBackTransFunc,
		class SymbolTransFunc>
	AutDescription dumpToAutDescSymbolic(
		StateBackTransFunc           /* stateBackTranslator */,
		SymbolTransFunc              /* symbolTranslator */) const
	{
		throw NotImplementedException(__func__);
	}

	void addArityToSymbol(
		SymbolType&                 symbol,
		size_t                      arity) const
	{
		// Assertions
		assert(arity <= MAX_SYMBOL_ARITY);

		SymbolType prefix(SYMBOL_ARITY_LENGTH, arity);

		symbol.append(prefix);
	}


public:   // methods


	BDDTDTreeAutCore();


	BDDTDTreeAutCore(
		TransTablePtr                     transTable);


	BDDTDTreeAutCore(
		const BDDTDTreeAutCore&           aut);


	BDDTDTreeAutCore(
		BDDTDTreeAutCore&&                aut);


	BDDTDTreeAutCore& operator=(
		const BDDTDTreeAutCore&           rhs);


	BDDTDTreeAutCore& operator=(
		BDDTDTreeAutCore&&                rhs);


	const StateMap& GetStates() const
	{
		return transTable_->GetStateMap();
	}

	const StateSet& GetFinalStates() const
	{
		return finalStates_;
	}

	TransTablePtr& GetTransTable() const
	{
		return const_cast<TransTablePtr&>(transTable_);
	}

	const TransMTBDD& GetMtbdd(
		const StateType&               state) const
	{
		return transTable_->GetMtbdd(state);
	}

	void SetMtbdd(
		const StateType&               state,
		const TransMTBDD&              mtbdd)
	{
		transTable_->SetMtbdd(state, mtbdd);
	}

	void SetStateFinal(
		const StateType&               state)
	{
		finalStates_.insert(state);
	}

	bool IsStateFinal(
		const StateType&               state) const
	{
		return finalStates_.find(state) != finalStates_.end();
	}


	void AddTransition(
		const StateTuple&       children,
		SymbolType              symbol,
		const StateType&        parent);


	void AddSimplyTransition(
		const StateTuple&     children,
		SymbolType            symbol,
		const StateType&      parent);


	template <class OperationFunc>
	static void ForeachDownSymbolFromStateAndStateSetDo(
		const BDDTDTreeAutCore&          lhs,
		const BDDTDTreeAutCore&          rhs,
		const StateType&                 lhsState,
		const StateSetLight&             rhsSet,
		OperationFunc&                   opFunc)
	{
		GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
		class OperationApplyFunctor : public VATA::MTBDDPkg::VoidApply2Functor<
			OperationApplyFunctor,
			StateTupleSet,
			StateTupleSet>
		{
		GCC_DIAG_ON(effc++)

		private:  // data members

			OperationFunc& opFunc_;

		public:   // methods

			OperationApplyFunctor(OperationFunc& opFunc) :
				opFunc_(opFunc)
			{ }

			void ApplyOperation(
				const StateTupleSet&      lhs,
				const StateTupleSet&      rhs)
			{
				auto AccessElementF = [](const StateTuple& tuple){return tuple;};
				opFunc_(lhs, AccessElementF, rhs, AccessElementF);

				if (opFunc_.IsProcessingStopped())
				{	// in case the operator wants to stop processing
					this->stopProcessing();
				}
			}
		};

		UnionApplyFunctor unioner;
		TransMTBDD rhsUnionMtbdd((StateTupleSet()));

		// collect the RHS's MTBDDs leaves
		for (const StateType& rhsState : rhsSet)
		{
			rhsUnionMtbdd = unioner(rhsUnionMtbdd, rhs.GetMtbdd(rhsState));
		}

		// create apply functor
		OperationApplyFunctor opApplyFunc(opFunc);

		// perform the apply operation
		opApplyFunc(lhs.GetMtbdd(lhsState), rhsUnionMtbdd);
	}


	template <
		class StateTransFunc,
		class SymbolTransFunc>
	void LoadFromAutDesc(
		const AutDescription&      desc,
		StateTransFunc             stateTranslator,
		SymbolTransFunc            symbolTranslator,
		const std::string&         params = "")
	{
		if (params == "symbolic")
		{
			loadFromAutDescSymbolic(desc, stateTranslator, symbolTranslator);
		}
		else
		{
			loadFromAutDescExplicit(desc, stateTranslator, symbolTranslator);
		}
	}


	void LoadFromAutDesc(
		const AutDescription&         desc,
		StringToStateDict&            stateDict);


	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		StringToStateDict&               stateDict);


	template <
		class StateTransFunc,
		class SymbolTransFunc>
	void LoadFromString(
		VATA::Parsing::AbstrParser&       parser,
		const std::string&                str,
		StateTransFunc                    stateTranslator,
		SymbolTransFunc                   symbolTranslator,
		const std::string&                params = "")
	{
		LoadFromAutDesc(parser.ParseString(str), stateTranslator,
				symbolTranslator, params);
	}


	template <class SymbolTransFunc>
	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		SymbolTransFunc                  symbolTranslator)
	{
		typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
			StateTranslator;

		AutBase::StringToStateDict dict;

		LoadFromString(parser, str, StateTranslator(dict, *this), symbolTranslator);
	}

	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer) const;

	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StringToStateDict&                   stateDict) const;


	template <
		class StateBackTransFunc,
		class SymbolTransFunc>
	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&    serializer,
		StateBackTransFunc                       stateBackTranslator,
		SymbolTransFunc                          symbolTranslator,
		const std::string&                       params = "") const
	{
		AutDescription desc;
		if (params == "symbolic")
		{
			desc = dumpToAutDescSymbolic(stateBackTranslator, symbolTranslator);
		}
		else
		{
			desc = dumpToAutDescExplicit(stateBackTranslator, symbolTranslator);
		}

		return serializer.Serialize(desc);
	}


	template <class SymbolTransFunc>
	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		SymbolTransFunc                            symbolTranslator,
		const std::string&                         params = "") const
	{
		return DumpToString(serializer,
			[](const StateType& state){return Convert::ToString(state);},
			symbolTranslator, params);
	}


	std::string DumpToDot() const;


	static bool ShareTransTable(
		const BDDTDTreeAutCore&     lhs,
		const BDDTDTreeAutCore&     rhs)
	{
		return lhs.transTable_ == rhs.transTable_;
	}


	static const DownInclStateTupleVector& StateTupleSetToVector(
		const DownInclStateTupleSet&        tupleSet)
	{
		return tupleSet.ToVector();
	}


	template <class MTBDD>
	static MTBDD GetMtbddForArity(
		const MTBDD&                 mtbdd,
		size_t                       arity)
	{
		// Assertions
		assert(arity <= MAX_SYMBOL_ARITY);

		SymbolType arityAsgn(SYMBOL_ARITY_LENGTH, arity);

		return mtbdd.GetMtbddForPrefix(arityAsgn, SYMBOL_SIZE);
	}


	void ReindexStates(
		BDDTDTreeAutCore&           dstAut,
		StateToStateTranslator&     stateTrans) const;


	BDDTDTreeAutCore ReindexStates(
		StateToStateTranslator&     stateTrans) const
	{
		BDDTDTreeAutCore res;
		this->ReindexStates(res, stateTrans);

		return res;
	}


	BDDTDTreeAutCore RemoveUnreachableStates() const;


	BDDTDTreeAutCore RemoveUselessStates() const;


	static BDDTDTreeAutCore UnionDisjointStates(
		const BDDTDTreeAutCore&       lhs,
		const BDDTDTreeAutCore&       rhs);


	static BDDTDTreeAutCore Intersection(
		const BDDTDTreeAutCore&       lhs,
		const BDDTDTreeAutCore&       rhs,
		AutBase::ProductTranslMap*    pTranslMap = nullptr);


	static BDDTDTreeAutCore Union(
		const BDDTDTreeAutCore&       lhs,
		const BDDTDTreeAutCore&       rhs,
		AutBase::StateToStateMap*     pTranslMapLhs = nullptr,
		AutBase::StateToStateMap*     pTranslMapRhs = nullptr);


	static bool CheckInclusion(
		const BDDTDTreeAutCore&       smaller,
		const BDDTDTreeAutCore&       bigger,
		const VATA::InclParam&        params);
};


#endif
