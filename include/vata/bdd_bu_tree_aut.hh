/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a BDD-based bottom-up tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_BDD_BU_TREE_AUT_HH_
#define _VATA_BDD_BU_TREE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/mtbdd/apply1func.hh>
#include <vata/mtbdd/apply2func.hh>
#include <vata/mtbdd/ondriks_mtbdd.hh>
#include <vata/mtbdd/void_apply2func.hh>
#include <vata/util/bdd_bu_trans_table.hh>
#include <vata/util/ord_vector.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>

namespace VATA { class BDDBottomUpTreeAut; }

GCC_DIAG_OFF(effc++)
class VATA::BDDBottomUpTreeAut
	: public SymbolicAutBase
{
GCC_DIAG_ON(effc++)

	friend BDDBottomUpTreeAut Union(const BDDBottomUpTreeAut&,
		const BDDBottomUpTreeAut&, AutBase::StateToStateMap*,
		AutBase::StateToStateMap*);

	friend BDDBottomUpTreeAut Intersection(const BDDBottomUpTreeAut&,
		const BDDBottomUpTreeAut&, AutBase::ProductTranslMap*);

	friend BDDBottomUpTreeAut RemoveUnreachableStates(const BDDBottomUpTreeAut&,
		AutBase::StateToStateMap* pTranslMap);

	friend BDDBottomUpTreeAut RemoveUselessStates(const BDDBottomUpTreeAut&,
		AutBase::StateToStateMap* pTranslMap);

	friend bool CheckInclusion(const BDDBottomUpTreeAut&,
		const BDDBottomUpTreeAut&);

	friend bool CheckInclusionNoUseless(const BDDBottomUpTreeAut&,
		const BDDBottomUpTreeAut&);

public:   // data types

	typedef std::unordered_set<StateType> StateHT;
	typedef VATA::Util::OrdVector<StateType> StateSet;
	typedef std::vector<StateType> StateTuple;
	typedef VATA::Util::OrdVector<StateTuple> StateTupleSet;
	typedef VATA::MTBDDPkg::OndriksMTBDD<StateSet> TransMTBDD;

	typedef VATA::Util::TranslatorStrict<StringToSymbolDict> SymbolTranslatorStrict;
	typedef VATA::Util::TranslatorStrict<typename StringToSymbolDict::MapBwdType>
		SymbolBackTranslatorStrict;

private:  // data types

	typedef size_t MTBDDHandle;
	typedef VATA::MTBDDPkg::OndriksMTBDD<bool> BDD;

	typedef VATA::Util::BDDBottomUpTransTable<MTBDDHandle, StateSet> TransTable;
	typedef std::shared_ptr<TransTable> TransTablePtr;

	typedef typename TransTable::TupleMap TupleMap;

	typedef VATA::Util::AutDescription AutDescription;


	typedef VATA::Util::Convert Convert;

	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class UnionApplyFunctor :
		public VATA::MTBDDPkg::Apply2Functor<UnionApplyFunctor, StateSet,
		StateSet, StateSet>
	{
	GCC_DIAG_ON(effc++)
	public:   // methods

		inline StateSet ApplyOperation(const StateSet& lhs, const StateSet& rhs)
		{
			return lhs.Union(rhs);
		}
	};

private:  // constants

	static const MTBDDHandle DEFAULT_HANDLE = 0;

private:  // data members

	StateHT finalStates_;
	TransTablePtr transTable_;
	TransMTBDD nullaryMtbdd_;

private:  // methods

	bool isValid() const
	{
		if (transTable_.get() == nullptr)
		{	// in case the transition table pointer is bad
			return false;
		}

		return true;
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

		for (auto trans : desc.transitions)
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

	template <class StateBackTransFunc>
	inline void dumpBddToAutDescExplicit(AutDescription& desc,
		const StateTuple& tuple, const TransMTBDD& bdd,
		StateBackTransFunc stateBackTranslator) const
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

		std::vector<std::string> tupleStr;
		CondColApplyFunctor collector;

		for (auto state : tuple)
		{
			std::string stateStr = stateBackTranslator(state);

			tupleStr.push_back(stateStr);
			desc.states.insert(stateStr);
		}

		for (auto sym : GetSymbolDict())
		{	// iterate over all known symbols
			const std::string& symbol = sym.first;
			BDD symbolBdd(sym.second, true, false);

			collector.Clear();
			collector(bdd, symbolBdd);

			for (auto state : collector.GetAccumulator())
			{	// for each state tuple for which there is a transition
				std::string stateStr = stateBackTranslator(state);

				desc.transitions.insert(AutDescription::Transition(tupleStr, symbol,
					stateStr));
			}
		}
	}

	template <class StateBackTransFunc, class SymbolBackTransFunc>
	AutDescription dumpToAutDescExplicit(
		StateBackTransFunc stateBackTranslator,
		SymbolBackTransFunc /* symbolBackTranslator */) const
	{
		AutDescription desc;

		// copy final states
		for (auto fst : finalStates_)
		{	// copy final states
			desc.finalStates.insert(stateBackTranslator(fst));
		}

		// for the nullary transition
		dumpBddToAutDescExplicit(desc, StateTuple(),
			nullaryMtbdd_, stateBackTranslator);

		for (auto tupleHandlePair : transTable_->GetTupleMap())
		{	// for regular transitions
			dumpBddToAutDescExplicit(desc, tupleHandlePair.first,
				tupleHandlePair.second, stateBackTranslator);
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

public:   // methods

	BDDBottomUpTreeAut() :
		finalStates_(),
		transTable_(new TransTable),
		nullaryMtbdd_(StateSet())
	{
		// Assertions
		assert(isValid());
	}

	BDDBottomUpTreeAut(TransTablePtr transTable) :
		finalStates_(),
		transTable_(transTable),
		nullaryMtbdd_(StateSet())
	{
		// Assertions
		assert(isValid());
	}

	BDDBottomUpTreeAut(const BDDBottomUpTreeAut& aut) :
		finalStates_(aut.finalStates_),
		transTable_(aut.transTable_),
		nullaryMtbdd_(aut.nullaryMtbdd_)
	{
		// Assertions
		assert(isValid());
	}

	BDDBottomUpTreeAut(BDDBottomUpTreeAut&& aut) :
		finalStates_(std::move(aut.finalStates_)),
		transTable_(std::move(aut.transTable_)),
		nullaryMtbdd_(std::move(aut.nullaryMtbdd_))
	{
		// Assertions
		assert(isValid());
	}

	BDDBottomUpTreeAut& operator=(const BDDBottomUpTreeAut& rhs)
	{
		if (this != &rhs)
		{
			finalStates_ = rhs.finalStates_;
			transTable_ = rhs.transTable_;
			nullaryMtbdd_ = rhs.nullaryMtbdd_;

			// Assertions
			assert(isValid());
		}

		return *this;
	}

	BDDBottomUpTreeAut& operator=(BDDBottomUpTreeAut&& rhs)
	{
		if (this != &rhs)
		{
			finalStates_ = std::move(rhs.finalStates_);
			transTable_ = std::move(rhs.transTable_);
			nullaryMtbdd_ = std::move(rhs.nullaryMtbdd_);
		}

		return *this;
	}

	inline TransTablePtr& GetTransTable() const
	{
		return const_cast<TransTablePtr&>(transTable_);
	}


	void AddTransition(const StateTuple& children,
		SymbolType symbol, const StateType& parent)
	{
		// Assertions
		assert(isValid());
		assert(symbol.length() == SYMBOL_SIZE);

		if (transTable_.unique())
		{
			UnionApplyFunctor unioner;

			const TransMTBDD& oldMtbdd = GetMtbdd(children);
			TransMTBDD addedMtbdd(symbol, StateSet(parent), StateSet());
			SetMtbdd(children, unioner(oldMtbdd, addedMtbdd));
		}
		else
		{	// copy on write
			assert(false);
		}

		UnionApplyFunctor unioner;

		const TransMTBDD& oldMtbdd = GetMtbdd(children);
		TransMTBDD addedMtbdd(symbol, StateSet(parent), StateSet());

		// TODO: this could be done better
		SetMtbdd(children, unioner(oldMtbdd, addedMtbdd));

		assert(isValid());
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

	inline const StateHT& GetFinalStates() const
	{
		// Assertions
		assert(isValid());

		return finalStates_;
	}

	inline const TupleMap& GetTuples() const
	{
		return transTable_->GetTupleMap();
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

	std::string DumpToDot() const
	{
		std::vector<const TransMTBDD*> tupleVec;
		for (auto tupleHandlePair : transTable_->GetTupleMap())
		{
			tupleVec.push_back(&tupleHandlePair.second);
		}

		return TransMTBDD::DumpToDot(tupleVec);
	}

	BDDTopDownTreeAut GetTopDownAut(StateToStateMap* pTranslMap = nullptr) const;


	template <class OperationFunc>
	static void ForeachUpSymbolFromTupleAndTupleSetDo(
		const BDDBottomUpTreeAut& lhs, const BDDBottomUpTreeAut& rhs,
		const StateTuple& lhsTuple, const StateTupleSet& rhsTupleSet,
		OperationFunc& opFunc)
	{
		// Assertions
		assert(lhs.isValid());
		assert(rhs.isValid());

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

	inline const TransMTBDD& GetMtbdd(const StateTuple& children) const
	{
		// Assertions
		assert(isValid());

		if (children.empty())
		{
			return nullaryMtbdd_;
		}
		else
		{
			return transTable_->GetMtbdd(children);
		}
	}

	inline void SetMtbdd(const StateTuple& children, const TransMTBDD& mtbdd)
	{
		// Assertions
		assert(isValid());

		if (children.empty())
		{
			nullaryMtbdd_ = mtbdd;;
		}
		else
		{
			transTable_->SetMtbdd(children, mtbdd);
		}
	}

	inline TransMTBDD ReindexStates(BDDBottomUpTreeAut& dstAut,
		StateToStateTranslator& stateTrans) const
	{
		GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
		class RewriteApplyFunctor :
			public VATA::MTBDDPkg::Apply1Functor<RewriteApplyFunctor,
			StateSet, StateSet>
		{
		GCC_DIAG_ON(effc++)
		private:  // data members

			StateToStateTranslator trans_;

		public:   // methods

			RewriteApplyFunctor(StateToStateTranslator& trans) :
				trans_(trans)
			{ }

			inline StateSet ApplyOperation(const StateSet& value)
			{
				StateSet result;

				for (const StateType& state : value)
				{ // for every state
					result.insert(trans_(state));
				}

				return result;
			}
		};

		RewriteApplyFunctor rewriter(stateTrans);
		for (auto tupleBddPair : GetTuples())
		{
			const StateTuple& oldTuple = tupleBddPair.first;
			StateTuple newTuple;
			for (StateTuple::const_iterator itTup = oldTuple.begin();
				itTup != oldTuple.end(); ++itTup)
			{
				newTuple.push_back(stateTrans(*itTup));
			}
			assert(newTuple.size() == oldTuple.size());

			dstAut.SetMtbdd(newTuple, rewriter(tupleBddPair.second));
		}

		return rewriter(GetMtbdd(StateTuple()));
	}
};

#endif
