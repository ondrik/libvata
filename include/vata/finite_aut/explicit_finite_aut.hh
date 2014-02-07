/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Header file for explicit representation of finite automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_AUT_HH_
#define _VATA_EXPLICIT_FINITE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>
#include <vata/util/convert.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/util/transl_weak.hh>
#include <vata/explicit_lts.hh>
#include <vata/incl_param.hh>

// Standard library headers
#include <unordered_set>

namespace VATA {
	class ExplicitFiniteAut;
}

/*
 * There is no template needed, because
 * symbol is not ranked
 */
GCC_DIAG_OFF(effc++) // non virtual destructors warnings suppress
class VATA::ExplicitFiniteAut : public AutBase
{
GCC_DIAG_ON(effc++)

	/*
	 * Friend functions
	 */

	template <class SymbolType, class Dict>
	friend ExplicitFiniteAut Complement(
			const ExplicitFiniteAut &,
			const Dict &);

	friend bool CheckEquivalence(
			 const ExplicitFiniteAut& smaller,
			 const ExplicitFiniteAut& bigger,
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
	using SymbolType         = uintptr_t ;
	using StringSymbolType   = std::string;

private: // private type definitions
	typedef AutBase::StateType StateType;

	typedef std::string string;

	using Convert = VATA::Util::Convert;

	// Stateset is unordered_set with operation for checking subset
	GCC_DIAG_OFF(effc++)
	class StateSet : public std::unordered_set<StateType>	 {
	GCC_DIAG_ON(effc++)
	public:
		bool IsSubsetOf(const StateSet& rhs) const {
			for (StateType state : *this) {
				if (!rhs.count(state)) { // counterexample found
					return false;
				}
			}
			return true;
		}
	};

	// Structure given by parser or given to serializer
	typedef VATA::Util::AutDescription AutDescription;

	typedef AutDescription::State State;

	typedef std::unordered_set<SymbolType> SymbolSet;
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

public: //pubic type definitions

	using SymbolDict             = VATA::Util::TwoWayDict<string, SymbolType>;
	using SymbolBackTranslStrict =
		VATA::Util::TranslatorStrict<typename SymbolDict::MapBwdType>;
	using StringSymbolToSymbolTranslWeak = Util::TranslatorWeak<SymbolDict>;


	class Alphabet
	{
	private:  // data members

		SymbolDict symbolDict_{};
		SymbolType nextSymbol_ = 0;

	public:   // methods

		StringSymbolToSymbolTranslWeak GetSymbolTransl()
		{
			return StringSymbolToSymbolTranslWeak{symbolDict_,
				[&](const StringSymbolType&){return nextSymbol_++;}};
		}

		SymbolBackTranslStrict GetSymbolBackTransl()
		{
			return SymbolBackTranslStrict(symbolDict_.GetReverseMap());
		}
	};


	using AlphabetType = std::shared_ptr<Alphabet>;


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

	explicit ExplicitFiniteAut(AlphabetType& alphabet = globalAlphabet_) :
		finalStates_(),
		startStates_(),
		startStateToSymbols_(),
		transitions_(StateToTransitionClusterMapPtr(new StateToTransitionClusterMap)),
		alphabet_(alphabet)
	{ }

	ExplicitFiniteAut(const ExplicitFiniteAut& aut) :
		finalStates_(aut.finalStates_),
		startStates_(aut.startStates_),
		startStateToSymbols_(aut.startStateToSymbols_),
		transitions_(aut.transitions_),
		alphabet_(aut.alphabet_)
	{ }

	ExplicitFiniteAut& operator=(const ExplicitFiniteAut& rhs)
	{
		if (this != &rhs)
		{
			finalStates_          = rhs.finalStates_;
			startStates_          = rhs.startStates_;
			startStateToSymbols_  = rhs.startStateToSymbols_;
			transitions_          = rhs.transitions_;
			alphabet_             = rhs.alphabet_;
		}

		return *this;
	}


	// TODO: move constructor and assignment operator


	~ExplicitFiniteAut()
	{ }

	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		const std::string&               params = "")
	{
		this->LoadFromAutDesc(parser.ParseString(str), params);
	}

	/*
	 ** Function loads automaton to the intern representation
	 ** from the string.
	 ** It translates from string to the automaton descrtiption
	 ** data structure and the calls another function.
	 **
	 */
	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		StateDict&                       stateDict,
		const std::string&               params = "")
	{
		this->LoadFromAutDesc(parser.ParseString(str), stateDict, params);
	}

	template <
		class StateTranslFunc>
	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		StateTranslFunc                  stateTransl,
		const std::string&               params = "")
	{
		this->LoadFromAutDesc(parser.ParseString(str), stateTransl, params);
	}


	void LoadFromAutDesc(
		const AutDescription&            desc,
		const std::string&               params = "")
	{
		StateDict stateDict;

		this->LoadFromAutDesc(desc, stateDict, params);
	}

	template <
		class StateTranslFunc>
	void LoadFromAutDesc(
		const AutDescription&            desc,
		StateTranslFunc                  stateTransl,
		const std::string&               params = "")
	{
		this->loadFromAutDescInternal(
			desc,
			stateTransl,
			this->GetAlphabet()->GetSymbolTransl(),
			params);
	}

	/*
	 * Loads to internal (explicit) representation from the structure given by
	 * parser
	 */
	void LoadFromAutDesc(
		const AutDescription&            desc,
		StateDict&                       stateDict,
		const std::string&               params = "")
	{
		StateType stateCnt = 0;

		this->LoadFromAutDesc(
			desc,
			StringToStateTranslWeak(stateDict,
				[&stateCnt](const std::string&){return stateCnt++;}),
			params);
	}


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
		const AutDescription&					 desc,
		StateTranslFunc							   stateTransl,
		SymbolTranslFunc							 symbolTransl,
		const std::string&						 /* params */ = "")
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


	template <
		class StateTranslFunc>
	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&			serializer,
		StateTranslFunc                           stateTransl,
		const std::string&                        params = "") const
	{
		return this->dumpToStringInternal(
			serializer,
			stateTransl,
			this->GetAlphabet()->GetSymbolBackTransl(),
			params);
	}


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&			serializer,
		const StateDict&                          stateDict,
		const std::string&                        params = "") const
	{
		return this->DumpToString(
			serializer,
			StateBackTranslStrict(stateDict.GetReverseMap()),
			params);
	}


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&			serializer,
		const std::string&                        params = "") const
	{
		return this->DumpToString(
			serializer,
			[](const StateType& state){return Convert::ToString(state);},
			params);
	}


private:  // methods

	/*
	 * Function converts the internal automaton description
	 * to a string.
	 */
	template <
		class StateBackTranslFunc,
		class SymbolBackTranslFunc>
	std::string dumpToStringInternal(
		VATA::Serialization::AbstrSerializer&   serializer,
		StateBackTranslFunc                     stateTransl, // States from internal to string
		SymbolBackTranslFunc                    symbolTransl, // Symbols from internal to string
		const std::string&                      /* params */ = "") const
	{
		AutDescription desc;

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
						symbolTransl(sym),
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
						symbolTransl(s.first),
						stateTransl(rs));
					desc.transitions.insert(trans);
				}
			}
		}

		return serializer.Serialize(desc);
	}


public:   // methods


	ExplicitFiniteAut ReindexStates(
		StateToStateTranslWeak&    stateTransl) const
	{
		ExplicitFiniteAut res;
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
	void ReindexStates(ExplicitFiniteAut& dst, Index& index) const {

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

public: // Public inline functions

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

	ExplicitFiniteAut RemoveUnreachableStates(
			VATA::AutBase::StateToStateMap* pTranslMap = nullptr);


	ExplicitFiniteAut RemoveUselessStates(
			VATA::AutBase::StateToStateMap* pTranslMap = nullptr);

	ExplicitFiniteAut GetCandidateTree() const;


	template <class Dict>
	ExplicitFiniteAut Complement(
		const                     Dict&) const
	{
		throw NotImplementedException(__func__);
	}


	template <class Index = Util::IdentityTranslator<AutBase::StateType>>
	VATA::ExplicitLTS Translate(
		const ExplicitFiniteAut&              aut,
		std::vector<std::vector<size_t>>&     partition,
		VATA::Util::BinaryRelation&           relation,
		const Index&                          stateIndex = Index());


	/*
	 * Creates union of two automata. It just reindexs
	 * existing states of both automata to a new one.
	 * Reindexing of states is not done in this function, this
	 * function just prepares translators.
	 */
	static ExplicitFiniteAut Union(
		const ExplicitFiniteAut& lhs,
		const ExplicitFiniteAut& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr);

	static ExplicitFiniteAut UnionDisjointStates(
		const ExplicitFiniteAut& lhs,
		const ExplicitFiniteAut& rhs);

	static VATA::ExplicitFiniteAut Intersection(
			const VATA::ExplicitFiniteAut &lhs,
			const VATA::ExplicitFiniteAut &rhs,
			AutBase::ProductTranslMap* pTranslMap = nullptr);


	static bool CheckInclusion(
		const VATA::ExplicitFiniteAut&    smaller,
		const VATA::ExplicitFiniteAut&    bigger,
		const VATA::InclParam&						params);

	// Checking inclusion
	template<class Rel, class Functor>
	static bool CheckFiniteAutInclusion (
		const ExplicitFiniteAut& smaller,
		const ExplicitFiniteAut& bigger,
		const Rel& preorder);

	ExplicitFiniteAut Reverse(
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
		return Translate(*this, partition, relation, index).computeSimulation(size);
	}

	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		size_t              size)
	{
		AutBase::StateBinaryRelation relation;
		std::vector<std::vector<size_t>> partition(1);

		return Translate(*this, partition, relation).computeSimulation(partition,relation,size);
	}

	ExplicitFiniteAut Reduce() const
	{
		throw NotImplementedException(__func__);
	}

#if 0
	// Automaton has not been sanitized
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const ExplicitFiniteAut& aut) {

		return ComputeDownwardSimulation(aut, AutBase::SanitizeAutForSimulation(aut));
	}
#endif

	/*****************************************************************
	 * Upward simulation just for compatibility
	 */
	template <class Index>
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		size_t                  /* size */,
		const Index&            /* index */) const
	{
		throw NotImplementedException(__func__);
	}

	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		size_t                  /* size */) const
	{
		throw NotImplementedException(__func__);
	}

	AutBase::StateBinaryRelation ComputeUpwardSimulation() const
	{
		throw NotImplementedException(__func__);
	}
};
#endif
