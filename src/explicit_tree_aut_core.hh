/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the core of an explicitly represented tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_AUT_CORE_HH_
#define _VATA_EXPLICIT_TREE_AUT_CORE_HH_

// VATA headers
#include <vata/aut_base.hh>
#include <vata/ta_expl/explicit_tree_aut.hh>

#include <vata/util/ord_vector.hh>
#include <vata/util/util.hh>
#include <vata/util/convert.hh>
#include <vata/explicit_lts.hh>
#include <vata/incl_param.hh>

#include "util/cache.hh"

namespace VATA { class ExplicitTreeAutCore; }

GCC_DIAG_OFF(effc++)
class VATA::ExplicitTreeAutCore : public AutBase
{
GCC_DIAG_ON(effc++)

	friend class ExplicitUpwardInclusion;
	friend class ExplicitDownwardComplementation;
	friend class ExplicitDownwardInclusion;

public:   // data types

	using SymbolType     = ExplicitTreeAut::SymbolType;
	using StateTuple     = ExplicitTreeAut::StateTuple;
	using TuplePtr       = std::shared_ptr<StateTuple>;
	using StateSet       = std::unordered_set<StateType>;

	using DownInclStateTupleSet       = std::set<TuplePtr>;
	using DownInclStateTupleVector    = std::vector<TuplePtr>;

private:  // data types

	using AutDescription   = VATA::Util::AutDescription;

	using AlphabetType     = ExplicitTreeAut::AlphabetType;

	using StateSetLight    = VATA::Util::OrdVector<StateType>;

	using StringRank       = ExplicitTreeAut::StringRank;


	class Transition
	{
		friend class ExplicitTreeAutCore;

	private:  // data members

		const StateTuple& children_;
		const SymbolType& symbol_;
		const StateType& parent_;

	private:  // methods

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


	using TuplePtrSet      = std::set<TuplePtr>;
	using TuplePtrSetPtr   = std::shared_ptr<TuplePtrSet>;
	using TupleSet         = std::set<StateTuple>;
	using TupleCache       = Util::Cache<StateTuple>;

	using Convert    = VATA::Util::Convert;

	using StringToSymbolDict    = ExplicitTreeAut::StringToSymbolDict;

	using SymbolTranslatorStrict      = VATA::Util::TranslatorStrict<StringToSymbolDict>;
	using SymbolBackTranslatorStrict  = ExplicitTreeAut::SymbolBackTranslatorStrict;


	GCC_DIAG_OFF(effc++)
	class TransitionCluster : public std::unordered_map<
		SymbolType,
		TuplePtrSetPtr>
	{
	GCC_DIAG_ON(effc++)

	public:

		const TuplePtrSetPtr& uniqueTuplePtrSet(const SymbolType& symbol)
		{
			auto& tupleSet = this->insert(
				std::make_pair(symbol, TuplePtrSetPtr(nullptr))
			).first->second;

			if (!tupleSet)
			{
				tupleSet = TuplePtrSetPtr(new TuplePtrSet());
			}
			else if (!tupleSet.unique())
			{
				tupleSet = TuplePtrSetPtr(new TuplePtrSet(*tupleSet));
			}

			return tupleSet;
		}
	};


	using TransitionClusterPtr    = std::shared_ptr<TransitionCluster>;

	GCC_DIAG_OFF(effc++)
	class StateToTransitionClusterMap : public std::unordered_map<
		StateType,
		TransitionClusterPtr>
	{
	GCC_DIAG_ON(effc++)

	public:

		const TransitionClusterPtr& uniqueCluster(const StateType& state)
		{
			auto& cluster = this->insert(
				std::make_pair(state, TransitionClusterPtr(nullptr))
			).first->second;

			if (!cluster)
			{
				cluster = TransitionClusterPtr(new TransitionCluster());
			}
			else if (!cluster.unique())
			{
				cluster = TransitionClusterPtr(new TransitionCluster(*cluster));
			}

			return cluster;
		}
	};


	using StateToTransitionClusterMapPtr  = std::shared_ptr<StateToTransitionClusterMap>;


	struct Iterator
	{
	public:   // data types
		typedef std::input_iterator_tag iterator_category;
		typedef size_t difference_type;
		typedef Transition value_type;
		typedef Transition* pointer;
		typedef Transition& reference;

	public:    // data members

		const ExplicitTreeAutCore& aut_;
		typename StateToTransitionClusterMap::const_iterator stateClusterIterator_;
		typename TransitionCluster::const_iterator symbolSetIterator_;
		TuplePtrSet::const_iterator tupleIterator_;

	public:   // methods


		Iterator(
			int                          /* FILL (only to distinguish signature?) */,
			const ExplicitTreeAutCore&   aut) :
			aut_(aut),
			stateClusterIterator_(),
			symbolSetIterator_(),
			tupleIterator_()
		{ }


		Iterator(
			const ExplicitTreeAutCore&   aut) :
			aut_(aut),
			stateClusterIterator_(aut.transitions_->begin()),
			symbolSetIterator_(),
			tupleIterator_()
		{
			if (aut.transitions_->end() == stateClusterIterator_)
			{
				return;
			}

			symbolSetIterator_ = stateClusterIterator_->second->begin();
			assert(stateClusterIterator_->second->end() != symbolSetIterator_);

			tupleIterator_ = symbolSetIterator_->second->begin();
			assert(symbolSetIterator_->second->end() != tupleIterator_);
		}

		Iterator& operator++()
		{
			if (symbolSetIterator_->second->end() != ++tupleIterator_)
			{
				return *this;
			}

			if (stateClusterIterator_->second->end() != ++symbolSetIterator_)
			{
				tupleIterator_ = symbolSetIterator_->second->begin();
				return *this;
			}

			if (aut_.transitions_->end() != ++stateClusterIterator_)
			{
				symbolSetIterator_ = stateClusterIterator_->second->begin();
				tupleIterator_ = symbolSetIterator_->second->begin();
				return *this;
			}

			tupleIterator_ = TuplePtrSet::const_iterator();

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


	Iterator begin() const
	{
		return Iterator(*this);
	}


	Iterator end() const
	{
		return Iterator(0, *this);
	}


private:  // data members

	TupleCache& cache_;

	StateSet finalStates_;

	StateToTransitionClusterMapPtr transitions_;

	static StringToSymbolDict* pSymbolDict_;
	static SymbolType* pNextSymbol_;

	static TupleCache globalTupleCache_;

private:  // methods


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
		this->uniqueClusterMap()->uniqueCluster(parent)
			->uniqueTuplePtrSet(symbol)->insert(children);
	}


public:   // methods


	explicit ExplicitTreeAutCore(
		TupleCache&                   tupleCache = globalTupleCache_);


	ExplicitTreeAutCore(
		const ExplicitTreeAutCore&    aut);


	ExplicitTreeAutCore(
		ExplicitTreeAutCore&&         aut);


	ExplicitTreeAutCore(
		const ExplicitTreeAutCore&    aut,
		TupleCache&                   tupleCache);


	ExplicitTreeAutCore& operator=(
		const ExplicitTreeAutCore&    rhs);


	ExplicitTreeAutCore& operator=(
		ExplicitTreeAutCore&&         rhs);


	~ExplicitTreeAutCore()
	{ }


	const StateSet& GetFinalStates() const
	{
		return finalStates_;
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
		ExplicitTreeAutCore&           dst,
		const ExplicitTreeAutCore&     src)
	{
		dst.transitions_ = src.transitions_;
	}


	template <
		class StateTransFunc,
		class SymbolTransFunc
		>
	void LoadFromAutDesc(
		const AutDescription&          desc,
		StateTransFunc                 stateTranslator,
		SymbolTransFunc                symbolTranslator,
		const std::string&             /* params */ = "")
	{
		for (auto symbolRankPair : desc.symbols)
		{
			symbolTranslator(StringRank(symbolRankPair.first, symbolRankPair.second));
		}

		for (const AutDescription::State& s : desc.finalStates)
		{
			finalStates_.insert(stateTranslator(s));
		}

		for (const AutDescription::Transition& t : desc.transitions)
		{
			// traverse the transitions
			const AutDescription::StateTuple& childrenStr = t.first;
			const std::string& symbolStr = t.second;
			const AutDescription::State& parentStr = t.third;

			// translate children
			StateTuple children;
			for (const AutDescription::State& c : childrenStr)
			{ // for all children states
				children.push_back(stateTranslator(c));
			}

			this->AddTransition(
				children,
				symbolTranslator(StringRank(symbolStr, children.size())),
				stateTranslator(parentStr));
		}
	}


	void LoadFromAutDesc(
		const AutDescription&             desc,
		StringToStateDict&                stateDict);


	void LoadFromString(
		VATA::Parsing::AbstrParser&       parser,
		const std::string&                str,
		StringToStateDict&                stateDict);


	template <
		class StateTransFunc,
		class SymbolTransFunc
		>
	void LoadFromString(
		VATA::Parsing::AbstrParser&       parser,
		const std::string&                str,
		StateTransFunc                    stateTranslator,
		SymbolTransFunc                   symbolTranslator,
		const std::string&                params = "")
	{
		this->LoadFromAutDesc(
			parser.ParseString(str),
			stateTranslator,
			symbolTranslator,
			params);
	}

	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&     serializer) const;

	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&     serializer,
		const StringToStateDict&                  stateDict) const;


	template <class SymbolTransFunc>
	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&     serializer,
		SymbolTransFunc                           symbolTranslator,
		const std::string&                        params = "") const
	{
		return this->DumpToString(
			serializer,
			[](const StateType& state){return Convert::ToString(state);},
			symbolTranslator,
			params);
	}



	template <
		class StatePrintFunc,
		class SymbolPrintFunc
		>
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

		for (const StateType& s : finalStates_)
		{
			desc.finalStates.insert(statePrinter(s));
		}

		for (const Transition& t : *this)
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
	void ReindexStates(
		ExplicitTreeAutCore&      dst,
		Index&                    index) const
	{
		for (const StateType& state : finalStates_)
		{
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

					for (const StateType& s : *tuple)
					{
						newTuple.push_back(index[s]);
					}

					tuplePtrSet->insert(dst.tupleLookup(newTuple));
				}
			}
		}
	}


	template <class Index>
	ExplicitTreeAutCore ReindexStates(
		Index&                    index) const
	{
		ExplicitTreeAutCore res;
		this->ReindexStates(res, index);

		return res;
	}


	ExplicitTreeAutCore ReindexStates(
		StateToStateTranslator&     stateTrans) const
	{
		return this->ReindexStates<StateToStateTranslator>(stateTrans);
	}


	template <class OperationFunc>
	static void ForeachDownSymbolFromStateAndStateSetDo(
		const ExplicitTreeAutCore&   lhs,
		const ExplicitTreeAutCore&   rhs,
		const StateType&             lhsState,
		const StateSetLight&         rhsSet,
		OperationFunc&               opFunc)
	{
		assert(nullptr != lhs.transitions_);
		assert(nullptr != rhs.transitions_);

		auto leftCluster = ExplicitTreeAutCore::genericLookup(*lhs.transitions_, lhsState);

		if (!leftCluster)
		{
			return;
		}

		std::vector<const TransitionCluster*> rightClusters;

		for (const StateType& rhsState : rhsSet)
		{
			auto rightCluster = ExplicitTreeAutCore::genericLookup(*rhs.transitions_, rhsState);

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
				auto rightTupleSet = ExplicitTreeAutCore::genericLookup(
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


	static SymbolType AddSymbol()
	{
		// Assertions
		assert(pNextSymbol_ != nullptr);

		return (*pNextSymbol_)++;
	}

	static StringToSymbolDict& GetSymbolDict()
	{
		// Assertions
		assert(pSymbolDict_ != nullptr);

		return *pSymbolDict_;
	}

	static void SetSymbolDictPtr(StringToSymbolDict* pSymbolDict)
	{
		// Assertions
		assert(pSymbolDict != nullptr);

		pSymbolDict_ = pSymbolDict;
	}

	static void SetNextSymbolPtr(SymbolType* pNextSymbol)
	{
		// Assertions
		assert(pNextSymbol != nullptr);

		pNextSymbol_ = pNextSymbol;
	}

	static DownInclStateTupleVector StateTupleSetToVector(
		const DownInclStateTupleSet& tupleSet)
	{
		return DownInclStateTupleVector(tupleSet.begin(), tupleSet.end());
	}


	static AlphabetType GetAlphabet();


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
	ExplicitTreeAutCore CollapseStates(
		const Rel&                       rel,
		const Index&                     bwIndex = Index()) const
	{
		std::vector<size_t> representatives;

		rel.buildClasses(representatives);

		std::vector<StateType> transl(representatives.size());

		Util::RebindMap2(transl, representatives, bwIndex);

		// TODO: directly return the output of ReindexStates?
		ExplicitTreeAutCore res(cache_);

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
		size_t             size) const;


#if 0
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const ExplicitTreeAut& aut)
	{
		return ComputeUpwardSimulation(aut, AutBase::SanitizeAutForSimulation(aut));
	}
#endif


	static ExplicitTreeAutCore Union(
		const ExplicitTreeAutCore&            lhs,
		const ExplicitTreeAutCore&            rhs,
		AutBase::StateToStateMap*             pTranslMapLhs = nullptr,
		AutBase::StateToStateMap*             pTranslMapRhs = nullptr);


	static ExplicitTreeAutCore UnionDisjointStates(
		const ExplicitTreeAutCore&        lhs,
		const ExplicitTreeAutCore&        rhs);


	static ExplicitTreeAutCore Intersection(
		const ExplicitTreeAutCore&           lhs,
		const ExplicitTreeAutCore&           rhs,
		VATA::AutBase::ProductTranslMap*     pTranslMap = nullptr);

	ExplicitTreeAutCore GetCandidateTree() const;


	ExplicitTreeAutCore RemoveUnreachableStates(
		AutBase::StateToStateMap*            pTranslMap = nullptr) const;


	ExplicitTreeAutCore RemoveUselessStates(
		StateToStateMap*            pTranslMap = nullptr) const;


	template <
		class Rel,
		class Index
	>
	ExplicitTreeAutCore RemoveUnreachableStates(
		const Rel&                                 rel,
		const Index&                               index) const;


	static bool CheckInclusion(
		const ExplicitTreeAutCore&             smaller,
		const ExplicitTreeAutCore&             bigger,
		const VATA::InclParam&                 params);


	template <
		class Dict,
		class Rel>
	ExplicitTreeAutCore ComplementWithPreorder(
		const Dict&                            alphabet,
		const Rel&                             preorder) const;


	template <class Dict>
	ExplicitTreeAutCore Complement(
		const Dict&                           alphabet) const;


	ExplicitTreeAutCore Reduce() const;
};

#endif
