/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a BDD-based top-down tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_TD_TREE_AUT_HH_
#define _VATA_BDD_TD_TREE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/symbolic_aut_base.hh>
#include <vata/mtbdd/apply1func.hh>
#include <vata/mtbdd/apply2func.hh>
#include <vata/mtbdd/ondriks_mtbdd.hh>
#include <vata/mtbdd/void_apply1func.hh>
#include <vata/mtbdd/void_apply2func.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/util/ord_vector.hh>
#include <vata/util/bdd_td_trans_table.hh>
#include <vata/util/vector_map.hh>
#include <vata/util/util.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>

// Standard library headers
#include <cstdint>
#include <unordered_set>

namespace VATA { class BDDTopDownTreeAut; }

GCC_DIAG_OFF(effc++)
class VATA::BDDTopDownTreeAut
	: public SymbolicAutBase
{
GCC_DIAG_ON(effc++)

	friend BDDTopDownTreeAut Union(const BDDTopDownTreeAut&,
		const BDDTopDownTreeAut&, AutBase::StateToStateMap*,
		AutBase::StateToStateMap*);

	friend BDDTopDownTreeAut Intersection(const BDDTopDownTreeAut&,
		const BDDTopDownTreeAut&, AutBase::ProductTranslMap*);

	friend BDDTopDownTreeAut RemoveUnreachableStates(const BDDTopDownTreeAut&);

	friend BDDTopDownTreeAut RemoveUselessStates(const BDDTopDownTreeAut&,
		AutBase::StateToStateMap*);

	friend bool CheckInclusion(const BDDTopDownTreeAut&, const BDDTopDownTreeAut&);

	friend bool CheckInclusionNoUseless(const BDDTopDownTreeAut&,
		const BDDTopDownTreeAut&);

	friend class BDDBottomUpTreeAut;

public:   // data types

	typedef typename SymbolicAutBase::SymbolType SymbolType;
	typedef std::vector<StateType> StateTuple;
	typedef VATA::Util::OrdVector<StateTuple> StateTupleSet;
	typedef StateTupleSet DownInclStateTupleSet;
	typedef const std::vector<StateTuple>& DownInclStateTupleVector;
	typedef std::unordered_set<StateType> StateSet;
	typedef VATA::Util::OrdVector<StateType> StateSetLight;

	typedef VATA::Util::TwoWayDict<std::string, SymbolType> StringToSymbolDict;

	typedef VATA::Util::TranslatorStrict<StringToSymbolDict> SymbolTranslatorStrict;
	typedef VATA::Util::TranslatorStrict<typename StringToSymbolDict::MapBwdType>
		SymbolBackTranslatorStrict;

	typedef VATA::MTBDDPkg::OndriksMTBDD<StateTupleSet> TransMTBDD;

private:  // data types

	typedef VATA::MTBDDPkg::OndriksMTBDD<bool> BDD;

	typedef VATA::Util::BDDTopDownTransTable<StateType, StateTupleSet>
		TransTable;

	typedef std::shared_ptr<TransTable> TransTablePtr;
	typedef typename TransTable::StateMap StateMap;

	typedef VATA::Util::AutDescription AutDescription;

	typedef VATA::Util::Convert Convert;

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

	static const size_t SYMBOL_ARITY_LENGTH = 6;
	static const size_t MAX_SYMBOL_ARITY =
		VATA::Util::IntExp2(SYMBOL_ARITY_LENGTH) - 1;

	static const size_t SYMBOL_TOTAL_SIZE = SYMBOL_SIZE + SYMBOL_ARITY_LENGTH;


private:  // data members

	StateSet finalStates_;
	TransTablePtr transTable_;

private:  // methods

	bool isValid() const
	{
		if (transTable_.get() == nullptr)
		{	// in case the transition table pointer is bad
			return false;
		}

		return true;
	}

	inline const TransMTBDD& GetMtbdd(const StateType& state) const
	{
		// Assertions
		assert(isValid());

		return transTable_->GetMtbdd(state);
	}

	inline void SetMtbdd(const StateType& state, const TransMTBDD& mtbdd)
	{
		// Assertions
		assert(isValid());

		transTable_->SetMtbdd(state, mtbdd);
	}

	template <class StateTransFunc, class SymbolTransFunc>
	void loadFromAutDescExplicit(const AutDescription& desc,
		StateTransFunc stateTranslator, SymbolTransFunc symbolTranslator)
	{
		for (auto fst : desc.finalStates)
		{	// traverse final states
			finalStates_.insert(stateTranslator(fst));
		}

		assert(isValid());

		for (auto tr : desc.transitions)
		{	// traverse the transitions
			const AutDescription::StateTuple& childrenStr = tr.first;
			const std::string& symbolStr = tr.second;
			const AutDescription::State& parentStr = tr.third;

			// translate the parent state
			StateType parent = stateTranslator(parentStr);

			// translate children
			StateTuple children;
			for (auto tupSt : childrenStr)
			{	// for all children states
				children.push_back(stateTranslator(tupSt));
			}

			// translate the symbol
			SymbolType symbol = symbolTranslator(symbolStr);

			AddTransition(children, symbol, parent);
			assert(isValid());
		}

		assert(isValid());
	}

	template <class StateTransFunc, class SymbolTransFunc>
	void loadFromAutDescSymbolic(const AutDescription&/* desc */,
		StateTransFunc /* stateTranslator */, SymbolTransFunc /* symbolTranslator */)
	{
		assert(false);

		assert(isValid());
	}

	template <class StateBackTransFunc, class SymbolBackTransFunc>
	AutDescription dumpToAutDescExplicit(StateBackTransFunc stateBackTranslator,
		SymbolBackTransFunc /* symbolTranslator */) const
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

			inline const AccumulatorType& GetAccumulator() const
			{
				return accumulator_;
			}

			inline void Clear()
			{
				accumulator_.clear();
			}

			inline void ApplyOperation(const StateTupleSet& lhs, const bool& rhs)
			{
				if (rhs)
				{
					accumulator_.insert(accumulator_.end(), lhs.begin(), lhs.end());
				}
			}
		};

		AutDescription desc;

		// copy final states
		for (auto fst : finalStates_)
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

	template <class StateBackTransFunc, class SymbolTransFunc>
	AutDescription dumpToAutDescSymbolic(
		StateBackTransFunc /* stateBackTranslator */,
		SymbolTransFunc /* symbolTranslator */) const
	{
		throw std::runtime_error("Unimplemented");
	}

	inline void addArityToSymbol(SymbolType& symbol, size_t arity) const
	{
		// Assertions
		assert(arity <= MAX_SYMBOL_ARITY);

		SymbolType prefix(SYMBOL_ARITY_LENGTH, arity);

		symbol.append(prefix);
	}


public:   // public methods

	BDDTopDownTreeAut() :
		finalStates_(),
		transTable_(new TransTable)
	{
		// Assertions
		assert(isValid());
	}

	BDDTopDownTreeAut(TransTablePtr transTable) :
		finalStates_(),
		transTable_(transTable)
	{
		// Assertions
		assert(isValid());
	}

	BDDTopDownTreeAut(const BDDTopDownTreeAut& aut) :
		finalStates_(aut.finalStates_),
		transTable_(aut.transTable_)
	{
		// Assertions
		assert(isValid());
	}

	BDDTopDownTreeAut& operator=(const BDDTopDownTreeAut& rhs)
	{
		if (this != &rhs)
		{
			transTable_ = rhs.transTable_;
			finalStates_ = rhs.finalStates_;
		}

		assert(isValid());
		return *this;
	}

	inline const StateMap& GetStates() const
	{
		return transTable_->GetStateMap();
	}

	inline const StateSet& GetFinalStates() const
	{
		return finalStates_;
	}

	inline TransTablePtr& GetTransTable() const
	{
		return const_cast<TransTablePtr&>(transTable_);
	}

	void LoadFromString(VATA::Parsing::AbstrParser& parser, const std::string& str,
		StringToStateDict& stateDict)
	{
		typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
			StateTranslator;
		typedef VATA::Util::TranslatorWeak<StringToSymbolDict>
			SymbolTranslator;

		StateType stateCnt = 0;

		LoadFromString(parser, str,
			StateTranslator(stateDict,
				[&stateCnt](const std::string&){return stateCnt++;}),
			SymbolTranslator(GetSymbolDict(),
				[this](const std::string&){return AddSymbol();}));
	}

	template <class SymbolTransFunc>
	void LoadFromString(VATA::Parsing::AbstrParser& parser, const std::string& str,
		SymbolTransFunc symbolTranslator)
	{
		typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
			StateTranslator;

		AutBase::StringToStateDict dict;

		LoadFromString(parser, str, StateTranslator(dict, *this), symbolTranslator);
	}

	template <class StateTransFunc, class SymbolTransFunc>
	void LoadFromString(VATA::Parsing::AbstrParser& parser, const std::string& str,
		StateTransFunc stateTranslator, SymbolTransFunc symbolTranslator,
		const std::string& params = "")
	{
		if (params == "symbolic")
		{
			loadFromAutDescSymbolic(parser.ParseString(str), stateTranslator,
				symbolTranslator);
		}
		else
		{
			loadFromAutDescExplicit(parser.ParseString(str), stateTranslator,
				symbolTranslator);
		}

		assert(isValid());
	}

	template <class StateBackTransFunc, class SymbolTransFunc>
	std::string DumpToString(VATA::Serialization::AbstrSerializer& serializer,
		StateBackTransFunc stateBackTranslator, SymbolTransFunc symbolTranslator,
		const std::string& params = "") const
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
	std::string DumpToString(VATA::Serialization::AbstrSerializer& serializer,
		SymbolTransFunc symbolTranslator,
		const std::string& params = "") const
	{
		return DumpToString(serializer,
			[](const StateType& state){return Convert::ToString(state);},
			symbolTranslator, params);
	}

	inline void SetStateFinal(const StateType& state)
	{
		// Assertions
		assert(isValid());

		finalStates_.insert(state);
	}

	inline bool IsStateFinal(const StateType& state) const
	{
		// Assertions
		assert(isValid());

		return finalStates_.find(state) != finalStates_.end();
	}

	void AddTransition(const StateTuple& children, SymbolType symbol,
		const StateType& parent)
	{
		// Assertions
		assert(symbol.length() == SYMBOL_SIZE);

		addArityToSymbol(symbol, children.size());
		assert(symbol.length() == SYMBOL_TOTAL_SIZE);

		if (transTable_.unique())
		{
			UnionApplyFunctor unioner;

			const TransMTBDD& oldMtbdd = GetMtbdd(parent);
			TransMTBDD addedMtbdd(symbol, StateTupleSet(children), StateTupleSet());
			SetMtbdd(parent, unioner(oldMtbdd, addedMtbdd));
		}
		else
		{	// copy on write
			assert(false);
		}
	}

	void AddSimplyTransition(const StateTuple& children, SymbolType symbol,
		const StateType& parent);

	template <class OperationFunc>
	static void ForeachDownSymbolFromStateAndStateSetDo(
		const BDDTopDownTreeAut& lhs, const BDDTopDownTreeAut& rhs,
		const StateType& lhsState, const StateSetLight& rhsSet,
		OperationFunc& opFunc)
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

	std::string DumpToDot() const
	{
		std::vector<const TransMTBDD*> stateVec;
		for (auto stateBddPair : GetStates())
		{
			stateVec.push_back(&GetMtbdd(stateBddPair.first));
		}

		return TransMTBDD::DumpToDot(stateVec);
	}

	inline static DownInclStateTupleVector StateTupleSetToVector(
		const DownInclStateTupleSet& tupleSet)
	{
		return tupleSet.ToVector();
	}

	inline void ReindexStates(BDDTopDownTreeAut& dstAut,
		StateToStateTranslator& stateTrans) const
	{
		GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
		class RewriteApplyFunctor :
			public VATA::MTBDDPkg::Apply1Functor<RewriteApplyFunctor,
			StateTupleSet, StateTupleSet>
		{
		GCC_DIAG_ON(effc++)
		private:  // data members

			StateToStateTranslator trans_;

		public:   // methods

			RewriteApplyFunctor(StateToStateTranslator& trans) :
				trans_(trans)
			{ }

			inline StateTupleSet ApplyOperation(const StateTupleSet& value)
			{
				StateTupleSet result;

				for (const StateTuple& tuple : value)
				{ // for every tuple
					StateTuple resTuple;
					for (StateTuple::const_iterator itTup = tuple.begin();
						itTup != tuple.end(); ++itTup)
					{
						resTuple.push_back(trans_(*itTup));
					}

					result.insert(resTuple);
				}

				return result;
			}
		};

		RewriteApplyFunctor rewriter(stateTrans);
		for (auto stateBddPair : GetStates())
		{
			StateType newState = stateTrans(stateBddPair.first);
			dstAut.SetMtbdd(newState, rewriter(stateBddPair.second));
		}
	}
};

#endif
