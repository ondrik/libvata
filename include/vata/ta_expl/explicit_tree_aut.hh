/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a explicitly represented tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_AUT_HH_
#define _VATA_EXPLICIT_TREE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>
#include <vata/explicit_lts.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/incl_param.hh>

#include <vata/util/ord_vector.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>
#include <vata/util/convert.hh>
#include <vata/util/cache.hh>
#include <vata/util/util.hh>

// Standard library headers
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <unordered_map>

namespace VATA
{
	class ExplicitTreeAut;

	struct Explicit
	{
		typedef AutBase::StateType StateType;

		typedef std::vector<StateType> StateTuple;
		typedef std::shared_ptr<StateTuple> TuplePtr;
		typedef std::set<StateTuple> TupleSet;
		typedef std::unordered_set<StateType> StateSet;

		typedef Util::Cache<StateTuple> TupleCache;

		static TupleCache tupleCache;

	};

}

GCC_DIAG_OFF(effc++)
class VATA::ExplicitTreeAut : public AutBase
{
GCC_DIAG_ON(effc++)

	friend class ExplicitUpwardInclusion;
	friend class ExplicitDownwardComplementation;
	friend class ExplicitDownwardInclusion;

public:   // public data types

	typedef VATA::Util::OrdVector<StateType> StateSetLight;
	typedef Explicit::StateType StateType;
	typedef Explicit::StateTuple StateTuple;
	typedef Explicit::TuplePtr TuplePtr;
	typedef std::vector<TuplePtr> StateTupleSet;
	typedef std::set<TuplePtr> DownInclStateTupleSet;
	typedef std::vector<TuplePtr> DownInclStateTupleVector;

	typedef Explicit::StateSet StateSet;
	typedef Explicit::TupleCache TupleCache;

	typedef uintptr_t SymbolType;

	struct StringRank
	{
		std::string symbolStr;
		size_t rank;

		StringRank(const std::string& symbolStr, size_t rank) :
			symbolStr(symbolStr),
			rank(rank)
		{ }

		bool operator<(const StringRank& rhs) const
		{
			return ((rank < rhs.rank) ||
				((rank == rhs.rank) && (symbolStr < rhs.symbolStr)));
		}
	};

	typedef std::vector<std::pair<SymbolType, size_t>> AlphabetType;

	typedef VATA::Util::TwoWayDict<StringRank, SymbolType> StringToSymbolDict;

	typedef VATA::Util::TranslatorStrict<StringToSymbolDict> SymbolTranslatorStrict;
	typedef VATA::Util::TranslatorStrict<typename StringToSymbolDict::MapBwdType>
		SymbolBackTranslatorStrict;

private:  // private data types

	typedef VATA::Util::AutDescription AutDescription;
	typedef std::set<TuplePtr> TuplePtrSet;
	typedef std::shared_ptr<TuplePtrSet> TuplePtrSetPtr;

	typedef VATA::Util::Convert Convert;

	GCC_DIAG_OFF(effc++)
	class TransitionCluster : public std::unordered_map<SymbolType, TuplePtrSetPtr> {
	GCC_DIAG_ON(effc++)

	public:

		const TuplePtrSetPtr& uniqueTuplePtrSet(const SymbolType& symbol) {

			auto& tupleSet = this->insert(
				std::make_pair(symbol, TuplePtrSetPtr(nullptr))
			).first->second;

			if (!tupleSet) {

				tupleSet = TuplePtrSetPtr(new TuplePtrSet());

			} else if (!tupleSet.unique()) {

				tupleSet = TuplePtrSetPtr(new TuplePtrSet(*tupleSet));

			}

			return tupleSet;

		}

	};

	typedef std::shared_ptr<TransitionCluster> TransitionClusterPtr;

	GCC_DIAG_OFF(effc++)
	class StateToTransitionClusterMap : public std::unordered_map<StateType, TransitionClusterPtr> {
	GCC_DIAG_ON(effc++)

	public:

		const TransitionClusterPtr& uniqueCluster(const StateType& state) {

			auto& cluster = this->insert(
				std::make_pair(state, TransitionClusterPtr(nullptr))
			).first->second;

			if (!cluster) {

				cluster = TransitionClusterPtr(new TransitionCluster());

			} else if (!cluster.unique()) {

				cluster = TransitionClusterPtr(new TransitionCluster(*cluster));

			}

			return cluster;

		}

	};

	typedef std::shared_ptr<StateToTransitionClusterMap> StateToTransitionClusterMapPtr;

protected:

	template <class T>
	static const typename T::mapped_type::element_type* genericLookup(
		const T&                        cont,
		const typename T::key_type&     key)
	{
		auto iter = cont.find(key);
		if (iter == cont.end())
		{
			return nullptr;
		}

		return iter->second.get();
	}

	TuplePtr tupleLookup(const StateTuple& tuple)
	{
		return cache_.lookup(tuple);
	}

	const StateToTransitionClusterMapPtr& uniqueClusterMap()
	{
		assert(nullptr != transitions_);

		if (!transitions_.unique())
		{
			transitions_ = StateToTransitionClusterMapPtr(
				new StateToTransitionClusterMap(*transitions_)
			);

		}

		return transitions_;
	}

	void internalAddTransition(
		const TuplePtr&           children,
		const SymbolType&         symbol,
		const StateType&          parent)
	{
		this->uniqueClusterMap()->uniqueCluster(parent)->uniqueTuplePtrSet(symbol)->insert(children);
	}

public:

	class Transition
	{
		friend class ExplicitTreeAut;

		const StateTuple& children_;
		const SymbolType& symbol_;
		const StateType& parent_;

		Transition(
			const StateTuple&        children,
			const SymbolType&        symbol,
			const StateType&         parent) :
			children_(children),
			symbol_(symbol),
			parent_(parent)
		{ }

	public:

		const StateTuple& children() const { return children_; }
		const SymbolType& symbol()   const { return symbol_;   }
		const StateType& parent()    const { return parent_;   }
	};

	struct Iterator
	{
		typedef std::input_iterator_tag iterator_category;
		typedef size_t difference_type;
		typedef Transition value_type;
		typedef Transition* pointer;
		typedef Transition& reference;

		const ExplicitTreeAut& aut_;
		typename StateToTransitionClusterMap::const_iterator stateClusterIterator_;
		typename TransitionCluster::const_iterator symbolSetIterator_;
		TuplePtrSet::const_iterator tupleIterator_;

		Iterator(int, const ExplicitTreeAut& aut) : aut_(aut), stateClusterIterator_(),
			symbolSetIterator_(), tupleIterator_() {}

		Iterator(const ExplicitTreeAut& aut) : aut_(aut),
			stateClusterIterator_(aut.transitions_->begin()), symbolSetIterator_(), tupleIterator_()
		{

			if (this->stateClusterIterator_ == aut.transitions_->end())
				return;

			this->symbolSetIterator_ = this->stateClusterIterator_->second->begin();

			assert(this->symbolSetIterator_ != this->stateClusterIterator_->second->end());

			this->tupleIterator_ = this->symbolSetIterator_->second->begin();

			assert(this->tupleIterator_ != this->symbolSetIterator_->second->end());

		}

		Iterator& operator++() {

			if (++this->tupleIterator_ != this->symbolSetIterator_->second->end())
				return *this;

			if (++this->symbolSetIterator_ != this->stateClusterIterator_->second->end()) {
				this->tupleIterator_ = this->symbolSetIterator_->second->begin();
				return *this;
			}

			if (++this->stateClusterIterator_ != this->aut_.transitions_->end()) {
				this->symbolSetIterator_ = this->stateClusterIterator_->second->begin();
				this->tupleIterator_ = this->symbolSetIterator_->second->begin();
				return *this;
			}

			this->tupleIterator_ = TuplePtrSet::const_iterator();

			return *this;

		}

		Iterator operator++(int)
		{
			return ++Iterator(*this);
		}

		bool operator==(const Iterator& rhs) const
		{
			return tupleIterator_ == rhs.tupleIterator_;
		}

		bool operator!=(const Iterator& rhs) const
		{
			return tupleIterator_ != rhs.tupleIterator_;
		}

		Transition operator*() const
		{
			assert(*tupleIterator_);

			return Transition(
				**tupleIterator_,
				symbolSetIterator_->first,
				stateClusterIterator_->first
			);
		}
	};

	typedef Iterator iterator;
	typedef Iterator const_iterator;

	Iterator begin() const { return Iterator(*this); }
	Iterator end() const { return Iterator(0, *this); }

public:

// TODO: remove --- I suspect this was not used anywhere
#if 0
	struct AcceptingTransitions {

		const ExplicitTreeAut& aut_;

		AcceptingTransitions(const ExplicitTreeAut& aut) : aut_(aut) {}

		struct Iterator {

			typedef std::input_iterator_tag iterator_category;
			typedef size_t difference_type;
			typedef Transition value_type;
			typedef Transition* pointer;
			typedef Transition& reference;

			const ExplicitTreeAut& aut_;

			StateSet::const_iterator stateSetIterator_;
			typename StateToTransitionClusterMap::const_iterator stateClusterIterator_;
			typename TransitionCluster::const_iterator symbolSetIterator_;
			TuplePtrSet::const_iterator tupleIterator_;

			Iterator(int, const ExplicitTreeAut& aut) : aut_(aut), stateSetIterator_(),
				stateClusterIterator_(), symbolSetIterator_(), tupleIterator_() {}

			Iterator(const ExplicitTreeAut& aut) : aut_(aut),
				stateSetIterator_(aut.finalStates_.begin()), stateClusterIterator_(),
				symbolSetIterator_(), tupleIterator_() {

				this->_init();

			}

			void _init() {

				for (; this->stateSetIterator_ != this->aut_.finalStates_.end();
					++this->stateSetIterator_) {

					this->stateClusterIterator_ =
						this->aut_.transitions_->find(*this->stateSetIterator_);

					if (this->stateClusterIterator_ != this->aut_.transitions->.end())
						break;

				}

				if (this->stateSetIterator_ == this->aut.finalStates_->end()) {
					this->tupleIterator_ = TuplePtrSet::const_iterator();
					return;
				}

				this->symbolSetIterator_ = this->stateClusterIterator_->second->begin();
				this->tupleIterator_ = this->symbolSetIterator_->second->begin();

			}

			Iterator& operator++() {

				if (++this->tupleIterator_ != this->symbolSetIterator_->second->end())
					return *this;

				if (++this->symbolSetIterator_ != this->stateClusterIterator_->second->end()) {
					this->tupleIterator_ = this->symbolSetIterator_->second->begin();
					return *this;
				}

				++this->stateSetIterator_;

				this->_init();

				return *this;

			}

			Iterator operator++(int) {

				return ++Iterator(*this);

			}

			bool operator==(const Iterator& rhs) const {

				return this->tupleIterator_ == rhs.tupleIterator_;

			}

			bool operator!=(const Iterator& rhs) const {

				return this->tupleIterator_ != rhs.tupleIterator_;

			}

			Transition operator*() const {

				assert(*this->tupleIterator_);

				return Transition(
					**this->tupleIterator_,
					this->symbolSetIterator_->first,
					this->stateClusterIterator_->first
				);

			}

		};

		typedef Iterator iterator;
		typedef Iterator const_iterator;

		Iterator begin() const { return Iterator(this->aut_); }
		Iterator end() const { return Iterator(0, this->aut_); }

	};

	AcceptingTransitions accepting;
#endif

public:

#if 0
	struct ClusterAccessor {

		const size_t& state_;
		const TransitionCluster* cluster_;

		ClusterAccessor(const size_t& state, const TransitionCluster* cluster) : state_(state),
			cluster_(cluster) {}

		struct Iterator {

			typedef std::input_iterator_tag iterator_category;
			typedef size_t difference_type;
			typedef Transition value_type;
			typedef Transition* pointer;
			typedef Transition& reference;

			const ClusterAccessor& accessor_;

			typename TransitionCluster::const_iterator symbolSetIterator_;
			TuplePtrSet::const_iterator tupleIterator_;

			Iterator(int, ClusterAccessor& accessor) : accessor_(accessor), symbolSetIterator_(),
				tupleIterator_() {}

			Iterator(ClusterAccessor& accessor) : accessor_(accessor), symbolSetIterator_(),
				tupleIterator_() {

				if (!accessor.cluster_)
					return;

				this->symbolSetIterator_ = accessor.cluster_->begin();

				assert(this->symbolSetIterator_ != this->stateClusterIterator_->second->end());

				this->tupleIterator_ = this->symbolSetIterator_->second->begin();

				assert(this->tupleIterator_ != this->symbolSetIterator_->second->end());

			}

			Iterator& operator++() {

				if (++this->tupleIterator_ != this->symbolSetIterator_->second->end())
					return *this;

				if (++this->symbolSetIterator_ != this->stateClusterIterator_->second->end()) {
					this->tupleIterator_ = this->symbolSetIterator_->second->begin();
					return *this;
				}

				this->tupleIterator_ = TuplePtrSet::const_iterator();

				return *this;

			}

			Iterator operator++(int) {

				return ++Iterator(*this);

			}

			bool operator==(const Iterator& rhs) const {

				return this->tupleIterator_ == rhs.tupleIterator_;

			}

			bool operator!=(const Iterator& rhs) const {

				return this->tupleIterator_ != rhs.tupleIterator_;

			}

			Transition operator*() const {

				assert(*this->tupleIterator_);

				return Transition(
					**this->tupleIterator_, this->symbolSetIterator_->first, this->accessor.state_
				);

			}

		};

		typedef Iterator iterator;
		typedef Iterator const_iterator;

		Iterator begin() const { return Iterator(this->aut_); }
		Iterator end() const { return Iterator(0, this->aut_); }

		bool empty() const { return this->cluster != nullptr; }

	};

#endif

private:  // data members

	TupleCache& cache_;

	StateSet finalStates_;

	StateToTransitionClusterMapPtr transitions_;

	static StringToSymbolDict* pSymbolDict_;
	static SymbolType* pNextSymbol_;

public:   // public methods

	ExplicitTreeAut(Explicit::TupleCache& tupleCache = Explicit::tupleCache) :
		cache_(tupleCache),
		finalStates_(),
		transitions_(StateToTransitionClusterMapPtr(new StateToTransitionClusterMap()))
	{ }

	ExplicitTreeAut(const ExplicitTreeAut& aut) :
		cache_(aut.cache_),
		finalStates_(aut.finalStates_),
		transitions_(aut.transitions_)
	{ }

	ExplicitTreeAut(const ExplicitTreeAut& aut, Explicit::TupleCache& tupleCache) :
		cache_(tupleCache),
		finalStates_(aut.finalStates_),
		transitions_(aut.transitions_)
	{ }

	ExplicitTreeAut& operator=(const ExplicitTreeAut& rhs)
	{
		if (this != &rhs)
		{
			finalStates_ = rhs.finalStates_;
			transitions_ = rhs.transitions_;
		}

		return *this;
	}

	~ExplicitTreeAut() {}

	void LoadFromString(VATA::Parsing::AbstrParser& parser, const std::string& str,
		StringToStateDict& stateDict)
	{
		LoadFromAutDesc(parser.ParseString(str), stateDict);
	}

	void LoadFromAutDesc(const AutDescription& desc, StringToStateDict& stateDict)
	{

		typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
			StateTranslator;
		typedef VATA::Util::TranslatorWeak<StringToSymbolDict>
			SymbolTranslator;

		StateType stateCnt = 0;

		LoadFromAutDesc(desc,
			StateTranslator(stateDict,
				[&stateCnt](const std::string&){return stateCnt++;}),
			SymbolTranslator(GetSymbolDict(),
				[this](const StringRank&){return this->AddSymbol();}));
	}

	template <class StateTransFunc, class SymbolTransFunc>
	void LoadFromString(VATA::Parsing::AbstrParser& parser, const std::string& str,
		StateTransFunc stateTranslator, SymbolTransFunc symbolTranslator,
		const std::string& params = "") {

		LoadFromAutDesc(parser.ParseString(str), stateTranslator, symbolTranslator, params);
	}

	template <class StateTransFunc, class SymbolTransFunc>
	void LoadFromAutDesc(
		const AutDescription&          desc,
		StateTransFunc                 stateTranslator,
		SymbolTransFunc                symbolTranslator,
		const std::string&             /* params */ = "")
	{
		for (auto symbolRankPair : desc.symbols)
			symbolTranslator(StringRank(symbolRankPair.first, symbolRankPair.second));

		for (auto s : desc.finalStates) {
			this->finalStates_.insert(stateTranslator(s));
		}

		for (auto t : desc.transitions) {

			// traverse the transitions
			const AutDescription::StateTuple& childrenStr = t.first;
			const std::string& symbolStr = t.second;
			const AutDescription::State& parentStr = t.third;

			// translate children
			StateTuple children;
			for (auto c : childrenStr) {
				// for all children states
				children.push_back(stateTranslator(c));
			}

			this->AddTransition(
				children,
				symbolTranslator(StringRank(symbolStr, children.size())),
				stateTranslator(parentStr));
		}

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

	std::string DumpToString(VATA::Serialization::AbstrSerializer& serializer,
		const StringToStateDict& stateDict) const
	{
		struct SymbolTranslatorPrinter
		{
			const SymbolBackTranslatorStrict& translator;

			explicit SymbolTranslatorPrinter(const SymbolBackTranslatorStrict& transl) :
				translator(transl)
			{ }

			const StringRank& operator()(const SymbolType& /* sym */) const
			{
				throw NotImplementedException(__func__);
			}
		};

		SymbolTranslatorPrinter printer(
			SymbolBackTranslatorStrict(GetSymbolDict().GetReverseMap()));

		return DumpToString(serializer,
			StateBackTranslatorStrict(stateDict.GetReverseMap()),
			printer);
	}

	template <class StatePrintFunc, class SymbolPrintFunc>
	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&     serializer,
		StatePrintFunc                            statePrinter,
		SymbolPrintFunc                           symbolPrinter,
		const std::string&                        /* params */ = "") const
	{
#if 0
		struct SymbolTranslatorPrinter
		{
			std::string operator()(const SymbolType& sym) const
			{
				return SymPrFnc(sym).symbolStr;
			}
		};
#endif

		auto printer = [&symbolPrinter](const SymbolType& sym)
			{
				return symbolPrinter(sym).symbolStr;
			};

		AutDescription desc;

		for (auto& s : this->finalStates_)
		{
			desc.finalStates.insert(statePrinter(s));
		}

		for (auto t : *this)
		{
			std::vector<std::string> tupleStr;

			for (const StateType& s : t.children())
			{
				tupleStr.push_back(statePrinter(s));
			}

			AutDescription::Transition trans(
				tupleStr,
				printer(t.symbol()),
				statePrinter(t.parent()));

			desc.transitions.insert(trans);
		}

		return serializer.Serialize(desc);
	}

	const StateSet& GetFinalStates() const
	{
		return finalStates_;
	}

	template <class TranslIndex, class SanitizeIndex>
	std::string PrintSimulationMapping (TranslIndex index, SanitizeIndex sanitizeIndex) {
		std::string res = "";
		std::unordered_set<StateType> translatedStates;
		
		for (auto t : *this)
		{

			for (auto& s : t.children())
			{
				if (!translatedStates.count(s))
				{
					res = res + VATA::Util::Convert::ToString(index(s)) + " -> " +
						VATA::Util::Convert::ToString(sanitizeIndex[s]) + "\n";
					translatedStates.insert(s);
				}
			}

			if (!translatedStates.count(t.state()))
			{
				res = res + VATA::Util::Convert::ToString(index(t.state())) + " -> " +
					VATA::Util::Convert::ToString(sanitizeIndex[t.state()]) + "\n";
				translatedStates.insert(t.state());
			}
		}
		return res;
	}

	void SetStateFinal(
		const StateType&          state)
	{
		finalStates_.insert(state);
	}

	bool IsFinalState(
		const StateType&          state) const
	{
		return finalStates_.count(state) > 0;
	}

	void AddTransition(
		const StateTuple&         children,
		const SymbolType&         symbol,
		const StateType&          state)
	{
		this->internalAddTransition(this->tupleLookup(children), symbol, state);
	}

	static void CopyTransitions(
		ExplicitTreeAut&           dst,
		const ExplicitTreeAut&     src)
	{
		dst.transitions_ = src.transitions_;
	}

#if 0
	ClusterAccessor GetCluster(
		const StateType&           state) const
	{
		return ClusterAccessor(state, ExplicitTreeAut::genericLookup(transitions_, state));
	}

	ClusterAccessor operator[](
		const StateType&           state) const
	{
		return this->GetCluster(state);
	}
#endif

	template <class Index>
	void BuildStateIndex(
		Index&                     index) const
	{
		for (const StateType& state : finalStates_)
		{
			index(state);
		}

		for (auto& stateClusterPair : *transitions_)
		{
			assert(stateClusterPair.second);

			index(stateClusterPair.first);

			for (auto& symbolTupleSetPair : *stateClusterPair.second)
			{
				assert(symbolTupleSetPair.second);

				for (auto& tuple : *symbolTupleSetPair.second)
				{
					assert(tuple);

					for (const StateType& s : *tuple)
					{
						index(s);
					}
				}
			}
		}
	}

	template <class Index>
<<<<<<< HEAD:include/vata/ta_expl/explicit_tree_aut.hh
	void ReindexStates(
		ExplicitTreeAut&          dst,
		Index&                    index) const
	{
		for (const StateType& state : finalStates_)
		{
=======
	void ReindexStates(ExplicitTreeAut& dst, Index& index) const {

		for (auto& state : this->finalStates_) {
>>>>>>> improve_sim_output:include/vata/explicit_tree_aut.hh
			dst.SetStateFinal(index[state]);
		}

		auto clusterMap = dst.uniqueClusterMap();

		for (auto& stateClusterPair : *transitions_)
		{
			assert(stateClusterPair.second);

			auto cluster = clusterMap->uniqueCluster(index[stateClusterPair.first]);

			for (auto& symbolTupleSetPair : *stateClusterPair.second)
			{
				assert(symbolTupleSetPair.second);

				auto tuplePtrSet = cluster->uniqueTuplePtrSet(symbolTupleSetPair.first);

				for (auto& tuple : *symbolTupleSetPair.second)
				{
					assert(tuple);

					StateTuple newTuple;

<<<<<<< HEAD:include/vata/ta_expl/explicit_tree_aut.hh
					for (const StateType& s : *tuple)
					{
=======
					for (auto& s : *tuple) {
>>>>>>> improve_sim_output:include/vata/explicit_tree_aut.hh
						newTuple.push_back(index[s]);
					}

					tuplePtrSet->insert(dst.tupleLookup(newTuple));
				}
			}
		}
	}

	template <class OperationFunc>
	static void ForeachDownSymbolFromStateAndStateSetDo(
		const ExplicitTreeAut&      lhs,
		const ExplicitTreeAut&      rhs,
		const StateType&            lhsState,
		const StateSetLight&        rhsSet,
		OperationFunc&              opFunc)
	{
		assert(nullptr != lhs.transitions_);
		assert(nullptr != rhs.transitions_);

		auto leftCluster = ExplicitTreeAut::genericLookup(*lhs.transitions_, lhsState);

		if (!leftCluster)
		{
			return;
		}

		std::vector<const TransitionCluster*> rightClusters;

		for (const StateType& rhsState : rhsSet)
		{
			auto rightCluster = ExplicitTreeAut::genericLookup(*rhs.transitions_, rhsState);

			if (rightCluster)
			{
				rightClusters.push_back(rightCluster);
			}
		}

		for (auto& leftSymbolTupleSetPair : *leftCluster)
		{
			TuplePtrSet rightTuples;

			for (auto& rightCluster : rightClusters)
			{
				auto rightTupleSet = ExplicitTreeAut::genericLookup(
					*rightCluster, leftSymbolTupleSetPair.first
				);

				if (!rightTupleSet)
				{
					continue;
				}

				rightTuples.insert(rightTupleSet->begin(), rightTupleSet->end());
			}

			auto AccessElementF = [](const TuplePtr& tuplePtr){return *tuplePtr;};

			assert(leftSymbolTupleSetPair.second);

			opFunc(
				*leftSymbolTupleSetPair.second,
				AccessElementF,
				rightTuples,
				AccessElementF
			);
		}
	}

public:

#if 0
	inline StateType AddState()
	{
		// Assertions
		assert(pNextState_ != nullptr);

		return (*pNextState_)++;
	}
#endif

	static inline SymbolType AddSymbol()
	{
		// Assertions
		assert(pNextSymbol_ != nullptr);

		return (*pNextSymbol_)++;
	}

	static inline StringToSymbolDict& GetSymbolDict()
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

	inline static DownInclStateTupleVector StateTupleSetToVector(
		const DownInclStateTupleSet& tupleSet)
	{
		return DownInclStateTupleVector(tupleSet.begin(), tupleSet.end());
	}

	static AlphabetType GetAlphabet()
	{
		AlphabetType alphabet;
		for (auto stringRankAndSymbolPair : GetSymbolDict())
		{
			alphabet.push_back(std::make_pair(
				stringRankAndSymbolPair.second,
				stringRankAndSymbolPair.first.rank));
		}

		return alphabet;
	}


	template <class Index = Util::IdentityTranslator<StateType>>
	ExplicitLTS TranslateDownward(
		const Index&                 stateIndex = Index()) const;


	template <
		class Rel,
		class Index = Util::IdentityTranslator<StateType>
	>
	ExplicitLTS TranslateUpward(
		std::vector<std::vector<size_t>>&     partition,
		Util::BinaryRelation&                 relation,
		const Rel&                            param,
		const Index&                          stateIndex = Index()) const;


	template <
		class Rel,
		class Index = Util::IdentityTranslator<AutBase::StateType>>
	ExplicitTreeAut CollapseStates(
		const Rel&                       rel,
		const Index&                     bwIndex = Index()) const
	{
		std::vector<size_t> representatives;

		rel.buildClasses(representatives);

		std::vector<StateType> transl(representatives.size());

		Util::RebindMap2(transl, representatives, bwIndex);

		ExplicitTreeAut res(cache_);

		this->ReindexStates(res, transl);

		return res;
	}

	template <class Index>
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		size_t            size,
		const Index&      index) const;

	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		size_t            size) const;

#if 0
	AutBase::StateBinaryRelation ComputeDownwardSimulation() const
	{
		return ComputeDownwardSimulation(aut, AutBase::SanitizeAutForSimulation(aut));
	}
#endif

	template <class Index>
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		size_t                   size,
		const Index&             index) const
	{
		std::vector<std::vector<size_t>> partition;

		AutBase::StateBinaryRelation relation;

		return TranslateUpward(
			*this, partition, relation, Util::Identity(size), index
		).computeSimulation(partition, relation, size);
	}


	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		size_t             size) const
	{
		std::vector<std::vector<size_t>> partition;

		AutBase::StateBinaryRelation relation;

		return this->TranslateUpward(
			partition, relation, Util::Identity(size)
		).computeSimulation(partition, relation, size);
	}

#if 0
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const ExplicitTreeAut& aut)
	{
		return ComputeUpwardSimulation(aut, AutBase::SanitizeAutForSimulation(aut));
	}
#endif


	ExplicitTreeAut Reduce() const;

	ExplicitTreeAut RemoveUnreachableStates(
		AutBase::StateToStateMap*            pTranslMap = nullptr);

	template <
		class Rel,
		class Index = Util::IdentityTranslator<AutBase::StateType>
	>
	ExplicitTreeAut RemoveUnreachableStates(
		const Rel&                           rel,
		const Index&                         index = Index());

	ExplicitTreeAut RemoveUselessStates(
		AutBase::StateToStateMap*          pTranslMap = nullptr) const;

	ExplicitTreeAut GetCandidateTree() const;


	/**
	 * @brief  Unites a pair of automata
	 *
	 * Function for the union of two automata. It takes a pair of automata,
	 * renames their states and then merges them into a single automaton. States
	 * are renamed by the default dictionary or by a user defined dictionary, so
	 * they may be overlapping.
   *
   * @param[in]      lhs             Left automaton for union
   * @param[in]      rhs             Right automaton for union
   * @param[in,out]  pTranslMapLhs   Dictionary for renaming left automaton
   * @param[in,out]  pTranslMapRhs   Dictionary for renaming right automaton
	 *
	 * @returns  An automaton accepting the union of languages of @p lhs and @p
	 * rhs
   */
	static ExplicitTreeAut Union(
		const ExplicitTreeAut&                lhs,
		const ExplicitTreeAut&                rhs,
		AutBase::StateToStateMap*             pTranslMapLhs = nullptr,
		AutBase::StateToStateMap*             pTranslMapRhs = nullptr);


	/**
	 * @brief  Unites two automata with disjoint sets of states
	 *
	 * Unites two automata. Note that these automata need to have disjoint sets of
	 * states, otherwise the result is undefined.
	 *
   * @param[in]      lhs             Left automaton for union
   * @param[in]      rhs             Right automaton for union
	 *
	 * @returns  An automaton accepting the union of languages of @p lhs and @p
	 * rhs
   */
	static ExplicitTreeAut UnionDisjointStates(
		const ExplicitTreeAut&           lhs,
		const ExplicitTreeAut&           rhs);


	/**
	 * @brief  Intersection of languages of a pair of automata
	 *
	 * This function creates an automaton that accepts the languages defined as
	 * the intersection of langauges of a pair of automata.
	 *
   * @param[in]   lhs             Left automaton
   * @param[in]   rhs             Right automaton
   * @param[out]  pTranslMapLhs   Dictionary for the result
	 *
	 * @returns  An automaton accepting the intersection of languages of @p lhs
	 * and @p rhs
   */
	static ExplicitTreeAut Intersection(
		const ExplicitTreeAut&            lhs,
		const ExplicitTreeAut&            rhs,
		AutBase::ProductTranslMap*        pTranslMap);


	/**
	 * @brief  Dispatcher for calling correct inclusion checking function
	 *
	 * This function is a dispatcher that calls a proper inclusion checking
	 * function between @p smaller and @p bigger according to the parameters in @p
	 * params.
	 *
	 * @param[in]  smaller  The smaller automaton
	 * @param[in]  bigger   The bigger automaton
	 * @param[in]  params   Parameters for the inclusion (can be @p nullptr for
	 *                      the default parameters)
	 *
	 * @returns  @p true if the language of @p smaller is a subset of the language
	 *           of @p bigger, @p false otherwise
	 */
	static bool CheckInclusion(
		const ExplicitTreeAut&                 smaller,
		const ExplicitTreeAut&                 bigger,
		const VATA::InclParam&                 params);
};

#endif
