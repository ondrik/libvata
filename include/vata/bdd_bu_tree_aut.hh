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
#include <vata/mtbdd/apply2func.hh>
#include <vata/mtbdd/ondriks_mtbdd.hh>
#include <vata/mtbdd/void_apply2func.hh>
#include <vata/util/bdd_trans_table.hh>
#include <vata/util/ord_vector.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>
#include <vata/util/vector_map.hh>

namespace VATA { class BDDBottomUpTreeAut; }

GCC_DIAG_OFF(effc++)
class VATA::BDDBottomUpTreeAut
	: public AutBase
{
GCC_DIAG_ON(effc++)

	friend BDDBottomUpTreeAut Union(const BDDBottomUpTreeAut&,
		const BDDBottomUpTreeAut&, AutBase::StateToStateMap*);

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

public:   // public data types

	typedef VATA::MTBDDPkg::VarAsgn SymbolType;

	typedef std::unordered_set<StateType> StateHT;
	typedef VATA::Util::OrdVector<StateType> StateSet;
	typedef std::vector<StateType> StateTuple;

	typedef VATA::Util::TwoWayDict<std::string, SymbolType> StringToSymbolDict;

	typedef VATA::Util::TranslatorStrict<StringToSymbolDict> SymbolTranslatorStrict;
	typedef VATA::Util::TranslatorStrict<typename StringToSymbolDict::MapBwdType>
		SymbolBackTranslatorStrict;

private:  // private data types

	typedef size_t MTBDDHandle;
	typedef VATA::MTBDDPkg::OndriksMTBDD<StateSet> TransMTBDD;
	typedef VATA::MTBDDPkg::OndriksMTBDD<bool> BDD;

	typedef VATA::Util::BDDTransTable<MTBDDHandle, StateSet> TransTable;

	typedef VATA::Util::AutDescription AutDescription;

	typedef std::shared_ptr<TransTable> TransTablePtr;

	typedef VATA::Util::VectorMap<StateType, MTBDDHandle> TupleToMTBDDMap;

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

	static const size_t SYMBOL_SIZE = 16;

private:  // data members

	StateHT states_;
	StateHT finalStates_;
	TransTablePtr transTable_;
	TupleToMTBDDMap mtbddMap_;
	MTBDDHandle defaultTrFuncHandle_;

	static StringToSymbolDict* pSymbolDict_;
	static SymbolType* pNextSymbol_;

private:  // methods

	bool isValid() const;

	bool isStandAlone() const;

	inline bool hasEmptyStateSet() const
	{
		// Assertions
		assert(isValid());

		return mtbddMap_.empty() && finalStates_.empty();
	}

	void copyStates(const BDDBottomUpTreeAut& src);

	inline const MTBDDHandle& getMtbddHandle(const StateTuple& children) const
	{
		// Assertions
		assert(isValid());

		typename TupleToMTBDDMap::const_iterator it;
		if ((it = mtbddMap_.find(children)) == mtbddMap_.end())
		{
			return defaultTrFuncHandle_;
		}
		else
		{
			return (*it).second;
		}
	}

	inline const TransMTBDD& getMtbdd(const MTBDDHandle& handle) const
	{
		// Assertions
		assert(isValid());

		return transTable_->GetMtbdd(handle);
	}

	inline const TransMTBDD& getMtbdd(const StateTuple& children) const
	{
		// Assertions
		assert(isValid());

		return getMtbdd(getMtbddHandle(children));
	}

	inline void decrementHandleRefCnt(const StateTuple& children)
	{
		// Assertions
		assert(isValid());

		typename TupleToMTBDDMap::iterator it;
		if ((it = mtbddMap_.find(children)) != mtbddMap_.end())
		{	// in case there is something
			transTable_->DecrementHandleRefCnt((*it).second);
		}
	}

	inline void setMtbdd(const MTBDDHandle& handle, const TransMTBDD& mtbdd)
	{
		// Assertions
		assert(isValid());
		assert(transTable_->GetHandleRefCnt(handle) == 1);

		transTable_->SetMtbdd(handle, mtbdd);
	}

	inline void setMtbdd(const StateTuple& children, const TransMTBDD& mtbdd)
	{
		// Assertions
		assert(isValid());

		typename TupleToMTBDDMap::iterator it;
		if ((it = mtbddMap_.find(children)) == mtbddMap_.end())
		{	// in case the value is unknown
			MTBDDHandle handle = transTable_->AddHandle();
			setMtbdd(handle, mtbdd);
			mtbddMap_.insert(std::make_pair(children, handle));
		}
		else
		{
			MTBDDHandle& handle = (*it).second;

			if (transTable_->GetHandleRefCnt(handle) == 1)
			{	// in case there is only one reference to the MTBDD
				setMtbdd(handle, mtbdd);
			}
			else
			{
				transTable_->DecrementHandleRefCnt(handle);
				handle = transTable_->AddHandle();
				setMtbdd(handle, mtbdd);
			}
		}
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

	template <class StateTransFunc, class SymbolTransFunc>
	void loadFromAutDescExplicit(const AutDescription& desc,
		StateTransFunc stateTranslator, SymbolTransFunc symbolTranslator)
	{
		// Assertions
		assert(hasEmptyStateSet());
		assert(pSymbolDict_ != nullptr);

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
		for (auto tupleHandlePair : mtbddMap_)
		{	// for all states
			const StateTuple& children = tupleHandlePair.first;

			std::vector<std::string> tupleStr;

			for (auto state : children)
			{
				std::string stateStr = stateBackTranslator(state);

				tupleStr.push_back(stateStr);
				desc.states.insert(stateStr);
			}

			const TransMTBDD& transMtbdd = getMtbdd(tupleHandlePair.second);

			for (auto sym : *pSymbolDict_)
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

	inline void deallocateTuples()
	{
		// Assertions
		assert(isValid());

		for (auto tupleHandlePair : mtbddMap_)
		{	// release all states
			transTable_->DecrementHandleRefCnt(tupleHandlePair.second);
		}

		transTable_->DecrementHandleRefCnt(defaultTrFuncHandle_);

		mtbddMap_.clear();
	}

public:   // methods

	BDDBottomUpTreeAut() :
		states_(),
		finalStates_(),
		transTable_(new TransTable),
		mtbddMap_(),
		defaultTrFuncHandle_(transTable_->AddHandle())
	{
		// Assertions
		assert(isValid());
	}

	BDDBottomUpTreeAut(TransTablePtr transTable) :
		states_(),
		finalStates_(),
		transTable_(transTable),
		mtbddMap_(),
		defaultTrFuncHandle_(transTable_->AddHandle())
	{
		// Assertions
		assert(isValid());
	}

	BDDBottomUpTreeAut(const BDDBottomUpTreeAut& aut) :
		states_(),
		finalStates_(),
		transTable_(aut.transTable_),
		mtbddMap_(),
		defaultTrFuncHandle_(aut.defaultTrFuncHandle_)
	{
		copyStates(aut);
		transTable_->IncrementHandleRefCnt(defaultTrFuncHandle_);

		// Assertions
		assert(isValid());
	}

	BDDBottomUpTreeAut(BDDBottomUpTreeAut&& aut) :
		states_(std::move(aut.states_)),
		finalStates_(std::move(aut.finalStates_)),
		transTable_(std::move(aut.transTable_)),
		mtbddMap_(std::move(aut.mtbddMap_)),
		defaultTrFuncHandle_(std::move(aut.defaultTrFuncHandle_))
	{
		// Assertions
		assert(isValid());
	}

	BDDBottomUpTreeAut& operator=(const BDDBottomUpTreeAut& rhs);

	BDDBottomUpTreeAut& operator=(BDDBottomUpTreeAut&& rhs)
	{
		if (this != &rhs)
		{
			deallocateTuples();

			states_ = std::move(rhs.states_);
			finalStates_ = std::move(rhs.finalStates_);
			transTable_ = std::move(rhs.transTable_);
			mtbddMap_ = std::move(rhs.mtbddMap_);
			defaultTrFuncHandle_ = std::move(rhs.defaultTrFuncHandle_);
		}

		return *this;
	}

	inline TransTablePtr& GetTransTable() const
	{
		return const_cast<TransTablePtr&>(transTable_);
	}

	inline StateType AddState()
	{
		// Assertions
		assert(isValid());

		StateType newState = transTable_->AddState();

		if (!states_.insert(newState).second)
		{	// in case the state is already there
			assert(false);
		}

		assert(isValid());

		return newState;
	}

	void AddSimplyTransition(const StateTuple& children, SymbolType symbol,
		const StateType& parent);

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

	inline const TupleToMTBDDMap& GetTuples() const
	{
		return mtbddMap_;
	}

	inline const StateHT& GetStates() const
	{
		return states_;
	}

	inline const StateHT& GetFinalStates() const
	{
		// Assertions
		assert(isValid());

		return finalStates_;
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

	std::string DumpToDot() const
	{
		std::vector<const TransMTBDD*> tupleVec;
		for (auto tupleHandlePair : mtbddMap_)
		{
			tupleVec.push_back(&getMtbdd(tupleHandlePair.second));
		}

		return TransMTBDD::DumpToDot(tupleVec);
	}

	inline SymbolType AddSymbol()
	{
		// Assertions
		assert(pNextSymbol_ != nullptr);

		return (*pNextSymbol_)++;
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

	inline static void SetNextSymbolPtr(SymbolType* pNextSymbol)
	{
		// Assertions
		assert(pNextSymbol != nullptr);

		pNextSymbol_ = pNextSymbol;
	}

	~BDDBottomUpTreeAut();
};

#endif
