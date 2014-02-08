/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Implementation of the ExplicitFiniteAut facade.
 *****************************************************************************/

#include <vata/vata.hh>
#include <vata/explicit_finite_aut.hh>

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
	core_(std::move(aut.core_))
{
	aut.core_ = nullptr;
}

ExplicitFiniteAut::ExplicitFiniteAut(CoreAut&& core) :
	core_(new CoreAut(std::move(core)))
{ }


ExplicitFiniteAut& ExplicitFiniteAut::operator=(const ExplicitFiniteAut& rhs)
{
	if (this != &rhs)
	{
		assert(nullptr != core_);

		*core_ = *rhs.core_;
	}

	return *this;
}

ExplicitFiniteAut& ExplicitFiniteAut::operator=(ExplicitFiniteAut&& rhs)
{
	assert(this != &rhs);

	assert(nullptr != core_);
	assert(nullptr != rhs.core_);

	core_ = std::move(rhs.core_);

	return *this;
}

ExplicitFiniteAut::~ExplicitFiniteAut()
{ }

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
	core_->SetExistingStateStart(state,symbolSet);
}


const ExplicitFiniteAut::SymbolSet& ExplicitFiniteAut::GetStartSymbols(
		StateType state) const
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

ExplicitFiniteAut::AlphabetType& ExplicitFiniteAut::GetAlphabet()
{
	assert(nullptr != core_);

	return core_->GetAlphabet();
}

const ExplicitFiniteAut::AlphabetType& ExplicitFiniteAut::GetAlphabet() const
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


template <class Index>
void ExplicitFiniteAut::ReindexStates(ExplicitFiniteAut& dst, Index& index) const
{
	assert(nullptr != core_);
	core_->ReindexStates(*dst.core_,index);
}

const ExplicitFiniteAut::StateSet& ExplicitFiniteAut::GetStartStates() const {
	assert(nullptr != core_);

	return core_->GetStartStates();
}

/*
template <
	class TranslIndex,
	class SanitizeIndex>
std::string ExplicitFiniteAut::PrintSimulationMapping(
	TranslIndex          index,
	SanitizeIndex        sanitizeIndex)
{
	assert(nullptr != core_);
	return core_->PrintSimulationMapping(index,sanitizeIndex);
}
*/
void ExplicitFiniteAut::LoadFromString(
	VATA::Parsing::AbstrParser&      parser,
	const std::string&               str,
	const std::string&               params)
{
	assert(nullptr != core_);
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
	assert(nullptr != core_);
	core_->LoadFromString(parser,str, stateDict, params);
}

void ExplicitFiniteAut::LoadFromString(
	VATA::Parsing::AbstrParser&      parser,
	const std::string&               str,
	StringToStateTranslWeak&         stateTransl,
	const std::string&               params)
{
	assert(nullptr != core_);
	core_->LoadFromString(parser,str, stateTransl, params);
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
	StateBackTranslStrict&                    stateTransl,
	const std::string&                        params) const
{
	assert(nullptr != core_);
	return core_->DumpToString(serializer, stateTransl, params);
}

std::string ExplicitFiniteAut::DumpToString(
	VATA::Serialization::AbstrSerializer&			serializer,
	const StateDict&                          stateDict,
	const std::string&                        params) const
{
	assert(nullptr != core_);
	return core_->DumpToString(serializer, stateDict, params);
}

std::string ExplicitFiniteAut::DumpToString(
	VATA::Serialization::AbstrSerializer&			serializer,
	const std::string&                        params) const
{
	assert(nullptr != core_);
	return core_->DumpToString(serializer, params);
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

template <class Index>
VATA::ExplicitLTS ExplicitFiniteAut::Translate(
	std::vector<std::vector<size_t>>&     partition,
	Util::BinaryRelation&                 relation,
	const Index&                          stateIndex) const
{
	assert(nullptr != core_);

	return core_->Translate(partition,relation,stateIndex);
}

ExplicitFiniteAut ExplicitFiniteAut::Union(
		const ExplicitFiniteAut&        lhs,
		const ExplicitFiniteAut&        rhs,
		AutBase::StateToStateMap*       pTranslMapLhs,
		AutBase::StateToStateMap*       pTranslMapRhs)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);
	return ExplicitFiniteAut(CoreAut::Union(
			*lhs.core_,*rhs.core_,pTranslMapLhs,pTranslMapRhs));
}

ExplicitFiniteAut ExplicitFiniteAut::UnionDisjointStates(
	const ExplicitFiniteAut&          lhs,
	const ExplicitFiniteAut&          rhs)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);
	return ExplicitFiniteAut(CoreAut::UnionDisjointStates(
			*lhs.core_,*rhs.core_));
}

ExplicitFiniteAut ExplicitFiniteAut::Intersection(
		const VATA::ExplicitFiniteAut   &lhs,
		const VATA::ExplicitFiniteAut   &rhs,
		AutBase::ProductTranslMap* pTranslMap)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);
	return ExplicitFiniteAut(CoreAut::Intersection(
				*lhs.core_,*rhs.core_,pTranslMap));
}

bool ExplicitFiniteAut::CheckInclusion(
	const VATA::ExplicitFiniteAut&    smaller,
	const VATA::ExplicitFiniteAut&    bigger,
	const VATA::InclParam&						params)
{
	assert(nullptr != smaller.core_);
	assert(nullptr != bigger.core_);
	return CoreAut::CheckInclusion(
				*smaller.core_,*bigger.core_,params);
}

ExplicitFiniteAut ExplicitFiniteAut::Reverse(
		AutBase::StateToStateMap* pTranslMap) const
{
	assert(nullptr != core_);
	return ExplicitFiniteAut(core_->Reverse(pTranslMap));
}

AutBase::StateBinaryRelation ExplicitFiniteAut::ComputeSimulation(
	const SimParam&             params) const
{
	assert(nullptr != core_);

	return core_->ComputeSimulation(params);
}
