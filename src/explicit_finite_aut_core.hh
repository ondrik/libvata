/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Header file for explicit representation of finite automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_AUT_CORE_HH_
#define _VATA_EXPLICIT_FINITE_AUT_CORE_HH_

// VATA headers
#include <vata/aut_base.hh>
#include <vata/explicit_finite_aut.hh>

#include <vata/util/transl_weak.hh>
#include <vata/explicit_lts.hh>
#include <vata/incl_param.hh>

// Standard library headers
#include <unordered_set>

namespace VATA
{
	class ExplicitFiniteAutCore;
}

/*
 * There is no template needed, because
 * symbol is not ranked
 */
GCC_DIAG_OFF(effc++) // non virtual destructors warnings suppress
class VATA::ExplicitFiniteAutCore : public AutBase
{
GCC_DIAG_ON(effc++)

	/*
	 * Friend functions
	 */

	template <class SymbolType, class Dict>
	friend ExplicitFiniteAutCore Complement(
			const ExplicitFiniteAutCore &,
			const Dict &);

	friend bool CheckEquivalence(
			 const ExplicitFiniteAutCore& smaller,
			 const ExplicitFiniteAutCore& bigger,
			 const InclParam& params);

	/*
	 * Functors for inclusion checking functions
	 */
	template<class Rel>
	friend class ExplicitFAAbstractFunctor;

	template<class Rel>
	friend class ExplicitFAInclusionFunctor;
	template<class Rel, class Comparator>
	friend class ExplicitFAInclusionFunctorOpt;

	template<class Rel, class Comparator>
	friend class ExplicitFAInclusionFunctorCache;

	template<class Rel>
	friend class ExplicitFACongrFunctor;
	template<class Rel>
	friend class ExplicitFACongrFunctorOpt;
	template<class Rel, class ProductSet, class NormalFormRel>
	friend class ExplicitFACongrFunctorCacheOpt;
	template<class Rel, class ProductSet>
	friend class ExplicitFACongrEquivFunctor;

	template<class Rel>
	friend class ExplicitFAStateSetComparator;
	template<class Rel>
	friend class ExplicitFAStateSetComparatorIdentity;
	template<class Rel>
	friend class ExplicitFAStateSetComparatorSimulation;

	template<class Rel>
	friend class NormalFormRelPreorder;
	template<class Rel>
	friend class NormalFormRelSimulation;

	template<class Key, class Value>
	friend class MapToList;
	template<class Aut>
	friend class MacroStateCache;

public:
	using SymbolType               = ExplicitFiniteAut::SymbolType;
	using SymbolSet                = ExplicitFiniteAut::SymbolSet;
	using StringSymbolType         = ExplicitFiniteAut::StringSymbolType;
	using SymbolBackTranslStrict   = ExplicitFiniteAut::SymbolBackTranslStrict;

	using AbstractAlphabet = ExplicitFiniteAut::AbstractAlphabet;

private: // private type definitions

	typedef std::string string;

	using Convert      = VATA::Util::Convert;

	using AlphabetType = ExplicitFiniteAut::AlphabetType;

	using StateSet     = ExplicitFiniteAut::StateSet;

	typedef AutDescription::State State;

	typedef std::unordered_map<StateType,SymbolSet> StateToSymbols;

	// The states on the right side of transitions
	typedef StateSet RStateSet;

	/*
	 * Transition cluster maps symbol to set of states when
	 * there exists transition for a given symbol to the states.
	 */
	GCC_DIAG_OFF(effc++)
	class TransitionCluster : public std::unordered_map<SymbolType,RStateSet>{
	GCC_DIAG_ON(effc++)
	public:
		 RStateSet& uniqueRStateSet(const SymbolType &symbol){
			auto iter = this->find(symbol);
			if (iter == this->end()) {
				return this->insert(std::make_pair(symbol, RStateSet())).first->second;
			}
			else {
			 return iter->second;
			}
		}
	};
	typedef std::shared_ptr<TransitionCluster> TransitionClusterPtr;

	/*
	 * Maps a state to transition cluster, so all transitions
	 * for given state are stored in the transition cluster.
	 */
	GCC_DIAG_OFF(effc++)
	class StateToTransitionClusterMap :
		public std::unordered_map<StateType,TransitionClusterPtr>{
	GCC_DIAG_ON(effc++)
	public:
		const TransitionClusterPtr &uniqueCluster(const StateType &state){
			auto& clusterPtr = this->insert(
				std::make_pair(state,TransitionClusterPtr(nullptr))
					).first->second;
				if (!clusterPtr){
					clusterPtr = TransitionClusterPtr(new TransitionCluster());
				}
				else if (!clusterPtr.unique()) {
					clusterPtr = TransitionClusterPtr(new TransitionCluster(*clusterPtr));
				}
				return clusterPtr;
		}
	};

	typedef std::shared_ptr<StateToTransitionClusterMap> StateToTransitionClusterMapPtr;

protected: // private data memebers

	StateSet finalStates_;
	StateSet startStates_;
	// Transitions defining start states
	StateToSymbols startStateToSymbols_;

private: // private static data memebers

	StateToTransitionClusterMapPtr transitions_;

	AlphabetType alphabet_;

	static AlphabetType globalAlphabet_;

public:

	explicit ExplicitFiniteAutCore(AlphabetType& alphabet = globalAlphabet_);

	ExplicitFiniteAutCore(const ExplicitFiniteAutCore& aut);

	ExplicitFiniteAutCore& operator=(const ExplicitFiniteAutCore& rhs);

	~ExplicitFiniteAutCore()
	{ }

	/*
	** Creating internal representation of automaton from
	** AutoDescription structure.
	** @param desc AutoDescription structure
	** @param stateTranslator Translates states to internal number representation
	** @param symbolTranslator Translates symbols to internal number representation
	*/
	template <
		class StateTranslFunc,
		class SymbolTranslFunc>
	void loadFromAutDescInternal(
		const AutDescription&            desc,
		StateTranslFunc&                 stateTransl,
		SymbolTranslFunc&                symbolTransl,
		const std::string&               /* params */ = "")
	{
		// Load symbols
		for (auto symbolRankPair : desc.symbols){ // Symbols translater
			symbolTransl(symbolRankPair.first);
		}

		// Load final states
		for (auto s : desc.finalStates) { // Finale states extraction
			this->finalStates_.insert(stateTransl(s));
		}

		// Load transitions
		for (auto t : desc.transitions) {
			const std::string& symbol = t.second;
			const State& rightState = t.third;

			// Check whether there are no start states
			if (t.first.empty()) {
				StateType translatedState = stateTransl(rightState);

				SymbolType translatedSymbol = symbolTransl(symbol);

				SetStateStart(translatedState, translatedSymbol);

				continue;
			}

			if (t.first.size() != 1) { // symbols only with arity one
				throw std::runtime_error("Not a finite automaton");
			}

			// load here because t could be empty
			const State& leftState = t.first[0];

			this->AddTransition(
				stateTransl(leftState),
				symbolTransl(symbol),
				stateTransl(rightState));
		}
	}

protected:  // methods

	/*
	 * Function converts the internal automaton description
	 * to a string.
	 */
	template <
		class StateBackTranslFunc>
	AutDescription dumpToAutDescInternal(
		StateBackTranslFunc&                    stateTransl, // States from internal to string
		const AlphabetType&                     alphabet, // Symbols from internal to string
		const std::string&                      /* params */ = "") const
	{
		assert(nullptr != alphabet);
		AutDescription desc;

		AbstractAlphabet::BwdTranslatorPtr symbolTransl =
			alphabet->GetSymbolBackTransl();
		assert(nullptr != symbolTransl);

		// Dump the final states
		for (auto& s : this->finalStates_)
		{
			desc.finalStates.insert(stateTransl(s));
		}

		// Dump start states
		std::string x("x"); // initial input symbol
		std::vector<std::string> leftStateAsTuple;
		for (auto &s : this->startStates_)
		{

			SymbolSet symset = this->GetStartSymbols(s);

			if (!symset.size())
			{ // No start symbols are defined,
				AutDescription::Transition trans(
					leftStateAsTuple,
					x,
					stateTransl(s));
				desc.transitions.insert(trans);
			}
			else
			{ // print all starts symbols
				for (auto &sym : symset)
				{
					AutDescription::Transition trans(
						leftStateAsTuple,
						(*symbolTransl)(sym),
						stateTransl(s));
					desc.transitions.insert(trans);
					break;
				}
			}
		}

		/*
		 * Converts transitions to data structure for serializer
		 */
		for (auto& ls : *(this->transitions_))
		{
			for (auto& s : *ls.second)
			{
				for (auto& rs : s.second)
				{
					std::vector<std::string> leftStateAsTuple;
					leftStateAsTuple.push_back(stateTransl(ls.first));

					AutDescription::Transition trans(
						leftStateAsTuple,
						(*symbolTransl)(s.first),
						stateTransl(rs));
					desc.transitions.insert(trans);
				}
			}
		}

		return desc;
	}


public:   // methods


	ExplicitFiniteAutCore ReindexStates(
		StateToStateTranslWeak&    stateTransl) const
	{
		ExplicitFiniteAutCore res;
		this->ReindexStates(res, stateTransl);

		return res;
	}


	template <
		class TranslIndex,
		class SanitizeIndex>
	std::string PrintSimulationMapping(
		TranslIndex          index,
		SanitizeIndex        sanitizeIndex)
	{
		std::string res = "";
		std::unordered_set<StateType> translatedStates;

		for (auto& ls : *(this->transitions_))
		{
			for (auto& s : *ls.second)
			{
				for (auto& rs : s.second)
				{
					if (!translatedStates.count(ls.first))
					{
						res = res + VATA::Util::Convert::ToString(index(ls.first)) + " -> " +
							VATA::Util::Convert::ToString(sanitizeIndex[ls.first]) + "\n";
						translatedStates.insert(ls.first);
					}

					if (!translatedStates.count(rs))
					{
						res = res + VATA::Util::Convert::ToString(index(rs)) + " -> " +
							VATA::Util::Convert::ToString(sanitizeIndex[rs]) + "\n";
						translatedStates.insert(rs);
					}
				}
			}
		}

		return res;
	}


	/*
	 * The current indexes for states are transform to the new ones,
	 * and stored to the new automaton
	 */
	template <class Index>
	void ReindexStates(ExplicitFiniteAutCore& dst, Index& index) const {

		// Converts the final states
		for (auto& state : this->finalStates_)
		{
			dst.SetStateFinal(index[state]);
		}

		// Converts the start states
		for( auto& state : this->startStates_) {
			dst.SetExistingStateStart(index[state],GetStartSymbols(state));
		}

		auto clusterMap = dst.uniqueClusterMap();

		/*
		 * Conversion of all states and symbols that are in
		 * transitions.
		 */
		for (auto& stateClusterPair : *this->transitions_) {

			//assert(stateClusterPair.second);

			auto cluster = clusterMap->uniqueCluster(index[stateClusterPair.first]);

			for (auto& symbolRStateSetPair : *stateClusterPair.second) {

				//assert(symbolTupleSetPair.second);

				RStateSet& rstatesSet = cluster->uniqueRStateSet(symbolRStateSetPair.first);

				for (auto& rState : symbolRStateSetPair.second) {
					rstatesSet.insert(index[rState]);
				}
			}
		}
	}



public: // Public static functions

	// Set of start states
	const StateSet& GetStartStates() const {
		return this->startStates_;
	}

	// compability with tree automata part - used by function for complementation
	AlphabetType& GetAlphabet()
	{
		assert(nullptr != alphabet_);

		return alphabet_;
	}

	const AlphabetType& GetAlphabet() const
	{
		assert(nullptr != alphabet_);

		return alphabet_;
	}

public: // Public functions

	void SetStateFinal(const StateType& state) {
		this->finalStates_.insert(state);
	}

	void AddTransition(
		const StateType&           lstate,
		const SymbolType&          symbol,
		const StateType&           rstate)
	{
		this->internalAddTransition(lstate, symbol, rstate);
	}

	bool IsStateFinal(const StateType &state) const
	{
		return (finalStates_.find(state) != finalStates_.end());
	}

	const StateSet& GetFinalStates() const
	{
		return finalStates_;
	}

	bool IsStateStart(const StateType &state) const
	{
		return (startStates_.find(state) != startStates_.end());
	}

public: // Public setter

	void SetStateStart(
		const StateType&       state,
		const SymbolType&      symbol)
	{
		startStates_.insert(state);

		// Add start transition
		if (!startStateToSymbols_.count(state))
		{
			startStateToSymbols_.insert(
				 std::make_pair(state,std::unordered_set<SymbolType>())).
				 first->second.insert(symbol);
		}
		else
		{ // Just add new symbol
			startStateToSymbols_.find(state)->second.insert(symbol);
		}
	}

	// Set start state with set of symbols in start transitions
	void SetExistingStateStart(
		const StateType&        state,
		const SymbolSet&        symbolSet)
	{
		startStates_.insert(state);

		assert(!startStateToSymbols_.count(state));

		startStateToSymbols_.insert(std::make_pair(state,symbolSet));
	}

public: // Getters

	// Return a set of the symbols which are in start transitions
	// for given state
	const SymbolSet& GetStartSymbols(StateType state) const
	{
		assert(startStateToSymbols_.find(state) != startStateToSymbols_.end());

		return startStateToSymbols_.find(state)->second;
	}

protected:

	// Returns pointer to map where state is mapped to transition cluster
	const StateToTransitionClusterMapPtr& uniqueClusterMap()
	{
		if (!transitions_.unique()) {
			transitions_ = StateToTransitionClusterMapPtr(
				new StateToTransitionClusterMap(*transitions_));
		}
		return transitions_;
	}


	/*
	 * Add internal transition to the automaton
	 */
	void internalAddTransition(
		const StateType&         lstate,
		const SymbolType&        symbol,
		const StateType&         rstate)
	{
		this->uniqueClusterMap()->uniqueCluster(lstate)->
			uniqueRStateSet(symbol).insert(rstate);
	}

	/*
	 * Get from tree automata part of library
	 */
	template <class T>
	static const typename T::mapped_type::element_type* genericLookup(
		const T&                         cont,
		const typename T::key_type&      key)
	{
		auto iter = cont.find(key);
		if (iter == cont.end())
			return nullptr;

		return iter->second.get();

	}

public:   // methods

	ExplicitFiniteAutCore RemoveUnreachableStates(
			VATA::AutBase::StateToStateMap* pTranslMap = nullptr);


	ExplicitFiniteAutCore RemoveUselessStates(
			VATA::AutBase::StateToStateMap* pTranslMap = nullptr);

	ExplicitFiniteAutCore GetCandidateTree() const;


	template <class Dict>
	ExplicitFiniteAutCore Complement(
		const                     Dict&) const
	{
		throw NotImplementedException(__func__);
	}


	template <class Index = Util::IdentityTranslator<AutBase::StateType>>
	VATA::ExplicitLTS Translate(
		std::vector<std::vector<size_t>>&     partition,
		VATA::Util::BinaryRelation&           relation,
		const Index&                          stateIndex = Index());


	/*
	 * Creates union of two automata. It just reindexes
	 * existing states of both automata to a new one.
	 * Reindexing of states is not done in this function, this
	 * function just prepares translators.
	 */
	static ExplicitFiniteAutCore Union(
		const ExplicitFiniteAutCore& lhs,
		const ExplicitFiniteAutCore& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr);

	static ExplicitFiniteAutCore UnionDisjointStates(
		const ExplicitFiniteAutCore& lhs,
		const ExplicitFiniteAutCore& rhs);

	static VATA::ExplicitFiniteAutCore Intersection(
			const VATA::ExplicitFiniteAutCore &lhs,
			const VATA::ExplicitFiniteAutCore &rhs,
			AutBase::ProductTranslMap* pTranslMap = nullptr);

	static bool CheckInclusion(
		const VATA::ExplicitFiniteAutCore&    smaller,
		const VATA::ExplicitFiniteAutCore&    bigger,
		const VATA::InclParam&                params);

	// Checking inclusion
	template<class Rel, class Functor>
	static bool CheckFiniteAutInclusion (
		const ExplicitFiniteAutCore& smaller,
		const ExplicitFiniteAutCore& bigger,
		const Rel& preorder);

	ExplicitFiniteAutCore Reverse(
			AutBase::StateToStateMap* pTranslMap = nullptr) const;

	/***************************************************
	 * Simulation functions
	 */
public:   // methods

	template <class Index>
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		size_t              size,
		const Index&        index)
	{
		AutBase::StateBinaryRelation relation;
		std::vector<std::vector<size_t>> partition(1);
		return Translate(partition, relation, index).computeSimulation(size);
	}

	AutBase::StateBinaryRelation ComputeSimulation(
		const SimParam&      /*   params */)
	{
		throw NotImplementedException(__func__);
	}
};
#endif
