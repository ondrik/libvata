/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for an explicitly represented tree automaton.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_AUT_HH_
#define _VATA_EXPLICIT_TREE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/incl_param.hh>

#include <vata/util/ord_vector.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>
#include <vata/util/util.hh>

// Standard library headers
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <unordered_map>


namespace VATA
{
	class ExplicitTreeAut;

	class ExplicitTreeAutCore;
}


GCC_DIAG_OFF(effc++)
class VATA::ExplicitTreeAut : public AutBase
{
GCC_DIAG_ON(effc++)
public:   // public data types

	using SymbolType     = uintptr_t;
	using StateTuple     = std::vector<StateType>;

	using AlphabetType   = std::vector<std::pair<SymbolType, size_t>>;

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

	using StringToSymbolDict   = VATA::Util::TwoWayDict<StringRank, SymbolType>;
	using SymbolBackTranslatorStrict  =
		VATA::Util::TranslatorStrict<typename StringToSymbolDict::MapBwdType>;


	//typedef Explicit::StateType StateType;
	//typedef Explicit::TuplePtr TuplePtr;
	//typedef std::vector<TuplePtr> StateTupleSet;

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

public:   // methods

	ExplicitTreeAut();
	ExplicitTreeAut(const ExplicitTreeAut& aut);
	ExplicitTreeAut(ExplicitTreeAut&& aut);

	ExplicitTreeAut& operator=(const ExplicitTreeAut& rhs);
	ExplicitTreeAut& operator=(ExplicitTreeAut&& rhs);

	~ExplicitTreeAut();

	explicit ExplicitTreeAut(ExplicitTreeAutCore&& core);


	static AlphabetType GetAlphabet()
	{
		throw NotImplementedException(__func__);
	}


	ExplicitTreeAut Reduce() const;


	ExplicitTreeAut ReindexStates(
		StateToStateTranslator&     stateTrans) const;


	ExplicitTreeAut RemoveUnreachableStates(
		AutBase::StateToStateMap*            pTranslMap = nullptr) const;


	template <
		class Rel,
		class Index = Util::IdentityTranslator<AutBase::StateType>
	>
	ExplicitTreeAut RemoveUnreachableStates(
		const Rel&                           rel,
		const Index&                         index = Index()) const;


	ExplicitTreeAut RemoveUselessStates(
		AutBase::StateToStateMap*          pTranslMap = nullptr) const;


	ExplicitTreeAut GetCandidateTree() const;


	static void SetSymbolDictPtr(StringToSymbolDict* pSymbolDict);


	static void SetNextSymbolPtr(SymbolType* pNextSymbol);


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&     serializer) const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&     serializer,
		const StringToStateDict&                  stateDict) const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StateBackTranslatorStrict&           stateTrans,
		const SymbolBackTranslatorStrict&          symbolTrans) const;


	template <
		class TranslIndex,
		class SanitizeIndex>
	std::string PrintSimulationMapping(
		TranslIndex            /*index*/,
		SanitizeIndex          /*sanitizeIndex*/)
	{
		throw NotImplementedException(__func__);

#if 0
		std::string res;
		std::unordered_set<StateType> translatedStates;

		for (auto trans : *this)
		{
			for (auto& s : trans.children())
			{
				if (!translatedStates.count(s))
				{
					res = res + VATA::Util::Convert::ToString(index(s)) + " -> " +
						VATA::Util::Convert::ToString(sanitizeIndex[s]) + "\n";
					translatedStates.insert(s);
				}
			}

			if (!translatedStates.count(trans.parent()))
			{
				res = res + VATA::Util::Convert::ToString(index(trans.parent())) + " -> " +
					VATA::Util::Convert::ToString(sanitizeIndex[trans.parent()]) + "\n";
				translatedStates.insert(trans.parent());
			}
		}

		return res;
#endif
	}


	static StringToSymbolDict& GetSymbolDict()
	{
		throw NotImplementedException(__func__);
	}


	void LoadFromString(
		VATA::Parsing::AbstrParser&       parser,
		const std::string&                str,
		StringToStateDict&                stateDict);


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


	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		size_t            size) const;


	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		size_t            size) const;


	template <class Dict>
	ExplicitTreeAut Complement(
		const Dict&                           /*alphabet*/) const
	{
		throw NotImplementedException(__func__);
	}


private:  // data members

	std::unique_ptr<ExplicitTreeAutCore> core_;

};

#endif
