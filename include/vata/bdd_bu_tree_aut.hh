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

	typedef VATA::Util::AutDescription AutDescription;


	typedef VATA::Util::Convert Convert;

	class TransTableWrapper
	{
	private:  // data types

		typedef VATA::Util::BDDBottomUpTransTable<MTBDDHandle, StateSet> Table;

		typedef typename Table::TupleMap::key_type key_type;
		typedef typename Table::TupleMap::mapped_type mapped_type;
		typedef typename Table::TupleMap::value_type value_type;

		typedef std::shared_ptr<Table> TablePtr;

		template <class TupleMapIterator>
		class generic_iterator
		{
		private:  // data members

			bool isNullary_;
			const TransTableWrapper& tableWrap_;
			TupleMapIterator itTupleMap_;

		public:   // methods

			inline bool operator==(const generic_iterator& rhs) const
			{
				bool match = isNullary_ == rhs.isNullary_;
				return match && (isNullary_ || (itTupleMap_ == rhs.itTupleMap_));
			}

			inline bool operator!=(const generic_iterator& rhs) const
			{
				return !operator==(rhs);
			}

			inline generic_iterator& operator++()
			{
				if (isNullary_)
				{
					isNullary_ = false;
				}
				else
				{
					++itTupleMap_;
				}

				return *this;
			}

		protected:// methods

			explicit generic_iterator(const TransTableWrapper& tableWrap, bool isBegin) :
				isNullary_(isBegin),
				tableWrap_(tableWrap),
				itTupleMap_(isBegin? tableWrap.table_->GetTupleMap().begin() :
					tableWrap.table_->GetTupleMap().end())
			{ }

#if 0
			inline value_type refOp()
			{
				if (isNullary_)
				{
					assert(false);
				}
				else
				{
					return *itTupleMap_;
				}
			}
#endif

			inline value_type constRefOp() const
			{
				if (isNullary_)
				{
					return std::make_pair(StateTuple(), tableWrap_.nullaryMtbdd_);
				}
				else
				{
					return *itTupleMap_;
				}
			}

		};

	public:   // data types

#if 0
		GCC_DIAG_OFF(effc++)
		class iterator :
			public generic_iterator<typename Table::TupleMap::iterator>
		{
		GCC_DIAG_ON(effc++)

		public:   // methods

			iterator(const typename Table::TupleMap::iterator& iter, bool isNullary) :
				generic_iterator(iter, isNullary)
			{ }

			inline value_type& operator*()
			{
				return refOp();
			}
		};
#endif

		GCC_DIAG_OFF(effc++)
		class const_iterator :
			public generic_iterator<typename Table::TupleMap::const_iterator>
		{
		GCC_DIAG_ON(effc++)

		public:   // methods

			const_iterator(const TransTableWrapper& tableWrap, bool isBegin) :
				generic_iterator(tableWrap, isBegin)
			{ }

			inline value_type operator*() const
			{
				return constRefOp();
			}
		};

	private:  // data members

		TablePtr table_;
		TransMTBDD nullaryMtbdd_;

	public:   // methods

		inline TransTableWrapper() :
			table_(new Table),
			nullaryMtbdd_(StateSet())
		{ }

		inline explicit TransTableWrapper(TablePtr& table) :
			table_(table),
			nullaryMtbdd_(StateSet())
		{ }

		inline TransTableWrapper(TablePtr& table, const TransMTBDD& nullaryMtbdd) :
			table_(table),
			nullaryMtbdd_(nullaryMtbdd)
		{ }

		inline const TransMTBDD& GetMtbdd(const StateTuple& children) const
		{
			if (children.empty())
			{
				return nullaryMtbdd_;
			}
			else
			{
				return table_->GetMtbdd(children);
			}
		}

		inline void SetMtbdd(const StateTuple& children, const TransMTBDD& mtbdd)
		{
			if (children.empty())
			{
				nullaryMtbdd_ = mtbdd;;
			}
			else
			{
				table_->SetMtbdd(children, mtbdd);
			}
		}

		inline TablePtr GetTable() const
		{
			return table_;
		}

		inline bool unique() const
		{
			return table_.unique();
		}

		inline const_iterator begin() const
		{
			return const_iterator(*this, true);
		}

		inline const_iterator cbegin() const
		{
			return begin();
		}

		inline const_iterator end() const
		{
			return const_iterator(*this, false);
		}

		inline const_iterator cend() const
		{
			return end();
		}
	};

public:   // data types

	typedef TransTableWrapper TransTable;

private:  // data types

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
	TransTable transTable_;

private:  // methods

	template <class StateTransFunc, class SymbolTransFunc>
	void loadFromAutDescExplicit(const AutDescription& desc,
		StateTransFunc stateTranslator, SymbolTransFunc symbolTranslator)
	{
		for (auto fst : desc.finalStates)
		{	// traverse final states
			finalStates_.insert(stateTranslator(fst));
		}

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
		}
	}


	template <class StateTransFunc, class SymbolTransFunc>
	void loadFromAutDescSymbolic(const AutDescription&/* desc */,
		StateTransFunc /* stateTranslator */, SymbolTransFunc /* symbolTranslator */)
	{
		assert(false);
	}

	template <class StateBackTransFunc, class SymbolBackTransFunc>
	AutDescription dumpToAutDescExplicit(
		StateBackTransFunc stateBackTranslator,
		SymbolBackTransFunc /* symbolBackTranslator */) const
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
		for (auto fst : finalStates_)
		{	// copy final states
			desc.finalStates.insert(stateBackTranslator(fst));
		}

		CondColApplyFunctor collector;

		// copy states, transitions and symbols
		for (auto tupleBddPair : GetTransTable())
		{	// for all states
			const StateTuple& children = tupleBddPair.first;

			std::vector<std::string> tupleStr;

			for (auto state : children)
			{
				std::string stateStr = stateBackTranslator(state);

				tupleStr.push_back(stateStr);
				desc.states.insert(stateStr);
			}

			const TransMTBDD& transMtbdd = tupleBddPair.second;

			for (auto sym : GetSymbolDict())
			{	// iterate over all known symbols
				const std::string& symbol = sym.first;
				BDD symbolBdd(sym.second, true, false);

				collector.Clear();
				collector(transMtbdd, symbolBdd);

				for (auto state : collector.GetAccumulator())
				{	// for each state tuple for which there is a transition
					std::string stateStr = stateBackTranslator(state);

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

public:   // methods

	BDDBottomUpTreeAut() :
		finalStates_(),
		transTable_()
	{ }

	BDDBottomUpTreeAut(const TransTable& transTable) :
		finalStates_(),
		transTable_(transTable)
	{ }

	BDDBottomUpTreeAut(const BDDBottomUpTreeAut& aut) :
		finalStates_(aut.finalStates_),
		transTable_(aut.transTable_)
	{ }

	BDDBottomUpTreeAut(BDDBottomUpTreeAut&& aut) :
		finalStates_(std::move(aut.finalStates_)),
		transTable_(std::move(aut.transTable_))
	{ }

	BDDBottomUpTreeAut& operator=(const BDDBottomUpTreeAut& rhs)
	{
		if (this != &rhs)
		{
			finalStates_ = rhs.finalStates_;
			transTable_ = rhs.transTable_;
		}

		return *this;
	}

	BDDBottomUpTreeAut& operator=(BDDBottomUpTreeAut&& rhs)
	{
		if (this != &rhs)
		{
			finalStates_ = std::move(rhs.finalStates_);
			transTable_ = std::move(rhs.transTable_);
		}

		return *this;
	}

	inline const TransTable& GetTransTable() const
	{
		return transTable_;
	}


	void AddTransition(const StateTuple& children,
		SymbolType symbol, const StateType& parent)
	{
		// Assertions
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
	}


	inline void SetStateFinal(const StateType& state)
	{
		finalStates_.insert(state);
	}

	inline bool IsStateFinal(const StateType& state) const
	{
		return finalStates_.find(state) != finalStates_.end();
	}

	inline const StateHT& GetFinalStates() const
	{
		return finalStates_;
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
		for (auto tupleHandlePair : transTable_)
		{
			tupleVec.push_back(&tupleHandlePair.second);
		}

		return TransMTBDD::DumpToDot(tupleVec);
	}

	BDDTopDownTreeAut GetTopDownAut() const;

	template <class OperationFunc>
	static void ForeachUpSymbolFromTupleAndTupleSetDo(
		const BDDBottomUpTreeAut& lhs, const BDDBottomUpTreeAut& rhs,
		const StateTuple& lhsTuple, const StateTupleSet& rhsTupleSet,
		OperationFunc& opFunc)
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

	inline const TransMTBDD& GetMtbdd(const StateTuple& children) const
	{
		return transTable_.GetMtbdd(children);
	}

	inline void SetMtbdd(const StateTuple& children, const TransMTBDD& mtbdd)
	{
		transTable_.SetMtbdd(children, mtbdd);
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
		for (auto tupleBddPair : transTable_)
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

		TransMTBDD nullaryBdd = rewriter(GetMtbdd(StateTuple()));
		dstAut.SetMtbdd(StateTuple(), nullaryBdd);

		return nullaryBdd;
	}

	static inline bool ShareTransTable(const BDDBottomUpTreeAut& lhs,
		const BDDBottomUpTreeAut& rhs)
	{
		return lhs.transTable_.GetTable() == rhs.transTable_.GetTable();
	}
};

#endif
