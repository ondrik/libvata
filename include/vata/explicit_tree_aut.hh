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
#include <vata/util/ord_vector.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>
#include <vata/util/convert.hh>
#include <vata/util/cache.hh>

// Standard library headers
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <unordered_map>

namespace VATA {

	template <class Symbol> class ExplicitTreeAut;

	struct Explicit {

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
template <class Symbol>
class VATA::ExplicitTreeAut : public AutBase {
GCC_DIAG_ON(effc++)

	template <class SymbolType>
	friend ExplicitTreeAut<SymbolType> Union(
		const ExplicitTreeAut<SymbolType>&, const ExplicitTreeAut<SymbolType>&,
		AutBase::StateToStateMap*, AutBase::StateToStateMap*);

	template <class SymbolType>
	friend ExplicitTreeAut<SymbolType> UnionDisjunctStates(const ExplicitTreeAut<SymbolType>&,
		const ExplicitTreeAut<SymbolType>&);

	template <class SymbolType>
	friend ExplicitTreeAut<SymbolType> Intersection(
		const ExplicitTreeAut<SymbolType>&, const ExplicitTreeAut<SymbolType>&,
		AutBase::ProductTranslMap*);

	template <class SymbolType>
	friend ExplicitTreeAut<SymbolType> RemoveUselessStates(
		const ExplicitTreeAut<SymbolType>&, AutBase::StateToStateMap*);

	template <class SymbolType>
	friend ExplicitTreeAut<SymbolType> RemoveUnreachableStates(
		const ExplicitTreeAut<SymbolType>&, AutBase::StateToStateMap*);

	template <class SymbolType, class Rel, class Index>
	friend ExplicitTreeAut<SymbolType> RemoveUnreachableStates(
		const ExplicitTreeAut<SymbolType>&, const Rel&, const Index&);

	template <class SymbolType, class Index>
	friend ExplicitLTS TranslateDownward(const ExplicitTreeAut<SymbolType>&, const Index&);

	template <class SymbolType, class Rel, class Index>
	friend ExplicitLTS TranslateUpward(const ExplicitTreeAut<SymbolType>&,
		std::vector<std::vector<size_t>>&, Util::BinaryRelation&, const Rel&, const Index&);

	template <class SymbolType, class Rel, class Index>
	friend ExplicitTreeAut<SymbolType> CollapseStates(const ExplicitTreeAut<SymbolType>&,
		const Rel&, const Index&);

	friend class ExplicitUpwardInclusion;
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

	typedef Symbol SymbolType;

	typedef VATA::Util::TwoWayDict<std::string, SymbolType> StringToSymbolDict;

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
	static const typename T::mapped_type::element_type* genericLookup(const T& cont,
		const typename T::key_type& key) {

		auto iter = cont.find(key);
		if (iter == cont.end())
			return nullptr;

		return iter->second.get();

	}

	TuplePtr tupleLookup(const StateTuple& tuple) {

		return this->cache_.lookup(tuple);

	}

	const StateToTransitionClusterMapPtr& uniqueClusterMap() {

		assert(this->transitions_);

		if (!this->transitions_.unique()) {

			this->transitions_ = StateToTransitionClusterMapPtr(
				new StateToTransitionClusterMap(*transitions_)
			);

		}

		return this->transitions_;

	}

	void internalAddTransition(const TuplePtr& children, const SymbolType& symbol, const StateType& state) {

		this->uniqueClusterMap()->uniqueCluster(state)->uniqueTuplePtrSet(symbol)->insert(children);

	}

public:

	class Transition {

		friend class ExplicitTreeAut;

		const StateTuple& children_;
		const SymbolType& symbol_;
		const StateType& state_;

		Transition(const StateTuple& children, const SymbolType& symbol, const StateType& state)
			 : children_(children), symbol_(symbol), state_(state) {}

	public:

		const StateTuple& children() const { return this->children_; }
		const SymbolType& symbol() const { return this->symbol_; }
		const StateType& state() const { return this->state_; }

	};

	struct Iterator {

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

	Iterator begin() const { return Iterator(*this); }
	Iterator end() const { return Iterator(0, *this); }

public:

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

				for (; this->stateSetIterator_ != this->aut.finalStates_->end();
					++this->stateSetIterator_) {

					this->stateClusterIterator_ =
						this->aut_.transitions_.find(*this->stateSetIterator_);

					if (this->stateClusterIterator_ != this->aut_.transitions_.end())
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

public:

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

private:  // data members

	TupleCache& cache_;

	StateSet finalStates_;

	StateToTransitionClusterMapPtr transitions_;

	static StringToSymbolDict* pSymbolDict_;
	static SymbolType* pNextSymbol_;

public:   // public methods

	ExplicitTreeAut(Explicit::TupleCache& tupleCache = Explicit::tupleCache) :
		accepting(*this),
		cache_(tupleCache),
		finalStates_(),
		transitions_(StateToTransitionClusterMapPtr(new StateToTransitionClusterMap()))
	{ }

	ExplicitTreeAut(const ExplicitTreeAut& aut) :
		accepting(*this),
		cache_(aut.cache_),
		finalStates_(aut.finalStates_),
		transitions_(aut.transitions_)
	{ }

	ExplicitTreeAut(const ExplicitTreeAut& aut, Explicit::TupleCache& tupleCache) :
		accepting(*this),
		cache_(tupleCache),
		finalStates_(aut.finalStates_),
		transitions_(aut.transitions)
	{ }

	ExplicitTreeAut& operator=(const ExplicitTreeAut& rhs) {

		if (this != &rhs) {

			this->finalStates_ = rhs.finalStates_;
			this->transitions_ = rhs.transitions_;

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
				[this](const std::string&){return this->AddSymbol();}));
	}

	template <class StateTransFunc, class SymbolTransFunc>
	void LoadFromString(VATA::Parsing::AbstrParser& parser, const std::string& str,
		StateTransFunc stateTranslator, SymbolTransFunc symbolTranslator,
		const std::string& params = "") {

		LoadFromAutDesc(parser.ParseString(str), stateTranslator, symbolTranslator, params);
	}

	template <class StateTransFunc, class SymbolTransFunc>
	void LoadFromAutDesc(const AutDescription& desc,
		StateTransFunc stateTranslator, SymbolTransFunc symbolTranslator,
		const std::string& /* params */ = "") {

		for (auto s : desc.finalStates)
			this->finalStates_.insert(stateTranslator(s));

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

			this->AddTransition(children, symbolTranslator(symbolStr), stateTranslator(parentStr));
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
		return DumpToString(serializer,
			StateBackTranslatorStrict(stateDict.GetReverseMap()),
			SymbolBackTranslatorStrict(GetSymbolDict().GetReverseMap()));
	}

	template <class StatePrintFunc, class SymbolPrintFunc>
	std::string DumpToString(VATA::Serialization::AbstrSerializer& serializer,
		StatePrintFunc statePrinter, SymbolPrintFunc symbolPrinter,
		const std::string& /* params */ = "") const {

		AutDescription desc;

		for (auto& s : this->finalStates_)
			desc.finalStates.insert(statePrinter(s));

		for (auto t : *this) {

			std::vector<std::string> tupleStr;

			for (auto& s : t.children())
				tupleStr.push_back(statePrinter(s));

			desc.transitions.insert(
				AutDescription::Transition(
					tupleStr, symbolPrinter(t.symbol()), statePrinter(t.state())
				)
			);

		}

		return serializer.Serialize(desc);

	}

	inline const StateSet& GetFinalStates() const {
		return this->finalStates_;
	}

	inline void SetStateFinal(const StateType& state) {
		this->finalStates_.insert(state);
	}

	inline bool IsFinalState(const StateType& state) const {
		return this->finalStates_.count(state) > 0;
	}

	inline void AddTransition(const StateTuple& children, const SymbolType& symbol,
		const StateType& state) {

		this->internalAddTransition(this->tupleLookup(children), symbol, state);

	}

	inline static void CopyTransitions(ExplicitTreeAut& dst, const ExplicitTreeAut& src) {

		dst.transitions_ = src.transitions_;

	}

	inline ClusterAccessor GetCluster(const StateType& state) const {

		return ClusterAccessor(state, ExplicitTreeAut::genericLookup(this->transitions_, state));

	}

	inline ClusterAccessor operator[](const StateType& state) const {

		return this->GetCluster(state);

	}

	template <class Index>
	void BuildStateIndex(Index& index) const {

		for (auto& state : this->finalStates_)
			index(state);

		for (auto& stateClusterPair : *this->transitions_) {

			assert(stateClusterPair.second);

			index(stateClusterPair.first);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);

					for (auto& s : *tuple)
						index(s);

				}

			}

		}

	}

	template <class Index>
	void ReindexStates(ExplicitTreeAut& dst, Index& index) const {

		for (auto& state : this->finalStates_)
			dst.SetStateFinal(index[state]);

		auto clusterMap = dst.uniqueClusterMap();

		for (auto& stateClusterPair : *this->transitions_) {

			assert(stateClusterPair.second);

			auto cluster = clusterMap->uniqueCluster(index[stateClusterPair.first]);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);

				auto tuplePtrSet = cluster->uniqueTuplePtrSet(symbolTupleSetPair.first);

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);

					StateTuple newTuple;

					for (auto& s : *tuple)
						newTuple.push_back(index[s]);

					tuplePtrSet->insert(dst.tupleLookup(newTuple));

				}

			}

		}

	}

	template <class OperationFunc>
	static void ForeachDownSymbolFromStateAndStateSetDo(const ExplicitTreeAut& lhs,
		const ExplicitTreeAut& rhs, const StateType& lhsState,
		const StateSetLight& rhsSet, OperationFunc& opFunc)
	{

		assert(lhs.transitions_);
		assert(rhs.transitions_);

		auto leftCluster = ExplicitTreeAut::genericLookup(*lhs.transitions_, lhsState);

		if (!leftCluster)
			return;

		std::vector<const TransitionCluster*> rightClusters;

		for (auto& rhsState : rhsSet) {

			auto rightCluster = ExplicitTreeAut::genericLookup(*rhs.transitions_, rhsState);

			if (rightCluster)
				rightClusters.push_back(rightCluster);

		}

		for (auto& leftSymbolTupleSetPair : *leftCluster) {

			TuplePtrSet rightTuples;

			for (auto& rightCluster : rightClusters) {

				auto rightTupleSet = ExplicitTreeAut::genericLookup(
					*rightCluster, leftSymbolTupleSetPair.first
				);

				if (!rightTupleSet)
					continue;

				rightTuples.insert(rightTupleSet->begin(), rightTupleSet->end());

			}

			auto AccessElementF = [](const TuplePtr& tuplePtr){return *tuplePtr;};

			assert(leftSymbolTupleSetPair.second);

			opFunc(
				*leftSymbolTupleSetPair.second, AccessElementF,
				rightTuples, AccessElementF
			);

		}

	}

public:

	inline StateType AddState()
	{
		// Assertions
		assert(pNextState_ != nullptr);

		return (*pNextState_)++;
	}

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
};

template <class Symbol>
typename VATA::ExplicitTreeAut<Symbol>::StringToSymbolDict*
	VATA::ExplicitTreeAut<Symbol>::pSymbolDict_ = nullptr;

template <class Symbol>
typename VATA::ExplicitTreeAut<Symbol>::SymbolType*
	VATA::ExplicitTreeAut<Symbol>::pNextSymbol_ = nullptr;

#endif
