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
#include <vata/reduce_param.hh>
#include <vata/sim_param.hh>

#include <vata/util/ord_vector.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/transl_weak.hh>
#include <vata/util/util.hh>

// Standard library headers
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>


namespace VATA
{
	class ExplicitTreeAut;

	template <
		class>
	class LoadableAut;

	class ExplicitTreeAutCore;

	namespace ExplicitTreeAutCoreUtil
	{
		class Iterator;
		class AcceptTrans;
	 	class AcceptTransIterator;
		class DownAccessor;
		class DownAccessorIterator;
	}

	class AbstractReindexF
	{
	public:
		virtual VATA::AutBase::StateType operator[](const VATA::AutBase::StateType&) = 0;
		virtual VATA::AutBase::StateType at(const VATA::AutBase::StateType&) const = 0;

		virtual ~AbstractReindexF()
		{ }
	};
}


class VATA::ExplicitTreeAut : public TreeAutBase
{
private:  // data types

	using CoreAut        = VATA::LoadableAut<ExplicitTreeAutCore>;
	using StateMap       = std::unordered_map<StateType, StateType>;

public:   // public data types

	using SymbolType     = uintptr_t;
	using StateSet       = std::unordered_set<StateType>;


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

		friend std::ostream& operator<<(std::ostream& os, const StringRank& strRank)
		{
			return os << strRank.symbolStr << ":" << strRank.rank;
		}
	};


	using StringSymbolType   = StringRank;
	using Transition         = TreeAutBase::TTransition<SymbolType>;

	using SymbolDict                      =
		VATA::Util::TwoWayDict<StringSymbolType, SymbolType>;
	using StringSymbolToSymbolTranslWeak  = Util::TranslatorWeak<SymbolDict>;
	using StringSymbolToSymbolTranslStrict= Util::TranslatorStrict<SymbolDict>;
	using SymbolBackTranslStrict          = Util::TranslatorStrict<SymbolDict::MapBwdType>;


	class AbstractCopyF
	{
	public:
		virtual bool operator()(const Transition&) = 0;

		virtual ~AbstractCopyF()
		{ }
	};


	class AbstractAlphabet
	{
	public:  // data types

		using FwdTranslator    = VATA::Util::AbstractTranslator<StringSymbolType, SymbolType>;
		using FwdTranslatorPtr = std::unique_ptr<FwdTranslator>;
		using BwdTranslator    = VATA::Util::AbstractTranslator<SymbolType, StringSymbolType>;
		using BwdTranslatorPtr = std::unique_ptr<BwdTranslator>;

	public:  // methods

		virtual FwdTranslatorPtr GetSymbolTransl() = 0;
		virtual BwdTranslatorPtr GetSymbolBackTransl() = 0;

		virtual ~AbstractAlphabet()
		{ }
	};


	class OnTheFlyAlphabet : public AbstractAlphabet
	{
	private:  // data members

		SymbolDict symbolDict_{};
		SymbolType nextSymbol_ = 0;

	public:   // methods

		OnTheFlyAlphabet() = default;

		OnTheFlyAlphabet(const OnTheFlyAlphabet& rhs) = default;

		virtual FwdTranslatorPtr GetSymbolTransl() override
		{
			FwdTranslator* fwdTransl = new
				StringSymbolToSymbolTranslWeak{symbolDict_,
				[&](const StringSymbolType&){return nextSymbol_++;}};

			return FwdTranslatorPtr(fwdTransl);
		}

		virtual BwdTranslatorPtr GetSymbolBackTransl() override
		{
			BwdTranslator* bwdTransl =
				new SymbolBackTranslStrict(symbolDict_.GetReverseMap());

			return BwdTranslatorPtr(bwdTransl);
		}

		/**
		 * @brief  Returns the dictionary
		 */
		const SymbolDict& GetSymbolDict() const
		{
			return symbolDict_;
		}

		virtual ~OnTheFlyAlphabet() override
		{ }
	};

	class DirectAlphabet : public AbstractAlphabet
	{
	public:  // data types

		class DirectTranslator : public FwdTranslator
		{ };

		class DirectBackTranslator : public BwdTranslator
		{
			virtual StringSymbolType operator()(const SymbolType& value) override
			{
				return const_cast<const DirectBackTranslator*>(this)->operator()(value);
			}

			virtual StringSymbolType operator()(const SymbolType& value) const override
			{
				return StringSymbolType(VATA::Util::Convert::ToString(value), 0);
			}
		};

	public:  // methods

		virtual FwdTranslatorPtr GetSymbolTransl() override
		{
			throw NotImplementedException(__func__);
		}

		virtual BwdTranslatorPtr GetSymbolBackTransl() override
		{
			return BwdTranslatorPtr(new DirectBackTranslator);
		}

		virtual ~DirectAlphabet() override
		{ }
	};

	/**
	 * @brief  Base class for translation of symbols
	 *
	 * This is the base class for functors translating symbols, see @fn TranslateSymbols.
	 */
	class AbstractSymbolTranslateF
	{
	public:
		virtual SymbolType operator()(const SymbolType&) = 0;

		virtual ~AbstractSymbolTranslateF()
		{ }
	};

	using AlphabetType = std::shared_ptr<AbstractAlphabet>;

	class Iterator
	{
	private:  // data types

		using CoreIterator  = ExplicitTreeAutCoreUtil::Iterator;

	private:  // data members

		/**
		 * @brief  Pointer to iterator in the core automaton
		 */
		std::unique_ptr<CoreIterator> coreIter_;

	public:   // methods

		Iterator(const Iterator& iter);
		explicit Iterator(const CoreIterator& coreIter);
		~Iterator();

		bool operator==(const Iterator& rhs) const;
		bool operator!=(const Iterator& rhs) const;
		Iterator& operator++();
		Transition operator*() const;
	};

	using iterator       = Iterator;
	using const_iterator = Iterator;

	class AcceptTrans
	{
	public:  // data types

		class Iterator
		{
		private:  // data types

			using CoreIterator = ExplicitTreeAutCoreUtil::AcceptTransIterator;

		private:  // data types

			std::unique_ptr<CoreIterator> coreAcceptTransIter_;

		public:   // methods

			explicit Iterator(const ExplicitTreeAut& aut);
			explicit Iterator(const CoreIterator& coreIter);
			Iterator(const Iterator& iter);
			~Iterator();

			bool operator==(const Iterator& rhs) const;
			bool operator!=(const Iterator& rhs) const;
			Transition operator*() const;

			/**
			 * @brief  Prefix increment operator
			 */
			Iterator& operator++();
		};

		using iterator         = Iterator;
		using const_iterator   = Iterator;

		using CoreAcceptTrans  = ExplicitTreeAutCoreUtil::AcceptTrans;


	private:  // data members

		std::unique_ptr<CoreAcceptTrans> coreAcceptTrans_;

	public:   // methods

		explicit AcceptTrans(
			const CoreAcceptTrans&       coreAcceptTrans);

		AcceptTrans(
			AcceptTrans&&                acceptTrans);

		~AcceptTrans();

		Iterator begin() const;
		Iterator end() const;
	};


	class DownAccessor
	{
	public:  // data types

		class Iterator
		{
		private:  // data types

			using CoreIterator = ExplicitTreeAutCoreUtil::DownAccessorIterator;

		private:  // data types

			std::unique_ptr<CoreIterator> coreDownAccessIter_;

		public:   // methods

			explicit Iterator(const ExplicitTreeAut& aut);
			explicit Iterator(const CoreIterator& coreIter);
			Iterator(const Iterator& iter);
			~Iterator();

			bool operator==(const Iterator& rhs) const;
			bool operator!=(const Iterator& rhs) const;
			Transition operator*() const;

			/**
			 * @brief  Prefix increment operator
			 */
			Iterator& operator++();
		};

		using iterator         = Iterator;
		using const_iterator   = Iterator;

		using CoreDownAccessor = ExplicitTreeAutCoreUtil::DownAccessor;

	private:  // data members

		std::unique_ptr<CoreDownAccessor> coreDownAccessor_;

	public:   // methods

		explicit DownAccessor(
			const CoreDownAccessor&      coreDownAccessor);

		DownAccessor(
			DownAccessor&&               downAccessor);

		~DownAccessor();

		Iterator begin() const;
		Iterator end() const;

		bool empty() const;
	};


private:  // data members

	std::unique_ptr<CoreAut> core_;

public:   // methods

	ExplicitTreeAut();
	ExplicitTreeAut(
            const ExplicitTreeAut& aut,
	        bool                   copyTrans = true,
	        bool                   copyFinal = true);
	ExplicitTreeAut(ExplicitTreeAut&& aut);

	ExplicitTreeAut& operator=(const ExplicitTreeAut& rhs);
	ExplicitTreeAut& operator=(ExplicitTreeAut&& rhs);

	~ExplicitTreeAut();

	explicit ExplicitTreeAut(CoreAut&& core);


	static StringSymbolType ToStringSymbolType(const std::string& str, size_t rank)
	{
		return StringRank(str, rank);
	}

	static SymbolType GetZeroSymbol()
	{
		return 0;
	}

	/**
	 * @brief  Sets a state as an accepting state
	 *
	 * @param[in]  state  The state to be set as accepting
	 */
	void SetStateFinal(const StateType& state);

	void SetStatesFinal(const std::set<StateType>& states);

	/**
	 * @brief  Checks whether a state is accepting
	 *
	 * @param[in]  state  The state to be checked
	 *
	 * @returns  @p true in the case @p state is accepting, @p false otherwise
	 */
	bool IsStateFinal(const StateType& state) const;

	/**
	 * @brief  Retrieves the set of accepting states
	 *
	 * @returns  The set of accepting states of the automaton
	 */
	const FinalStateSet& GetFinalStates() const;

	/**
	 * @brief  Clears the set of final states
	 */
	void EraseFinalStates();

	/**
	 * @brief  Retrieves a container with accepting transitions
	 *
	 * @returns  An (iterable) container with accepting transitions
	 */
	AcceptTrans GetAcceptTrans() const;

  /**
    * @brief Retrieves a container with all states of the automaton
    *
    * @return A vector with all used states
    */
  std::unordered_set<size_t> GetUsedStates() const;

  /**
    * @brief Clears automaton. It clears its final states and trasitions.
    */
  void Clear();

	/**
	 * @brief  Retrieves the transitions where the state is a parent
	 *
	 * This method retrieves an iterable container of transitions where @p state
	 * is the parent state.
	 *
	 * @param[in]  state  The parent state of the transitions to be retrieved
	 *
	 * @returns  A container of transitions going from @p state down
	 */
	DownAccessor GetDown(
		const StateType&          state) const;

	/**
	 * @brief  Retrieves the transitions where the state is a parent
	 *
	 * This method retrieves an iterable container of transitions where @p state
	 * is the parent state.
	 *
	 * @param[in]  state  The parent state of the transitions to be retrieved
	 *
	 * @returns  A container of transitions going from @p state down
	 */
	DownAccessor operator[](
		const StateType&          state) const;

	void AddTransition(
		const StateTuple&         children,
		const SymbolType&         symbol,
		const StateType&          state);

	void AddTransition(
		const Transition&         trans);

	bool ContainsTransition(
		const Transition&         trans) const;


	bool ContainsTransition(
		const StateTuple&         children,
		const SymbolType&         symbol,
		const StateType&          state) const;

    bool AreTransitionsEmpty();

	AlphabetType& GetAlphabet();


	const AlphabetType& GetAlphabet() const;


	/**
	 * @brief  Reduces the automaton while preserving its language
	 *
	 * This method provides the preferred way for reducing the automaton w.r.t.
	 * the number of states and transitions while preserving its language.
	 *
	 * @returns  An automaton which is a reduced version of the current object
	 */
	ExplicitTreeAut Reduce() const;


	/**
	 * @brief  Reduces the automaton while preserving its language
	 *
	 * This method reduces the automaton according to the parameters passed in
	 * the @p params argument.
	 *
	 * @param[in]  params  Parameters setting the reduction method
	 *
	 * @returns  An automaton which is a reduced version of the current object
	 *            w.r.t. the parameters
	 */
	ExplicitTreeAut Reduce(
		const VATA::ReduceParam&    params) const;


	/**
	 * @brief  Collapses states according to the passed map
	 *
	 * This method renames the states occuring in the automaton according to the
	 * @p collapseMap. @p collapseMap does not need to be injective, i.e. there
	 * may be more than one state mapping to the same state, in which case the two
	 * states are effectively collapsed into one.
	 *
	 * @param[in]  collapseMap  The map according to which the collapsing is done
	 *
	 * @returns  An automaton with states collapsed according to @p collapseMap
	 */
	ExplicitTreeAut CollapseStates(
		const StateToStateMap&      collapseMap) const;


	void BuildStateIndex(
	  Util::TranslatorWeak<StateMap>&    index) const;


	ExplicitTreeAut ReindexStates(
		StateToStateTranslWeak&     stateTransl) const;


	ExplicitTreeAut ReindexStates(
		AbstractReindexF&           fctor,
		bool                        addFinalStates = true) const;


	void ReindexStates(
		ExplicitTreeAut&            dst,
		AbstractReindexF&           fctor,
		bool                        addFinalStates = true) const;


	void CopyTransitionsFrom(
		const ExplicitTreeAut&      src,
		AbstractCopyF&              fctor);


	ExplicitTreeAut RemoveUnreachableStates(
		StateToStateMap*            pTranslMap = nullptr) const;


	template <
		class Rel,
		class Index = Util::IdentityTranslator<AutBase::StateType>
	>
	ExplicitTreeAut RemoveUnreachableStates(
		const Rel&                           rel,
		const Index&                         index = Index()) const;


	ExplicitTreeAut RemoveUselessStates(
		StateToStateMap*          pTranslMap = nullptr) const;


	ExplicitTreeAut GetCandidateTree() const;


	void SetAlphabet(AlphabetType& alphabet);


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&     serializer,
		const std::string&                        params = "") const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&     serializer,
		const StateDict&                          stateDict,
		const std::string&                        params = "") const;


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&     serializer,
		const StateBackTranslStrict&              stateTransl,
		const std::string&                        params = "") const;


	AutDescription DumpToAutDesc(
		const std::string&                        params = "") const;


	AutDescription DumpToAutDesc(
		const StateDict&                          stateDict,
		const std::string&                        params = "") const;


	AutDescription DumpToAutDesc(
		const StateBackTranslStrict&              stateTransl,
		const std::string&                        params = "") const;


	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;


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


	void LoadFromString(
		VATA::Parsing::AbstrParser&       parser,
		const std::string&                str,
		const std::string&                params = "");


	void LoadFromString(
		VATA::Parsing::AbstrParser&       parser,
		const std::string&                str,
		StateDict&                        stateDict,
		const std::string&                params = "");


	void LoadFromString(
		VATA::Parsing::AbstrParser&       parser,
		const std::string&                str,
		StringToStateTranslWeak&          stateTransl,
		const std::string&                params = "");


	void LoadFromAutDesc(
		const VATA::Util::AutDescription&   desc,
		const std::string&                  params = "");


	void LoadFromAutDesc(
		const VATA::Util::AutDescription&   desc,
		StateDict&                          stateDict,
		const std::string&                  params = "");


	void LoadFromAutDesc(
		const VATA::Util::AutDescription&   desc,
		StringToStateTranslWeak&            stateTransl,
		const std::string&                  params = "");


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
		StateToStateMap*                      pTranslMapLhs = nullptr,
		StateToStateMap*                      pTranslMapRhs = nullptr);


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
		AutBase::ProductTranslMap*        pTranslMap = nullptr);


	/**
	 * @brief  Intersection of languages of a pair of automata in bottom-up way
	 *
	 * This function creates an automaton that accepts the languages defined as
	 * the intersection of langauges of a pair of automata in bottom-up way.
	 *
	 * @param[in]   lhs             Left automaton
	 * @param[in]   rhs             Right automaton
	 * @param[out]  pTranslMapLhs   Dictionary for the result
	 *
	 * @returns  An automaton accepting the intersection of languages of @p lhs
	 * and @p rhs
     */
	static ExplicitTreeAut IntersectionBU(
		const ExplicitTreeAut&            lhs,
		const ExplicitTreeAut&            rhs,
		AutBase::ProductTranslMap*        pTranslMap = nullptr);


	/**
	 * @brief  Dispatcher for calling correct inclusion checking function
	 *
	 * This function is a dispatcher that calls a proper inclusion checking
	 * function between @p smaller and @p bigger according to the parameters in @p
	 * params.
	 *
	 * @param[in]  smaller  The smaller automaton
	 * @param[in]  bigger   The bigger automaton
	 * @param[in]  params   Parameters for the inclusion
	 *
	 * @returns  @p true if the language of @p smaller is a subset of the language
	 *           of @p bigger, @p false otherwise
	 */
	static bool CheckInclusion(
		const ExplicitTreeAut&                 smaller,
		const ExplicitTreeAut&                 bigger,
		const VATA::InclParam&                 params);


	/**
	 * @brief  Checks inclusion using default parameters
	 *
	 * This static method checks language inclusion of a pair of automata (@p
	 * smaller, @p bigger) using default parameters.
	 *
	 * @param[in]  smaller  The smaller automaton
	 * @param[in]  bigger   The bigger automaton
	 *
	 * @returns  @p true if the language of @p smaller is a subset of the language
	 *           of @p bigger, @p false otherwise
	 */
	static bool CheckInclusion(
		const ExplicitTreeAut&                 smaller,
		const ExplicitTreeAut&                 bigger);


	/**
	 * @brief  Computes the specified simulation relation on the automaton
	 *
	 * This method computes the simulation relation specified in the @p params
	 * structure among the states of the automaton. The relation is output as a
	 * matrix, indexed from 0. The mapping of indices of columns and rows to the
	 * states of the automaton is output by the @p transl translator.
	 *
	 * @param[in]  params  Parameters specifying which simulation is to be computed.
	 *
	 * @returns  The computed simulation relation, as a matrix indexed from 0
	 */
	AutBase::StateDiscontBinaryRelation ComputeSimulation(
		const VATA::SimParam&                  params) const;


	/**
	 * @brief  Computes the complement of the automaton
	 *
	 * This method computes the complement of the automaton with respect to the
	 * alphabet associated with the automaton.
	 *
	 * @returns  The complement of the automaton
	 */
	ExplicitTreeAut Complement() const;


	/**
	 * @brief  Checks language emptiness
	 *
	 * Determines whether the language of the automaton is empty.
	 *
	 * @returns  @p true if the language of the automaton is empty, @p false
	 *           otherwise
	 */
	bool IsLangEmpty() const;

	/**
	 * @brief  Translates all symbols according to a translator
	 *
	 * This method translates symbols of all transitions according to the @p
	 * transl functor passed to the method. The changed automaton is returned.
	 *
	 * @param[in,out]  transl  The functor for the translation
	 *
	 * @returns  The automaton with symbols in transitions changed
	 */
	ExplicitTreeAut TranslateSymbols(
		AbstractSymbolTranslateF&       transl) const;

	std::string ToString(const Transition& trans) const;
};

#endif
