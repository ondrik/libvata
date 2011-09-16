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
#include <vata/aut_base.hh>
#include <vata/aut_op.hh>
#include <vata/mtbdd/apply2func.hh>
#include <vata/mtbdd/ondriks_mtbdd.hh>
#include <vata/mtbdd/void_apply1func.hh>
#include <vata/mtbdd/void_apply2func.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/util/ord_vector.hh>
#include <vata/util/td_bdd_trans_table.hh>
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
	friend Aut RemoveUselessStates(const Aut&,
		AutBase::StateToStateMap* pTranslMap);

	template <class Aut>
	friend bool CheckInclusion(const Aut&, const Aut&);

	template <class Aut>
	friend bool CheckInclusionNoUseless(const Aut&, const Aut&);

public:   // data types

	typedef VATA::MTBDDPkg::VarAsgn SymbolType;
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

private:  // data types


	typedef VATA::MTBDDPkg::OndriksMTBDD<StateTupleSet> TransMTBDD;
	typedef VATA::MTBDDPkg::OndriksMTBDD<bool> BDD;

	typedef VATA::Util::TDBDDTransTable<StateType, VATA::Util::OrdVector>
		TransTable;

	typedef std::shared_ptr<TransTable> TransTablePtr;

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

	static const size_t SYMBOL_VALUE_LENGTH = 16;

	static const size_t SYMBOL_ARITY_LENGTH = 6;
	static const size_t MAX_SYMBOL_ARITY =
		VATA::Util::IntExp2(SYMBOL_ARITY_LENGTH) - 1;

	static const size_t SYMBOL_SIZE = SYMBOL_VALUE_LENGTH + SYMBOL_ARITY_LENGTH;


private:  // data members


	StateSet states_;
	StateSet finalStates_;
	TransTablePtr transTable_;

	static StringToSymbolDict* pSymbolDict_;
	static SymbolType* pNextBaseSymbol_;

private:  // methods

	bool isValid() const;

	void copyStates(const BDDTopDownTreeAut& src);

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

	template <class StateTransFunc, class SymbolTransFunc>
	void loadFromAutDescExplicit(const AutDescription& desc,
		StateTransFunc stateTranslator, SymbolTransFunc symbolTranslator)
	{
		// Assertions
		assert(hasEmptyStateSet());

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

			AddSimplyTransition(children, symbol, parent);
			assert(isValid());
		}

		assert(isValid());
	}

	template <class StateTransFunc, class SymbolTransFunc>
	void loadFromAutDescSymbolic(const AutDescription&/* desc */,
		StateTransFunc /* stateTranslator */, SymbolTransFunc /* symbolTranslator */)
	{
		// Assertions
		assert(hasEmptyStateSet());

		assert(false);

		assert(isValid());
	}

	template <class StateBackTransFunc, class SymbolBackTransFunc>
	AutDescription dumpToAutDescExplicit(StateBackTransFunc stateBackTranslator,
		SymbolBackTransFunc /* symbolTranslator */) const
	{
		// Assertions
		assert(pSymbolDict_ != nullptr);

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
		for (auto state : states_)
		{	// for all states
			std::string stateStr;

			// copy the state
			stateStr = stateBackTranslator(state);

			desc.states.insert(stateStr);

			const TransMTBDD& transMtbdd = getMtbdd(state);

			for (auto sym : *pSymbolDict_)
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

	BDDTopDownTreeAut() :
		states_(),
		finalStates_(),
		transTable_(new TransTable)
	{
		// Assertions
		assert(isValid());
	}

	BDDTopDownTreeAut(TransTablePtr transTable) :
		states_(),
		finalStates_(),
		transTable_(transTable)
	{
		// Assertions
		assert(isValid());
	}

	BDDTopDownTreeAut(const BDDTopDownTreeAut& aut);
	BDDTopDownTreeAut& operator=(const BDDTopDownTreeAut& rhs);

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

	void LoadFromString(VATA::Parsing::AbstrParser& parser, const std::string& str,
		StringToStateDict& stateDict)
	{
		typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
			StateTranslator;
		typedef VATA::Util::TranslatorWeak<StringToSymbolDict>
			SymbolTranslator;

		LoadFromString(parser, str,
			StateTranslator(stateDict, [this]{return this->AddState();}),
			SymbolTranslator(GetSymbolDict(), [this]{return this->AddSymbol();}));
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
		// Assertions
		assert(hasEmptyStateSet());

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

	void AddTransition(const StateTuple& children, const SymbolType& symbol,
		const StateType& state);

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
			rhsUnionMtbdd = unioner(rhsUnionMtbdd, rhs.getMtbdd(rhsState));
		}

		// create apply functor
		OperationApplyFunctor opApplyFunc(opFunc);

		// perform the apply operation
		opApplyFunc(lhs.getMtbdd(lhsState), rhsUnionMtbdd);
	}

	std::string DumpToDot() const
	{
		std::vector<const TransMTBDD*> stateVec;
		for (const StateType& state : states_)
		{
			stateVec.push_back(&getMtbdd(state));
		}

		return TransMTBDD::DumpToDot(stateVec);
	}

	inline SymbolType AddSymbol()
	{
		// Assertions
		assert(pNextBaseSymbol_ != nullptr);

		return (*pNextBaseSymbol_)++;
	}

	inline StringToSymbolDict& GetSymbolDict()
	{
		// Assertions
		assert(pSymbolDict_ != nullptr);

		return *pSymbolDict_;
	}

	inline const StringToSymbolDict& GetSymbolDict() const
	{
		// Assertions
		assert(pSymbolDict_ != nullptr);

		return *pSymbolDict_;
	}

	inline static void SetSymbolDictPtr(StringToSymbolDict* pSymbolDict)
	{
		// Assertions
		assert(pSymbolDict != nullptr);

		pSymbolDict_ = pSymbolDict;
	}

	inline static void SetNextSymbolPtr(SymbolType* pNextBaseSymbol)
	{
		// Assertions
		assert(pNextBaseSymbol != nullptr);

		pNextBaseSymbol_ = pNextBaseSymbol;
	}

	inline static DownInclStateTupleVector StateTupleSetToVector(
		const DownInclStateTupleSet& tupleSet)
	{
		return tupleSet.ToVector();
	}

	~BDDTopDownTreeAut();
};

#endif
