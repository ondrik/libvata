#include <vata/vata.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>

#include "explicit_finite_aut_core.hh"
#include "loadable_aut.hh"

using VATA::AutBase;
using VATA::ExplicitFiniteAut;

ExplicitFiniteAut::ExplicitFiniteAut() : 
	core_(new CoreAut(CoreAut::ParentAut()))
{ }

ExplicitFiniteAut::ExplicitFiniteAut(const ExplicitFiniteAut& aut) :
	core_(new CoreAut(*aut.core_))
{ }

ExplicitFiniteAut::ExplicitFiniteAut(ExplicitFiniteAut&& aut) :
	core_(std::move(aut_.core));
{
	aut.core_ = nullptr;
}

ExplicitFiniteAut::ExplicitFiniteAut(const CoreAut&& core) :
	core_(new CoreAut(std::move(core)));
{ }


ExplicitFiniteAut::ExplicitFiniteAut& operator=(const ExplicitFiniteAut& rhs) 
{
	if (this != &aut)
	{
		assert(nullptr != core_);

		*core_ = *aut.core_;
	}

	return *this;
}

ExplicitFiniteAut::ExplicitFiniteAut& operator=(ExplicitFiniteAut& rhs) 
{
	assert(this != &rhs);

	assert(nullptr != core_);
	assert(nullptr != aut.core_);

	core_ = std::move(aut.core_);

	return *this;
}

void ExplicitFiniteAut::SetStateFinal(const StateType& state) 
{
	assert(nullptr != core_);
	core_->SetStateFinal(state);
}

void ExplicitFiniteAut::SetStateStart(
	const StateType&       state,
	const SymbolType&      symbol)
{
	assert(nullptr != core_);
	core_->SetStateStart(state,symbol);
}

// Set start state with set of symbols in start transitions
void ExplicitFiniteAut::SetExistingStateStart(
	const StateType&        state,
	const SymbolSet&        symbolSet)
{
	assert(nullptr != core_);
	core_->SetStateStart(state,symbol);
}


const SymbolSet& ExplicitFiniteAut::GetStartSymbols(StateType state) const
{
	assert(nullptr != core_);
	return core_->GetStartSymbols(state);
}

void ExplicitFiniteAut::AddTransition(
	const StateType&           lstate,
	const SymbolType&          symbol,
	const StateType&           rstate)
{
	assert(nullptr != core_);

	core_->AddTransition(lstate, symbol, rstate);
}

AlphabetType& ExplicitFiniteAut::GetAlphabet()
{
	assert(nullptr != core_);

	return core_->GetAlphabet();
}

const AlphabetType& ExplicitFiniteAut::GetAlphabet() const
{
	assert(nullptr != core_);

	return core_->GetAlphabet();
}

ExplicitFiniteAut ExplicitFiniteAut::ReindexStates(
	StateToStateTranslWeak&    stateTransl) const
{
	assert(nullptr != core_);

	return ExplicitFiniteAut(core_->ReindexStates(stateTransl));
}

const StateSet& ExplcitFiniteAut::GetStartStates() const {
	assert(nullptr != core_);

	return core_->GetStartStates();
}
	
template <
	class TranslIndex,
	class SanitizeIndex>
std::string ExplicitFiniteAut::PrintSimulationMapping(
	TranslIndex          index,
	SanitizeIndex        sanitizeIndex)
{
	assert(nullptr != core_);
	core_->PrintSimulationMapping(index,sanitizeIndex);
}

void LoadFromString(
	VATA::Parsing::AbstrParser&      parser,
	const std::string&               str,
	const std::string&               params)
{
	core_->LoadFromAutDesc(parser.ParseString(str), params);
}

/*
 ** Function loads automaton to the intern representation
 ** from the string.
 ** It translates from string to the automaton descrtiption
 ** data structure and the calls another function.
 **
 */
void ExplicitFiniteAut::LoadFromString(
	VATA::Parsing::AbstrParser&      parser,
	const std::string&               str,
	StateDict&                       stateDict,
	const std::string&               params)
{
	core_->LoadFromAutDesc(parser,str, stateDict, params);
}

void ExplicitFiniteAut::LoadFromString(
	VATA::Parsing::AbstrParser&      parser,
	const std::string&               str,
	StringToStateTranslWeak&         stateTransl,
	const std::string&               params)
{
	core_->LoadFromAutDesc(parser,str, stateTransl, params);
}


void ExplicitFiniteAut::LoadFromAutDesc(
	const AutDescription&            desc,
	const std::string&               params)
{
	assert(nullptr != core_);
	core_->LoadFromAutDesc(desc, params);
}

void ExplicitFiniteAut::LoadFromAutDesc(
	const AutDescription&            desc,
	StringToStateTranslWeak&         stateTransl,
	const std::string&               params)
{
	assert(nullptr != core_);
	core_->LoadFromAutDesc(
		desc,
		stateTransl,
		params);
}

/*
 * Loads to internal (explicit) representation from the structure given by
 * parser
 */
void ExplicitFiniteAut::LoadFromAutDesc(
	const AutDescription&            desc,
	StateDict&                       stateDict,
	const std::string&               params)
{
	assert(nullptr != core_);
	core_->LoadFromAutDesc(
		desc,
		stateDict,
		params);
}

std::string ExplicitFiniteAut::DumpToString(
	VATA::Serialization::AbstrSerializer&			serializer,
	StringToStateTranslWeak&                  stateTransl,
	const std::string&                        params) const
{
	assert(nullptr != core_);
	return core_->DumpToString(serializer,stateTransl,params);
}

std::string ExplicitFiniteAut::DumpToString(
	VATA::Serialization::AbstrSerializer&			serializer,
	const StateDict&                          stateDict,
	const std::string&                        params) const
{
	assert(nullptr != core_);
	return core_->DumpToString(serializer,stateDict,params);
}

std::string ExplicitFiniteAut::DumpToString(
	VATA::Serialization::AbstrSerializer&			serializer,
	const std::string&                        params) const
{
	assert(nullptr != core_);
	return core_->DumpToString(serializer,params);
}

ExplicitFiniteAut ExplicitFiniteAut::RemoveUnreachableStates(
	VATA::AutBase::StateToStateMap*   pTranslMap)
{
	assert(nullptr != core_);
	return ExplicitFiniteAut(core_->RemoveUnreachableStates(pTranslMap));
}

ExplicitFiniteAut ExplicitFiniteAut::RemoveUselessStates(
	VATA::AutBase::StateToStateMap*   pTranslMap)
{
	assert(nullptr != core_);
	return ExplicitFiniteAut(core_->RemoveUselessStates(pTranslMap));
}

ExplicitFiniteAut ExplicitFiniteAut::GetCandidateTree() const
{
	assert(nullptr != core_);
	return ExplicitFiniteAut(core_->GetCandidateTree());
}

static ExplicitFiniteAut ExplicitFiniteAut::Union(
		const ExplicitFiniteAut&        lhs,
		const ExplicitFiniteAut&        rhs,
		AutBase::StateToStateMap*       pTranslMapLhs,
		AutBase::StateToStateMap*       pTranslMapRhs)
{
	assert(nullptr != lhs_);
	assert(nullptr != rhs_);
	return ExplicitFiniteAut(CoreAut::Union(
			*lhs.core_,*rhs.core_,pTranslMapLhs,pTranslMapRhs));
}

static ExplicitFiniteAut ExplicitFiniteAut::UnionDisjointStates(
	const ExplicitFiniteAut&          lhs,
	const ExplicitFiniteAut&          rhs)
{
	assert(nullptr != lhs_);
	assert(nullptr != rhs_);
	return ExplicitFiniteAut(CoreAut::UnionDisjointStates(
			*lhs.core_,*rhs.core_));
}

static ExplicitFiniteAut ExplicitFiniteAut::Intersection(
		const VATA::ExplicitFiniteAut   &lhs,
		const VATA::ExplicitFiniteAut   &rhs,
		AutBase::ProductTranslMap* pTranslMap)
{
	assert(nullptr != lhs_);
	assert(nullptr != rhs_);
	return ExplicitFiniteAut(CoreAut::Intersection(
				*lhs.core_,*rhs.core_,pTranslMap));
}

static bool ExplicitFiniteAut::CheckInclusion(
	const VATA::ExplicitFiniteAut&    smaller,
	const VATA::ExplicitFiniteAut&    bigger,
	const VATA::InclParam&						params)
{
	assert(nullptr != smaller);
	assert(nullptr != bigger);
	return ExplicitFiniteAut(CoreAut::CheckInclusion(
				*smaller.core_,*bigger.core_,params));
}

ExplicitFiniteAut ExplicitFiniteAut::Reverse(
		AutBase::StateToStateMap* pTranslMap) const
{
	assert(nullptr != core_);
	return ExplicitFiniteAut(core_->Reverse(pTranslMap));
}

AutBase::StateBinaryRelation ComputeDownwardSimulation(
	size_t                            size) const
{
	assert(nullptr != core_);
	return core_->ComputeDownwardSimulation(size)
}
