#ifndef _VATA_EXPLICIT_FINITE_AUT_HH_
#define _VATA_EXPLICIT_FINITE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>

#include <vata/incl_param.hh>

#include <vata/util/convert.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>
#include <vata/util/transl_weak.hh>


namespace VATA
{
	class ExplicitFiniteAut;

	template <class>
		class LoadableAut;

	class ExplicitFiniteAutCore;
}

GCC_DIAG_OFF(effc++) // non virtual destructors warnings suppress
class VATA::ExplicitFiniteAut : public AutBase
{
GCC_DIAG_ON(effc++)

public: // public data types
	using SymbolType       = uintptr_t ;
	using SymbolSet        = std::unordered_set<SymbolType>;
	using StringSymbolType = std::string;

private: // private data types
	typedef std::string string;
	using CoreAut = VATA::LoadableAut<ExplicitFiniteAutCore>;
	std::unique_ptr<CoreAut> core_; 

public: // alphabet
	using SymbolDict                     = 
		VATA::Util::TwoWayDict<string, SymbolType>;
	using SymbolBackTranslStrict         =
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

	using AlphabetType = std::shared_ptr<Alphabet>;

public: // Constructors and operators
	ExplicitFiniteAut();
	ExplicitFiniteAut(const ExplicitFiniteAut& aut);
	ExplicitFiniteAut(ExplicitFiniteAut&& aut);

	ExplicitFiniteAut& operator=(const ExplicitFiniteAut& rhs);
	ExplicitFiniteAut& operator=(ExplicitFiniteAut& rhs);

	~ExplicitFiniteAut();

	explicit ExplicitFiniteAut(CoreAut&& aut);

public: // loading automaton methods
	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		const std::string&               params = "");

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
		const std::string&               params = "");
	void LoadFromString(
		VATA::Parsing::AbstrParser&      parser,
		const std::string&               str,
		StringToStateTranslWeak&         stateTransl,
		const std::string&               params = "");

	/*
	 * Loads to internal (explicit) representation from the structure given by
	 * parser
	 */
	void LoadFromAutDesc(
		const AutDescription&            desc,
		const std::string&               params = "");
	void LoadFromAutDesc(
		const AutDescription&            desc,
		StringToStateTranslWeak&         stateTransl,
		const std::string&               params = "");
	void LoadFromAutDesc(
		const AutDescription&            desc,
		StateDict&                       stateDict,
		const std::string&               params = "");

	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&			serializer,
		StringToStateTranslWeak&                  stateTransl,
		const std::string&                        params = "") const;

	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&			serializer,
		const StateDict&                          stateDict,
		const std::string&                        params = "") const;

	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&			serializer,
		const std::string&                        params = "") const;

public: // public methods
	void SetStateFinal(const StateType& state);
	void SetStateStart(const StateType& state, const SymbolType& symbol);
	void SetExistingStateStart(const StateType& state, const SymbolType& symbol);
	const SymbolSet& GetStartSymbols(StateType state) const;

	AlphabetType& GetAlphabet();
	AlphabetType& GetAlphabet() const;
	const StateSet& GetStartStates() const;
	ExplicitFiniteAut ReindexStates (StateToStateTranslWeak& stateTransl) const;

	template <
		class TranslIndex,
		class SanitizeIndex>
	std::string PrintSimulationMapping(
		TranslIndex          index,
		SanitizeIndex        sanitizeIndex);


	ExplicitFiniteAut RemoveUnreachableStates(
			VATA::AutBase::StateToStateMap* pTranslMap = nullptr);
	ExplicitFiniteAut RemoveUselessStates(
			VATA::AutBase::StateToStateMap* pTranslMap = nullptr);
	ExplicitFiniteAut GetCandidateTree() const;

	/**
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

	ExplicitFiniteAut Reverse(
			AutBase::StateToStateMap* pTranslMap = nullptr) const;

	template <class Dict>
	friend ExplicitFiniteAut Complement(
			const Dict &) 
	{
		throw NotImplementedException(__func__);
	}

	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		size_t              size);

	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		size_t                  /* size */) const
	{
		throw NotImplementedException(__func__);
	}
};
#endif
