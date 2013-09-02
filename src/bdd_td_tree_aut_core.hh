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
#include <vata/symbolic.hh>
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
class VATA::BDDTDTreeAutCore : public SymbolicTreeAutBase
{
GCC_DIAG_ON(effc++)

	friend BDDBUTreeAutCore;

public:   // data types

	using StateTuple                = std::vector<StateType>;
	using StateTupleSet             = Util::OrdVector<StateTuple>;
	using DownInclStateTupleSet     = StateTupleSet;
	using DownInclStateTupleVector  = std::vector<StateTuple>;

	using TransMTBDD   = MTBDDPkg::OndriksMTBDD<StateTupleSet>;

private:  // data types

	using StateSet                  = std::unordered_set<StateType>;
	using StateSetLight             = Util::OrdVector<StateType>;

	using TransTable       = Util::BDDTopDownTransTable<StateType, StateTupleSet>;
	using TransTablePtr    = std::shared_ptr<TransTable>;
	using StateMap         = TransTable::StateMap;

	using BDD          = MTBDDPkg::OndriksMTBDD<bool>;

	using AutDescription   = Util::AutDescription;
	using Convert          = Util::Convert;

	using BackSymbolDict   = SymbolDict::MapBwdType;

	using SymbolBackTranslStrict   =
		BDDTopDownTreeAut::SymbolBackTranslStrict;

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
		class StateTranslFunc,
		class SymbolTranslFunc>
	void loadFromAutDescExplicit(
		const AutDescription&       desc,
		StateTranslFunc             stateTransl,
		SymbolTranslFunc            symbolTransl)
	{
		for (const AutDescription::State& fst : desc.finalStates)
		{	// traverse final states
			finalStates_.insert(stateTransl(fst));
		}

		for (auto tr : desc.transitions)
		{	// traverse the transitions
			const AutDescription::StateTuple& childrenStr = tr.first;
			const std::string& symbolStr = tr.second;
			const AutDescription::State& parentStr = tr.third;

			// translate the parent state
			StateType parent = stateTransl(parentStr);

			// translate children
			StateTuple children;
			for (const AutDescription::State& tupSt : childrenStr)
			{	// for all children states
				children.push_back(stateTransl(tupSt));
			}

			// translate the symbol
			SymbolType symbol = symbolTransl(symbolStr);

			AddTransition(children, symbol, parent);
		}
	}

	template <
		class StateTranslFunc,
		class SymbolTranslFunc>
	void loadFromAutDescSymbolic(
		const AutDescription&      /* desc */,
		StateTranslFunc            /* stateTranslator */,
		SymbolTranslFunc           /* symbolTranslator */)
	{
		assert(false);
	}

	template <
		class StateBackTranslFunc>
	AutDescription dumpToAutDescExplicit(
		StateBackTranslFunc           stateBackTransl,
		const SymbolDict&             symbolDict) const
	{
		GCC_DIAG_OFF(effc++)
		class CondColApplyFunctor : public VATA::MTBDDPkg::VoidApply2Functor<
			CondColApplyFunctor,
			StateTupleSet,
			bool>
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
			desc.finalStates.insert(stateBackTransl(fst));
		}

		CondColApplyFunctor collector;

		// copy states, transitions and symbols
		for (auto stateBddPair : this->GetStates())
		{	// for all states
			const StateType& state = stateBddPair.first;

			std::string stateStr = stateBackTransl(state);

			desc.states.insert(stateStr);

			const TransMTBDD& transMtbdd = this->GetMtbdd(state);

			for (auto strSymbol : symbolDict)
			{	// iterate over all known symbols
				const std::string& symbol = strSymbol.first;
				// ignore rank

				BDD symbolBdd(strSymbol.second, true, false);

				collector.Clear();
				collector(transMtbdd, symbolBdd);

				for (const StateTuple& tuple : collector.GetAccumulator())
				{	// for each state tuple for which there is a transition
					std::vector<std::string> tupleStr;
					for (const StateType& tupState : tuple)
					{	// for each element in the tuple
						tupleStr.push_back(stateBackTransl(tupState));
					}

					desc.transitions.insert(AutDescription::Transition(tupleStr, symbol,
						stateStr));
				}
			}
		}

		return desc;
	}

//	template <
//		class StateBackTransFunc,
//		class SymbolBackTransFunc>
//	AutDescription dumpToAutDescExplicit(
//		StateBackTransFunc          stateBackTranslator,
//		SymbolBackTransFunc         /* symbolTranslator */) const
//	{
//		GCC_DIAG_OFF(effc++)
//		class CondColApplyFunctor :
//			public VATA::MTBDDPkg::VoidApply2Functor<CondColApplyFunctor,
//			StateTupleSet, bool>
//		{
//		GCC_DIAG_ON(effc++)
//
//		public:   // data types
//
//			typedef std::list<StateTuple> AccumulatorType;
//
//		private:  // data members
//
//			AccumulatorType accumulator_;
//
//		public:
//
//			CondColApplyFunctor() :
//				accumulator_()
//			{ }
//
//			const AccumulatorType& GetAccumulator() const
//			{
//				return accumulator_;
//			}
//
//			void Clear()
//			{
//				accumulator_.clear();
//			}
//
//			void ApplyOperation(const StateTupleSet& lhs, const bool& rhs)
//			{
//				if (rhs)
//				{
//					accumulator_.insert(accumulator_.end(), lhs.begin(), lhs.end());
//				}
//			}
//		};
//
//		AutDescription desc;
//
//		// copy final states
//		for (const StateType& fst : finalStates_)
//		{	// copy final states
//			desc.finalStates.insert(stateBackTranslator(fst));
//		}
//
//		CondColApplyFunctor collector;
//
//		// copy states, transitions and symbols
//		for (auto stateBddPair : this->GetStates())
//		{	// for all states
//			const StateType& state = stateBddPair.first;
//
//			std::string stateStr = stateBackTranslator(state);
//
//			desc.states.insert(stateStr);
//
//			const TransMTBDD& transMtbdd = this->GetMtbdd(state);
//
//			for (auto strRankSymbol : this->GetSymbolDict())
//			{	// iterate over all known symbols
//				const std::string& symbol = strRankSymbol.first.symbolStr;
//				// ignore rank
//
//				BDD symbolBdd(strRankSymbol.second, true, false);
//
//				collector.Clear();
//				collector(transMtbdd, symbolBdd);
//
//				for (const StateTuple& tuple : collector.GetAccumulator())
//				{	// for each state tuple for which there is a transition
//					std::vector<std::string> tupleStr;
//					for (const StateType& tupState : tuple)
//					{	// for each element in the tuple
//						tupleStr.push_back(stateBackTranslator(tupState));
//					}
//
//					desc.transitions.insert(AutDescription::Transition(tupleStr, symbol,
//						stateStr));
//				}
//			}
//		}
//
//		return desc;
//	}

	template <
		class StateBackTranslFunc,
		class SymbolBackTranslFunc>
	AutDescription dumpToAutDescSymbolic(
		StateBackTranslFunc           /* stateBackTranslator */,
		SymbolBackTranslFunc          /* symbolTranslator */) const
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
		const SymbolType&       symbol,
		const StateType&        parent);


#if 0
	void AddSimplyTransition(
		const StateTuple&     children,
		SymbolType            symbol,
		const StateType&      parent);
#endif


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
		class StateTranslFunc,
		class SymbolTranslFunc>
	void LoadFromAutDescWithStateSymbolTransl(
		const AutDescription&      desc,
		StateTranslFunc            stateTransl,
		SymbolTranslFunc           symbolTransl,
		const std::string&         params = "")
	{
		if (params == "symbolic")
		{
			this->loadFromAutDescSymbolic(desc, stateTransl, symbolTransl);
		}
		else
		{
			this->loadFromAutDescExplicit(desc, stateTransl, symbolTransl);
		}
	}


	void LoadFromAutDesc(
		const AutDescription&         desc,
		StateDict&                    stateDict,
		const std::string&            params = "");


	void LoadFromAutDesc(
		const AutDescription&         desc,
		StateDict&                    stateDict,
		SymbolDict&                   symbolDict,
		const std::string&            params = "");


	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		StateDict&                       stateDict,
		const std::string&               params = "");


	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		StateDict&                       stateDict,
		SymbolDict&                      symbolDict,
		const std::string&               params = "");


	template <
		class StateTranslFunc,
		class SymbolTranslFunc>
	void LoadFromStringWithStateSymbolTransl(
		VATA::Parsing::AbstrParser&       parser,
		const std::string&                str,
		StateTranslFunc                   stateTransl,
		SymbolTranslFunc                  symbolTransl,
		const std::string&                params = "")
	{
		this->LoadFromAutDescWithStateSymbolTransl(
			parser.ParseString(str),
			stateTransl,
			symbolTransl,
			params);
	}


	template <class SymbolTranslFunc>
	void LoadFromStringWithSymbolTransl(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		StateDict&                       stateDict,
		SymbolTranslFunc                 symbolTransl,
		const std::string&               params)
	{
		StateType state(0);

		this->LoadFromStringWithStateSymbolTransl(
			parser,
			str,
			StringToStateTranslWeak(stateDict,
				[&state](const std::string&){return state++;}),
			symbolTransl,
			params);
	}


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const std::string&                         params = "") const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StateDict&                           stateDict,
		const std::string&                         params = "") const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StateDict&                           stateDict,
		const SymbolDict&                          symbolDict,
		const std::string&                         params = "") const;


	template <
		class StateBackTranslFunc>
	std::string DumpToStringWithStateTransl(
		VATA::Serialization::AbstrSerializer&    serializer,
		StateBackTranslFunc                      stateBackTransl,
		const std::string&                       params = "") const
	{
		return this->DumpToStringWithStateTransl(
			serializer,
			stateBackTransl,
			this->GetSymbolDict(),
			params);
	}


	template <
		class StateBackTranslFunc>
	std::string DumpToStringWithStateTransl(
		VATA::Serialization::AbstrSerializer&    serializer,
		StateBackTranslFunc                      stateBackTransl,
		const SymbolDict&                        symbolDict,
		const std::string&                       params = "") const
	{
		AutDescription desc;
		if (params == "symbolic")
		{
			desc = dumpToAutDescSymbolic(stateBackTransl, symbolDict);
		}
		else
		{
			desc = dumpToAutDescExplicit(stateBackTransl, symbolDict);
		}

		return serializer.Serialize(desc);
	}


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const SymbolDict&                          symbolDict,
		const std::string&                         params = "") const
	{
		return DumpToStringWithStateTransl(
			serializer,
			[](const StateType& state){return Convert::ToString(state);},
			symbolDict,
			params);
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
		BDDTDTreeAutCore&         dstAut,
		StateToStateTranslWeak&   stateTrans) const;


	BDDTDTreeAutCore ReindexStates(
		StateToStateTranslWeak&   stateTrans) const
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
