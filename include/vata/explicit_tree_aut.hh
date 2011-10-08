/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
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
	friend ExplicitTreeAut<SymbolType> UnionDisjunctStates(const ExplicitTreeAut<SymbolType>& lhs,
		const ExplicitTreeAut<SymbolType>& rhs);

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

	template <class SymbolType>
	friend ExplicitLTS TranslateDownward(const ExplicitTreeAut<SymbolType>& aut);

	template <class SymbolType, class Rel>
	friend ExplicitLTS TranslateUpward(const ExplicitTreeAut<SymbolType>& aut,
		std::vector<std::vector<size_t>>& partition, Util::BinaryRelation& relation,
		const Rel& param);

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

			if (!tupleSet || !tupleSet.unique()) {

				tupleSet = TuplePtrSetPtr(
					tupleSet ? (new TuplePtrSet(*tupleSet)) : (new TuplePtrSet())
				);

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

			if (!cluster || !cluster.unique()) {

				cluster = TransitionClusterPtr(
					cluster ? (new TransitionCluster(*cluster)) : (new TransitionCluster())
				);

			}

			return cluster;

		}

	};

	typedef std::shared_ptr<StateToTransitionClusterMap> StateToTransitionClusterMapPtr;

protected:

	template <class T>
	static const typename T::mapped_type::element_type* genericLookup(const T& cont, const typename T::key_type& key) {

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

	struct Iterator {

		const ExplicitTreeAut& aut_;

		typename StateToTransitionClusterMap::const_iterator stateClusterIterator_;
		typename TransitionCluster::const_iterator symbolSetIterator_;
		TuplePtrSet::const_iterator tupleIterator_;

		void _init() {

			if (this->stateClusterIterator_ == this->aut.transitions_->end())
				return;
			this->symbolSetIterator_ = this->stateClusterIterator_->second->begin();
			this->tupleIterator_ = this->symbolSetIterator_->second->begin();

		}

		void _increment() {

			++this->tupleIterator_;
			if (this->tupleIterator_ != this->symbolSetIterator_->second->end())
				return;

			++this->symbolSetIterator_;
			if (this->symbolSetIterator_ != this->stateClusterIterator_->second->end()) {
				this->tupleIterator_ = this->symbolSetIterator_->second->begin();
				return;
			}

			++this->stateClusterIterator_;
			this->_init();

		}

		Iterator(const ExplicitTreeAut& aut,
			typename StateToTransitionClusterMap::const_iterator pos)
			: aut_(aut), stateClusterIterator_(pos) {

			this->_init();

		}

		bool operator==(const Iterator& rhs) const {

			if (this->stateClusterIterator_ != rhs.stateClusterIterator_)
				return false;

			if (this->stateClusterIterator_ == this->aut.transitions_->end())
				return true;

			return (this->symbolSetIterator_ == rhs.symbolSetIterator_) &&
				(this->tupleIterator_ == rhs.tupleIterator_);

		}

		Iterator& operator++() {

			this->_increment();

			return *this;

		}

		Iterator operator++(int) {

			Iterator iter = *this;

			iter._increment();

			return iter;

		}

		const StateTuple& children() const {
			return **this->tupleIterator_;
		}

		const SymbolType& symbol() const {
			return this->symbolSetIterator_->first;
		}

		const StateType& state() const {
			return this->stateClusterIterator_->first;
		}

	};

	typedef Iterator iterator;
	typedef Iterator const_iterator;

	Iterator begin() { return Iterator(*this, this->transitions_->begin()); }
	Iterator end() { return Iterator(*this, this->transitions_->end()); }

public:

	struct AcceptingTransitions {

		const ExplicitTreeAut& aut_;

		AcceptingTransitions(const ExplicitTreeAut& aut) : aut_(aut) {}

		struct Iterator {

			const ExplicitTreeAut& aut_;

			StateSet::const_iterator stateSetIterator_;
			typename TransitionCluster::const_iterator symbolSetIterator_;
			TuplePtrSet::const_iterator tupleIterator_;

			void _init() {

				typename StateToTransitionClusterMap::const_iterator iter;

				while (1) {
					if (this->stateSetIterator_ == this->aut.finalStates_->end())
						return;
					iter = this->aut_.transitions_.find(*this->stateSetIterator_);
					if (iter != this->aut_.transitions_.end())
						break;
					++this->stateSetIterator_;
				}

				this->symbolSetIterator_ = iter->second->begin();
				this->tupleIterator_ = this->symbolSetIterator_->second->begin();

			}

			void _increment() {

				++this->tupleIterator_;
				if (this->tupleIterator_ != this->symbolSetIterator_->second->end())
					return;

				++this->symbolSetIterator_;
				if (this->symbolSetIterator_ != this->stateClusterIterator_->second->end()) {
					this->tupleIterator_ = this->symbolSetIterator_->second->begin();
					return;
				}

				++this->stateSetIterator_;
				this->_init();

			}

			Iterator(const ExplicitTreeAut& aut, StateSet::const_iterator pos)
				: aut_(aut), stateSetIterator_(pos) {

				this->_init();

			}

			bool operator==(const Iterator& rhs) const {

				if (this->stateSetIterator_ != rhs.stateSetIterator_)
					return false;

				if (this->stateSetIterator_ == this->aut.finalStates_->end())
					return true;

				return (this->symbolSetIterator_ == rhs.symbolSetIterator_) &&
					(this->tupleIterator_ == rhs.tupleIterator_);

			}

			Iterator& operator++() {

				this->_increment();

				return *this;

			}

			Iterator operator++(int) {

				Iterator iter = *this;

				iter._increment();

				return iter;

			}

			const StateTuple& children() const {
				return **this->tupleIterator_;
			}

			const SymbolType& symbol() const {
				return this->symbolSetIterator_->first;
			}

			const StateType& state() const {
				return *this->stateIterator_;
			}

		};

		typedef Iterator iterator;
		typedef Iterator const_iterator;

		Iterator begin() { return Iterator(this->aut_, this->aut_.finalStates_->begin()); }
		Iterator end() { return Iterator(this->aut_, this->aut_.finalStates_->end()); }

	};

	AcceptingTransitions accepting;

private:  // data members

	TupleCache& cache_;

	StateSet finalStates_;

	StateToTransitionClusterMapPtr transitions_;

	static StringToSymbolDict* pSymbolDict_;
	static SymbolType* pNextSymbol_;

public:   // public methods

	class Transition {

		TuplePtr children_;
		SymbolType symbol_;
		StateType state_;

	public:
	
		Transition(const TuplePtr& children, const SymbolType& symbol, const StateType& state)
			 : children_(children), symbol_(symbol), state_(state) {}

		const StateTuple& children() const { return *this->children_; }
		const SymbolType& symbol() const { return this->symbol_; }
		const StateType& state() const { return this->state_; }

		bool operator==(const Transition& rhs) const {

			return this->children_.get() == rhs.children_.get() &&
				this->symbol_ == rhs.symbol_ &&
				this->state_ == rhs.state_;

		}

		friend std::ostream& operator<<(std::ostream& os, const Transition& t) {
			return os << t.symbol_ << Util::Convert::ToString(*t.children_) << "->" << t.state_;
		}

		friend size_t hash_value(const Transition& t) {

			size_t seed = 0;

			boost::hash_combine(seed, t.children_.get());
			boost::hash_combine(seed, t.symbol_);
			boost::hash_combine(seed, t.state_);

			return seed;

		}

	};

	typedef std::shared_ptr<Transition> TransitionPtr;

public:

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
		typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
			StateTranslator;
		typedef VATA::Util::TranslatorWeak<StringToSymbolDict>
			SymbolTranslator;

		StateType stateCnt = 0;

		LoadFromString(parser, str,
			StateTranslator(stateDict,
				[&stateCnt](const std::string&){return stateCnt++;}),
			SymbolTranslator(GetSymbolDict(),
				[this](const std::string&){return this->AddSymbol();}));
	}

	template <class StateTransFunc, class SymbolTransFunc>
	void LoadFromString(VATA::Parsing::AbstrParser& parser, const std::string& str,
		StateTransFunc stateTranslator, SymbolTransFunc symbolTranslator,
		const std::string& /* params */ = "") {

		AutDescription desc = parser.ParseString(str);

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

	template <class StatePrintFunc, class SymbolPrintFunc>
	std::string DumpToString(VATA::Serialization::AbstrSerializer& serializer,
		StatePrintFunc statePrinter, SymbolPrintFunc symbolPrinter,
		const std::string& /* params */ = "") const {

		AutDescription desc;

		for (auto& s : this->finalStates_)
			desc.finalStates.insert(statePrinter(s));

		for (auto& stateClusterPair : *this->transitions_) {

			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);

				for (auto& tuple : *symbolTupleSetPair.second) {

					std::vector<std::string> tupleStr;

					assert(tuple);

					for (auto& s : *tuple)
						tupleStr.push_back(statePrinter(s));

					desc.transitions.insert(
						AutDescription::Transition(
							tupleStr,
							symbolPrinter(symbolTupleSetPair.first),
							statePrinter(stateClusterPair.first)
						)
					);

				}

			}

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

	void ReindexStates(ExplicitTreeAut& dst, StateToStateTranslator& stateTrans) const {

		for (auto& state : this->finalStates_)

			dst.SetStateFinal(stateTrans(state));

		auto clusterMap = dst.uniqueClusterMap();

		for (auto& stateClusterPair : *this->transitions_) {

			assert(stateClusterPair.second);

			auto cluster = clusterMap->uniqueCluster(stateTrans(stateClusterPair.first));

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);

				auto tuplePtrSet = cluster->uniqueTuplePtrSet(symbolTupleSetPair.first);

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);

					StateTuple newTuple;

					for (auto& s : *tuple)
						newTuple.push_back(stateTrans(s));

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

	typedef std::vector<TransitionPtr> TransitionList;
	typedef std::vector<TransitionList> IndexedTransitionList;
	typedef std::vector<IndexedTransitionList> DoubleIndexedTransitionList;
	typedef std::unordered_map<Symbol, TransitionList> SymbolToTransitionListMap;
	typedef std::unordered_map<Symbol, IndexedTransitionList> SymbolToIndexedTransitionListMap;
	typedef std::unordered_map<Symbol, DoubleIndexedTransitionList> SymbolToDoubleIndexedTransitionListMap;
	typedef std::vector<SymbolToTransitionListMap> IndexedSymbolToTransitionListMap;
	typedef std::vector<SymbolToIndexedTransitionListMap> IndexedSymbolToIndexedTransitionListMap;

public:

	void bottomUpIndex(IndexedSymbolToTransitionListMap& bottomUpIndex,
		SymbolToTransitionListMap& leaves) const {

		for (auto& stateClusterPair : *this->transitions_) {

			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);
				assert(symbolTupleSetPair.second->size());

				auto& first = *symbolTupleSetPair.second->begin();

				assert(first);

				if (first->empty()) {

					auto& transitionList = leaves.insert(
						std::make_pair(symbolTupleSetPair.first, TransitionList())
					).first->second;

					for (auto& tuple : *symbolTupleSetPair.second) {
	
						assert(tuple);
						assert(tuple->empty());
	
						transitionList.push_back(
							TransitionPtr(
								new Transition(
									tuple, symbolTupleSetPair.first, stateClusterPair.first
								)
							)									
						);
	
					}

					continue;

				}

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);
					assert(tuple->size());

					TransitionPtr transition(
						new Transition(tuple, symbolTupleSetPair.first, stateClusterPair.first)
					);

					std::unordered_set<StateType> states;

					for (auto& state : *tuple) {

						if (!states.insert(state).second)
							continue;

						if (bottomUpIndex.size() <= state)
							bottomUpIndex.resize(state + 1);

						bottomUpIndex[state].insert(
							std::make_pair(symbolTupleSetPair.first, TransitionList())
						).first->second.push_back(transition);

					}

				}

			}

		}

	}

	void bottomUpIndex(IndexedSymbolToIndexedTransitionListMap& bottomUpIndex,
		SymbolToTransitionListMap& leaves) const {

		for (auto& stateClusterPair : *this->transitions_) {

			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);
				assert(symbolTupleSetPair.second->size());

				auto& first = *symbolTupleSetPair.second->begin();

				assert(first);

				if (first->empty()) {

					auto& transitionList = leaves.insert(
						std::make_pair(symbolTupleSetPair.first, TransitionList())
					).first->second;

					for (auto& tuple : *symbolTupleSetPair.second) {
	
						assert(tuple);
						assert(tuple->empty());
	
						transitionList.push_back(
							TransitionPtr(
								new Transition(
									tuple, symbolTupleSetPair.first, stateClusterPair.first
								)
							)									
						);
	
					}

					continue;

				}

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);
					assert(tuple->size());

					TransitionPtr transition(
						new Transition(tuple, symbolTupleSetPair.first, stateClusterPair.first)
					);

					size_t i = 0;

					for (auto& state : *tuple) {

						if (bottomUpIndex.size() <= state)
							bottomUpIndex.resize(state + 1);

						auto& indexedTransitionList = bottomUpIndex[state].insert(
							std::make_pair(symbolTupleSetPair.first, IndexedTransitionList())
						).first->second;

						if (indexedTransitionList.size() <= i)
							indexedTransitionList.resize(i + 1);

						indexedTransitionList[i].push_back(transition);

						++i;

					}

				}

			}

		}

	}

	void bottomUpIndex(IndexedTransitionList& bottomUpIndex,
		SymbolToTransitionListMap& leaves) const {

		for (auto& stateClusterPair : *this->transitions_) {

			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);
				assert(symbolTupleSetPair.second->size());

				auto& first = *symbolTupleSetPair.second->begin();

				assert(first);

				if (first->empty()) {

					auto& transitionList = leaves.insert(
						std::make_pair(symbolTupleSetPair.first, TransitionList())
					).first->second;

					for (auto& tuple : *symbolTupleSetPair.second) {
	
						assert(tuple);
						assert(tuple->empty());
	
						transitionList.push_back(
							TransitionPtr(
								new Transition(
									tuple, symbolTupleSetPair.first, stateClusterPair.first
								)
							)									
						);
	
					}

					continue;

				}

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);
					assert(tuple->size());

					TransitionPtr transition(
						new Transition(tuple, symbolTupleSetPair.first, stateClusterPair.first)
					);

					std::unordered_set<StateType> states;

					for (auto& state : *tuple) {

						if (!states.insert(state).second)
							continue;

						if (bottomUpIndex.size() <= state)
							bottomUpIndex.resize(state + 1);

						bottomUpIndex[state].push_back(transition);

					}

				}

			}

		}

	}

	void bottomUpIndex(SymbolToTransitionListMap& bottomUpIndex) const {

		for (auto& stateClusterPair : *this->transitions_) {

			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);

				auto& transitionList = bottomUpIndex.insert(
					std::make_pair(symbolTupleSetPair.first, TransitionList())
				).first->second;

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);

					TransitionPtr transition(
						new Transition(tuple, symbolTupleSetPair.first, stateClusterPair.first)
					);

					transitionList.push_back(transition);

				}

			}

		}

	}

	void bottomUpIndex(SymbolToDoubleIndexedTransitionListMap& bottomUpIndex,
		SymbolToTransitionListMap& leaves) const {

		for (auto& stateClusterPair : *this->transitions_) {

			assert(stateClusterPair.second);

			for (auto& symbolTupleSetPair : *stateClusterPair.second) {

				assert(symbolTupleSetPair.second);
				assert(symbolTupleSetPair.second->size());

				auto& first = *symbolTupleSetPair.second->begin();

				assert(first);

				if (first->empty()) {

					auto& transitionList = leaves.insert(
						std::make_pair(symbolTupleSetPair.first, TransitionList())
					).first->second;

					for (auto& tuple : *symbolTupleSetPair.second) {
	
						assert(tuple);
						assert(tuple->empty());
	
						transitionList.push_back(
							TransitionPtr(
								new Transition(
									tuple, symbolTupleSetPair.first, stateClusterPair.first
								)
							)									
						);
	
					}

					continue;

				}

				auto& doubleIndexedTransitionList = bottomUpIndex.insert(
					std::make_pair(
						symbolTupleSetPair.first, DoubleIndexedTransitionList(first->size())
					)
				).first->second;

				for (auto& tuple : *symbolTupleSetPair.second) {

					assert(tuple);
					assert(tuple->size());

					TransitionPtr transition(
						new Transition(tuple, symbolTupleSetPair.first, stateClusterPair.first)
					);

					size_t i = 0;

					for (auto& state : *tuple) {

						assert(i < doubleIndexedTransitionList.size());

						if (doubleIndexedTransitionList[i].size() <= state)
							doubleIndexedTransitionList[i].resize(state + 1);

						doubleIndexedTransitionList[i][state].push_back(transition);

						++i;

					}

				}

			}

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
