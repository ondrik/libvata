/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Header file for the ExplicitFiniteAut facade.
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_AUT_HH_
#define _VATA_EXPLICIT_FINITE_AUT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>

#include <vata/incl_param.hh>
#include <vata/sim_param.hh>
#include <vata/explicit_lts.hh>

#include <vata/util/convert.hh>
#include <vata/parsing/abstr_parser.hh>
#include <vata/serialization/abstr_serializer.hh>

#include <vata/util/transl_weak.hh>
#include <vata/util/transl_strict.hh>

namespace VATA
{
	class ExplicitFiniteAut;

	template <class>
		class LoadableAut;

	class ExplicitFiniteAutCore;
}

class VATA::ExplicitFiniteAut : public AutBase
{
private: // private data types

	using CoreAut = VATA::LoadableAut<ExplicitFiniteAutCore>;
	std::unique_ptr<CoreAut> core_;

public: // public data types

	using SymbolType       = uintptr_t ;
	using SymbolSet        = std::unordered_set<SymbolType>;
	using StringSymbolType = std::string;

	using SymbolDict                     =
		VATA::Util::TwoWayDict<std::string, SymbolType>;
	using SymbolBackTranslStrict         =
		VATA::Util::TranslatorStrict<typename SymbolDict::MapBwdType>;
	using StringSymbolToSymbolTranslWeak = Util::TranslatorWeak<SymbolDict>;

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
	};

	using AlphabetType = std::shared_ptr<AbstractAlphabet>;

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

public: // Constructors and operators
	ExplicitFiniteAut();
	ExplicitFiniteAut(const ExplicitFiniteAut& aut);
	ExplicitFiniteAut(ExplicitFiniteAut&& aut);
	explicit ExplicitFiniteAut(CoreAut&& aut);

	ExplicitFiniteAut& operator=(const ExplicitFiniteAut& rhs);
	ExplicitFiniteAut& operator=(ExplicitFiniteAut&& rhs);

	~ExplicitFiniteAut();

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
		StateBackTranslStrict&                    stateTransl,
		const std::string&                        params = "") const;
	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&			serializer,
		const StateDict&                          stateDict,
		const std::string&                        params = "") const;
	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&			serializer,
		const std::string&                        params = "") const;

public: // public methods
	void AddTransition(
		const StateType&           lstate,
		const SymbolType&          symbol,
		const StateType&           rstate);

	void SetStateFinal(const StateType& state);
	void SetStateStart(const StateType& state, const SymbolType& symbol);
	// Set start state with set of symbols in start transitions
	void SetExistingStateStart(const StateType& state, const SymbolSet& symbol);
	const SymbolSet& GetStartSymbols(StateType state) const;

	AlphabetType& GetAlphabet();
	const AlphabetType& GetAlphabet() const;
	const StateSet& GetStartStates() const;
	ExplicitFiniteAut ReindexStates (StateToStateTranslWeak& stateTransl) const;
	template <class Index>
	void ReindexStates(ExplicitFiniteAut& dst, Index& index) const;

	template <
		class TranslIndex,
		class SanitizeIndex>
	std::string PrintSimulationMapping(
		TranslIndex          /* index */,
		SanitizeIndex        /* sanitizeIndex */)
	{
		throw NotImplementedException(__func__);
	}

	ExplicitFiniteAut RemoveUnreachableStates(
			VATA::AutBase::StateToStateMap* pTranslMap = nullptr);
	ExplicitFiniteAut RemoveUselessStates(
			VATA::AutBase::StateToStateMap* pTranslMap = nullptr);
	ExplicitFiniteAut GetCandidateTree() const;

	template <class Index = Util::IdentityTranslator<AutBase::StateType>>
	VATA::ExplicitLTS Translate(
		std::vector<std::vector<size_t>>&     partition,
		Util::BinaryRelation&                 relation,
		const Index&                          stateIndex = Index()) const;

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
		const ExplicitFiniteAut&    smaller,
		const ExplicitFiniteAut&    bigger);

	static bool CheckInclusion(
		const ExplicitFiniteAut&    smaller,
		const ExplicitFiniteAut&    bigger,
		const InclParam&            params);

	ExplicitFiniteAut Reverse(
			AutBase::StateToStateMap* pTranslMap = nullptr) const;

	ExplicitFiniteAut Complement()
	{
		throw NotImplementedException(__func__);
	}

	ExplicitFiniteAut Reduce() const
	{
		throw NotImplementedException(__func__);
	}

	AutBase::StateDiscontBinaryRelation ComputeSimulation(
		const SimParam&            params) const;
};

#endif
