/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the core of an BDD-based bottom-up tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_BU_TREE_AUT_CORE_HH_
#define _VATA_BDD_BU_TREE_AUT_CORE_HH_

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/util/aut_description.hh>
#include <vata/util/convert.hh>

// MTBDD headers
#include "mtbdd/apply1func.hh"
#include "mtbdd/apply2func.hh"
#include "mtbdd/ondriks_mtbdd.hh"
#include "mtbdd/void_apply2func.hh"

#include "util/bdd_bu_trans_table.hh"
#include "bdd_bu_tt_wrapper.hh"


GCC_DIAG_OFF(effc++)
class VATA::BDDBUTreeAutCore : public SymbolicAutBase
{
GCC_DIAG_ON(effc++)

public:   // data types

	using AutDescription  = VATA::Util::AutDescription;

	using StateType       = VATA::BDDBottomUpTreeAut::StateType;
	using StateSet        = VATA::Util::OrdVector<StateType>;
	using StateTuple      = VATA::BDDBottomUpTreeAut::StateTuple;
	using StateTupleSet   = VATA::Util::OrdVector<StateTuple>;
	using StateHT         = VATA::BDDBottomUpTreeAut::StateHT;

	using MTBDDHandle     = size_t;
	using TransTable      = VATA::Util::TransTableWrapper<MTBDDHandle, StateSet>;
	using TransMTBDD      = TransTable::TransMTBDD;

private:  // data types

	using BDD             = VATA::MTBDDPkg::OndriksMTBDD<bool>;

	using StateToStateTranslator      = VATA::AutBase::StateToStateTranslator;
	using StringToStateDict           = VATA::AutBase::StringToStateDict;
	using SymbolBackTranslatorStrict  =
		VATA::BDDBottomUpTreeAut::SymbolBackTranslatorStrict;

	using Convert       = VATA::Util::Convert;


	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class UnionApplyFunctor :
		public VATA::MTBDDPkg::Apply2Functor<UnionApplyFunctor, StateSet,
		StateSet, StateSet>
	{
	GCC_DIAG_ON(effc++)
	public:   // methods

		StateSet ApplyOperation(const StateSet& lhs, const StateSet& rhs)
		{
			return lhs.Union(rhs);
		}
	};

private:  // constants

	static const MTBDDHandle DEFAULT_HANDLE = 0;

private:  // data members

	TransTable transTable_;
	StateHT finalStates_;

private:  // methods


	template <
		class StateTransFunc,
		class SymbolTransFunc
		>
	void loadFromAutDescSymbolic(
		const AutDescription&     /* desc */,
		StateTransFunc            /* stateTranslator */,
		SymbolTransFunc           /* symbolTranslator */)
	{
		throw NotImplementedException(__func__);
	}


	template <
		class StateBackTransFunc,
		class SymbolTransFunc>
	AutDescription dumpToAutDescSymbolic(
		StateBackTransFunc   /* stateBackTranslator */,
		SymbolTransFunc      /* symbolTranslator */) const
	{
		throw NotImplementedException(__func__);
	}


	template <
		class StateBackTransFunc,
		class SymbolBackTransFunc
		>
	AutDescription dumpToAutDescExplicit(
		StateBackTransFunc        stateBackTranslator,
		SymbolBackTransFunc       /* symbolBackTranslator */) const
	{
		GCC_DIAG_OFF(effc++)
		class CondColApplyFunctor :
			public VATA::MTBDDPkg::VoidApply2Functor<CondColApplyFunctor,
			StateSet, bool>
		{
		GCC_DIAG_ON(effc++)

		public:   // data types

			typedef std::list<StateType> AccumulatorType;

		private:  // data members

			AccumulatorType accumulator_;

		public:

			CondColApplyFunctor() :
				accumulator_()
			{ }

			inline const AccumulatorType& GetAccumulator() const
			{
				return accumulator_;
			}

			inline void Clear()
			{
				accumulator_.clear();
			}

			inline void ApplyOperation(const StateSet& lhs, const bool& rhs)
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
		for (auto tupleBddPair : GetTransTable())
		{	// for all states
			const StateTuple& children = tupleBddPair.first;

			std::vector<std::string> tupleStr;

			for (const StateType& state : children)
			{
				std::string stateStr = stateBackTranslator(state);

				tupleStr.push_back(stateStr);
				desc.states.insert(stateStr);
			}

			const TransMTBDD& transMtbdd = tupleBddPair.second;

			for (auto sym : this->GetSymbolDict())
			{	// iterate over all known symbols
				const std::string& symbol = sym.first;
				BDD symbolBdd(sym.second, true, false);

				collector.Clear();
				collector(transMtbdd, symbolBdd);

				for (const StateType& state : collector.GetAccumulator())
				{	// for each state tuple for which there is a transition
					std::string stateStr = stateBackTranslator(state);

					desc.transitions.insert(AutDescription::Transition(tupleStr, symbol,
						stateStr));
				}
			}
		}

		return desc;
	}


	template <
		class StateTransFunc,
		class SymbolTransFunc
		>
	void loadFromAutDescExplicit(
		const AutDescription&      desc,
		StateTransFunc             stateTranslator,
		SymbolTransFunc            symbolTranslator)
	{
		for (const AutDescription::State& fst : desc.finalStates)
		{	// traverse final states
			finalStates_.insert(stateTranslator(fst));
		}

		for (const AutDescription::Transition& trans : desc.transitions)
		{	// traverse the transitions
			const AutDescription::StateTuple& childrenStr = trans.first;
			const std::string& symbolStr = trans.second;
			const AutDescription::State& parentStr = trans.third;

			// translate the parent state
			StateType parent = stateTranslator(parentStr);

			// translate children
			StateTuple children;
			for (auto tupState : childrenStr)
			{	// for all children states
				children.push_back(stateTranslator(tupState));
			}

			// translate the symbol
			SymbolType symbol = symbolTranslator(symbolStr);

			this->AddTransition(children, symbol, parent);
		}
	}


public:   // methods

	BDDBUTreeAutCore() :
		transTable_(),
		finalStates_()
	{ }


	BDDBUTreeAutCore(
		const BDDBUTreeAutCore&           aut);


	BDDBUTreeAutCore(
		BDDBUTreeAutCore&&                aut);


	explicit BDDBUTreeAutCore(
		const TransTable&                 transTable);


	BDDBUTreeAutCore& operator=(
		const BDDBUTreeAutCore&         rhs);


	BDDBUTreeAutCore& operator=(
		BDDBUTreeAutCore&&              rhs);


	const TransTable& GetTransTable() const
	{
		return transTable_;
	}

	const TransMTBDD& GetMtbdd(
		const StateTuple&          children) const
	{
		return transTable_.GetMtbdd(children);
	}

	void SetMtbdd(
		const StateTuple&      children,
		const TransMTBDD&      mtbdd)
	{
		transTable_.SetMtbdd(children, mtbdd);
	}

	static bool ShareTransTable(
		const BDDBUTreeAutCore&       lhs,
		const BDDBUTreeAutCore&       rhs)
	{
		return lhs.transTable_.GetTable() == rhs.transTable_.GetTable();
	}

	template <class OperationFunc>
	static void ForeachUpSymbolFromTupleAndTupleSetDo(
		const BDDBUTreeAutCore&     lhs,
		const BDDBUTreeAutCore&     rhs,
		const StateTuple&           lhsTuple,
		const StateTupleSet&        rhsTupleSet,
		OperationFunc&              opFunc)
	{
		GCC_DIAG_OFF(effc++)
		class OperationApplyFunctor :
			public VATA::MTBDDPkg::VoidApply2Functor<OperationApplyFunctor,
			StateSet, StateSet>
		{
		GCC_DIAG_ON(effc++)

		private:  // data members

			OperationFunc& opFunc_;

		public:   // methods

			OperationApplyFunctor(OperationFunc& opFunc) :
				opFunc_(opFunc)
			{ }

			inline void ApplyOperation(const StateSet& lhs, const StateSet& rhs)
			{
				auto AccessElementF = [](const StateType& state){return state;};
				opFunc_(lhs, AccessElementF, rhs, AccessElementF);

				if (opFunc_.IsProcessingStopped())
				{	// in case the operator wants to stop processing
					this->stopProcessing();
				}
			}
		};

		UnionApplyFunctor unioner;
		TransMTBDD rhsUnionMtbdd((StateSet()));

		// collect the RHS's MTBDDs leaves
		for (auto tuple : rhsTupleSet)
		{
			rhsUnionMtbdd = unioner(rhsUnionMtbdd, rhs.GetMtbdd(tuple));
		}

		// create apply functor
		OperationApplyFunctor opApplyFunc(opFunc);

		// perform the apply operation
		opApplyFunc(lhs.GetMtbdd(lhsTuple), rhsUnionMtbdd);
	}


	TransMTBDD ReindexStates(
		BDDBUTreeAutCore&          dstAut,
		StateToStateTranslator&    stateTrans) const;


	BDDBUTreeAutCore ReindexStates(
		StateToStateTranslator&    stateTrans) const;


	template <
		class StateBackTransFunc,
		class SymbolTransFunc
		>
	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		StateBackTransFunc                         stateBackTranslator,
		SymbolTransFunc                            symbolTranslator,
		const std::string&                         params = "") const
	{
		AutDescription desc;
		if ("symbolic" == params)
		{
			desc = dumpToAutDescSymbolic(stateBackTranslator, symbolTranslator);
		}
		else
		{
			desc = dumpToAutDescExplicit(stateBackTranslator, symbolTranslator);
		}

		return serializer.Serialize(desc);
	}

	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StringToStateDict&                   stateDict) const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer) const;


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


	void SetStateFinal(
		const StateType&       state)
	{
		finalStates_.insert(state);
	}


	bool IsStateFinal(
		const StateType&       state) const
	{
		return (finalStates_.cend() != finalStates_.find(state));
	}


	const StateHT& GetFinalStates() const
	{
		return finalStates_;
	}



	void AddTransition(
		const StateTuple&      children,
		SymbolType             symbol,
		const StateType&       parent);


	BDDBUTreeAutCore RemoveUselessStates() const;


	BDDBUTreeAutCore RemoveUnreachableStates() const;


	static BDDBUTreeAutCore Union(
		const BDDBUTreeAutCore&        lhs,
		const BDDBUTreeAutCore&        rhs,
		AutBase::StateToStateMap*      pTranslMapLhs = nullptr,
		AutBase::StateToStateMap*      pTranslMapRhs = nullptr);


	static BDDBUTreeAutCore UnionDisjointStates(
		const BDDBUTreeAutCore&        lhs,
		const BDDBUTreeAutCore&        rhs);


	static BDDBUTreeAutCore Intersection(
		const BDDBUTreeAutCore&        lhs,
		const BDDBUTreeAutCore&        rhs,
		AutBase::ProductTranslMap*     pTranslMap = nullptr);


	static bool CheckInclusion(
		const BDDBUTreeAutCore&     smaller,
		const BDDBUTreeAutCore&     bigger,
		const VATA::InclParam&      params);


	BDDTopDownTreeAut GetTopDownAut() const;

	StateBinaryRelation ComputeDownwardSimulation() const;

	StateBinaryRelation ComputeDownwardSimulation(
		size_t                    size) const;

	StateBinaryRelation ComputeUpwardSimulation() const;

	StateBinaryRelation ComputeUpwardSimulation(
		size_t                    size) const;

	template <
		class StateTransFunc,
		class SymbolTransFunc
		>
	void LoadFromAutDesc(
		const AutDescription&       desc,
		StateTransFunc              stateTranslator,
		SymbolTransFunc             symbolTranslator,
		const std::string&          params = "")
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
		VATA::Parsing::AbstrParser&     parser,
		const std::string&              str,
		StringToStateDict&              stateDict)
	{
		this->LoadFromAutDesc(parser.ParseString(str), stateDict);
	}
};

#endif
