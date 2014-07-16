/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Source file for an explicit tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_tree_aut.hh>

#include "explicit_tree_aut_core.hh"
#include "loadable_aut.hh"

using VATA::AutBase;
using VATA::ExplicitTreeAut;
using VATA::Util::AutDescription;


ExplicitTreeAut::Iterator::Iterator(const Iterator& iter) :
	coreIter_(new CoreIterator(*iter.coreIter_))
{
	assert(nullptr != coreIter_);
}


ExplicitTreeAut::Iterator::Iterator(const CoreIterator& coreIter) :
	coreIter_(new CoreIterator(coreIter))
{
	assert(nullptr != coreIter_);
}


ExplicitTreeAut::Iterator::~Iterator()
{
	assert(nullptr != coreIter_);
}


bool ExplicitTreeAut::Iterator::operator!=(const Iterator& rhs) const
{
	assert(nullptr != coreIter_);
	assert(nullptr != rhs.coreIter_);

	return *coreIter_ != *rhs.coreIter_;
}


ExplicitTreeAut::Iterator& ExplicitTreeAut::Iterator::operator++()
{
	assert(nullptr != coreIter_);

	++(*coreIter_);

	return *this;
}


const ExplicitTreeAut::Transition& ExplicitTreeAut::Iterator::operator*() const
{
	assert(nullptr != coreIter_);

	return **coreIter_;
}

ExplicitTreeAut::AcceptTrans::AcceptTrans(
	const CoreAcceptTrans&       coreAcceptTrans) :
	coreAcceptTrans_(new CoreAcceptTrans(coreAcceptTrans))
{
	assert(nullptr != coreAcceptTrans_);
}

ExplicitTreeAut::AcceptTrans::AcceptTrans(
	AcceptTrans&&                acceptTrans) :
	coreAcceptTrans_(std::move(acceptTrans.coreAcceptTrans_))
{
	assert(nullptr != coreAcceptTrans_);
	assert(nullptr == acceptTrans.coreAcceptTrans_);
}

ExplicitTreeAut::AcceptTrans::Iterator::Iterator(const CoreIterator& coreIter) :
	coreAcceptTransIter_(new CoreIterator(coreIter))
{
	assert(nullptr != coreAcceptTransIter_);
}

ExplicitTreeAut::AcceptTrans::~AcceptTrans()
{ }

ExplicitTreeAut::AcceptTrans::Iterator::~Iterator()
{
	assert(nullptr != coreAcceptTransIter_);
}


ExplicitTreeAut::AcceptTrans::Iterator ExplicitTreeAut::AcceptTrans::begin() const
{
	assert(nullptr != coreAcceptTrans_);

	return Iterator(coreAcceptTrans_->begin());
}


ExplicitTreeAut::AcceptTrans::Iterator ExplicitTreeAut::AcceptTrans::end() const
{
	assert(nullptr != coreAcceptTrans_);

	return Iterator(coreAcceptTrans_->end());
}


const ExplicitTreeAut::Transition&
ExplicitTreeAut::AcceptTrans::Iterator::operator*() const
{
	assert(nullptr != coreAcceptTransIter_);

	return **coreAcceptTransIter_;
}


const ExplicitTreeAut::Transition*
ExplicitTreeAut::AcceptTrans::Iterator::operator->() const
{
	assert(nullptr != coreAcceptTransIter_);

	return coreAcceptTransIter_->operator->();
}


ExplicitTreeAut::ExplicitTreeAut() :
	core_(new CoreAut(CoreAut::ParentAut()))
{ }


ExplicitTreeAut::ExplicitTreeAut(
	CoreAut&&                    core) :
	core_(new CoreAut(std::move(core)))
{ }


ExplicitTreeAut::ExplicitTreeAut(
	const ExplicitTreeAut&         aut) :
	core_(new CoreAut(*aut.core_))
{ }


ExplicitTreeAut& ExplicitTreeAut::operator=(
	const ExplicitTreeAut&         aut)
{
	if (this != &aut)
	{
		assert(nullptr != core_);

		*core_ = *aut.core_;
	}

	return *this;
}

ExplicitTreeAut::ExplicitTreeAut(
	ExplicitTreeAut&&              aut) :
	core_(std::move(aut.core_))
{
	aut.core_ = nullptr;
}

ExplicitTreeAut& ExplicitTreeAut::operator=(
	ExplicitTreeAut&&         aut)
{
	assert(this != &aut);

	assert(nullptr != core_);
	assert(nullptr != aut.core_);

	core_ = std::move(aut.core_);
	// aut.core_ set to nullptr in std::move()

	return *this;
}


ExplicitTreeAut::~ExplicitTreeAut()
{ }


void ExplicitTreeAut::SetAlphabet(AlphabetType& alphabet)
{
	assert(nullptr != core_);

	core_->SetAlphabet(alphabet);
}


ExplicitTreeAut::AlphabetType& ExplicitTreeAut::GetAlphabet()
{
	assert(nullptr != core_);

	return core_->GetAlphabet();
}


const ExplicitTreeAut::AlphabetType& ExplicitTreeAut::GetAlphabet() const
{
	assert(nullptr != core_);

	return core_->GetAlphabet();
}


ExplicitTreeAut::iterator ExplicitTreeAut::begin()
{
	assert(nullptr != core_);

	return iterator(core_->begin());
}


ExplicitTreeAut::const_iterator ExplicitTreeAut::begin() const
{
	assert(nullptr != core_);

	return const_iterator(core_->begin());
}


ExplicitTreeAut::iterator ExplicitTreeAut::end()
{
	assert(nullptr != core_);

	return iterator(core_->end());
}


ExplicitTreeAut::const_iterator ExplicitTreeAut::end() const
{
	assert(nullptr != core_);

	return const_iterator(core_->end());
}


void ExplicitTreeAut::SetStateFinal(const StateType& state)
{
	assert(nullptr != core_);

	core_->SetStateFinal(state);
}


bool ExplicitTreeAut::IsStateFinal(const StateType& state) const
{
	assert(nullptr != core_);

	return core_->IsStateFinal(state);
}

const ExplicitTreeAut::FinalStateSet& ExplicitTreeAut::GetFinalStates() const
{
	assert(nullptr != core_);

	return core_->GetFinalStates();
}

void ExplicitTreeAut::EraseFinalStates()
{
	assert(nullptr != core_);

	return core_->EraseFinalStates();
}

ExplicitTreeAut::AcceptTrans ExplicitTreeAut::GetAcceptTrans() const
{
	assert(nullptr != core_);

	return AcceptTrans(core_->GetAcceptTrans());
}


ExplicitTreeAut::AcceptTrans::Iterator&
	ExplicitTreeAut::AcceptTrans::Iterator::operator++()
{
	assert(nullptr != coreAcceptTransIter_);

	++(*coreAcceptTransIter_);

	return *this;
}


bool ExplicitTreeAut::AcceptTrans::Iterator::operator==(
	const AcceptTrans::Iterator&     rhs) const
{
	assert(nullptr != coreAcceptTransIter_);
	assert(nullptr != rhs.coreAcceptTransIter_);

	return (*coreAcceptTransIter_ == *rhs.coreAcceptTransIter_);
}

bool ExplicitTreeAut::AcceptTrans::Iterator::operator!=(
	const AcceptTrans::Iterator&     rhs) const
{
	assert(nullptr != coreAcceptTransIter_);
	assert(nullptr != rhs.coreAcceptTransIter_);

	return (*coreAcceptTransIter_ != *rhs.coreAcceptTransIter_);
}

void ExplicitTreeAut::AddTransition(
	const StateTuple&         children,
	const SymbolType&         symbol,
	const StateType&          state)
{
	assert(nullptr != core_);

	core_->AddTransition(children, symbol, state);
}


ExplicitTreeAut::DownAccessor ExplicitTreeAut::operator[](
	const StateType&           state) const
{
	assert(nullptr != core_);

	return DownAccessor((*core_)[state]);
}

ExplicitTreeAut::DownAccessor::DownAccessor(
	const CoreDownAccessor&      coreDownAccessor) :
	coreDownAccessor_(new CoreDownAccessor(coreDownAccessor))
{ }

ExplicitTreeAut::DownAccessor::Iterator
ExplicitTreeAut::DownAccessor::begin() const
{
	assert(nullptr != coreDownAccessor_);

	return Iterator(coreDownAccessor_->begin());
}


ExplicitTreeAut::DownAccessor::Iterator
ExplicitTreeAut::DownAccessor::end() const
{
	assert(nullptr != coreDownAccessor_);

	return Iterator(coreDownAccessor_->end());
}

bool ExplicitTreeAut::DownAccessor::empty() const
{
	assert(nullptr != coreDownAccessor_);

	return coreDownAccessor_->empty();
}

ExplicitTreeAut::DownAccessor::Iterator::Iterator(
	const CoreIterator&          coreIter) :
	coreDownAccessIter_(new CoreIterator(coreIter))
{ }

const ExplicitTreeAut::Transition&
ExplicitTreeAut::DownAccessor::Iterator::operator*() const
{
	assert(nullptr != coreDownAccessIter_);

	return **coreDownAccessIter_;
}

const ExplicitTreeAut::Transition*
ExplicitTreeAut::DownAccessor::Iterator::operator->() const
{
	assert(nullptr != coreDownAccessIter_);

	return coreDownAccessIter_->operator->();
}


ExplicitTreeAut::DownAccessor::Iterator&
ExplicitTreeAut::DownAccessor::Iterator::operator++()
{
	assert(nullptr != coreDownAccessIter_);

	++(*coreDownAccessIter_);

	return *this;
}

ExplicitTreeAut::DownAccessor::Iterator::~Iterator()
{ }

ExplicitTreeAut::DownAccessor::~DownAccessor()
{ }

bool ExplicitTreeAut::DownAccessor::Iterator::operator==(
	const Iterator&           rhs) const
{
	assert(nullptr != coreDownAccessIter_);
	assert(nullptr != rhs.coreDownAccessIter_);

	return *coreDownAccessIter_ == *rhs.coreDownAccessIter_;
}

bool ExplicitTreeAut::DownAccessor::Iterator::operator!=(
	const Iterator&           rhs) const
{
	assert(nullptr != coreDownAccessIter_);
	assert(nullptr != rhs.coreDownAccessIter_);

	return *coreDownAccessIter_ != *rhs.coreDownAccessIter_;
}

bool ExplicitTreeAut::ContainsTransition(
	const Transition&         trans) const
{
	assert(nullptr != core_);

	return core_->ContainsTransition(trans);
}


void ExplicitTreeAut::LoadFromString(
	VATA::Parsing::AbstrParser&       parser,
	const std::string&                str,
	const std::string&                params)
{
	assert(nullptr != core_);

	core_->LoadFromString(parser, str, params);
}


void ExplicitTreeAut::LoadFromString(
	VATA::Parsing::AbstrParser&       parser,
	const std::string&                str,
	StringToStateTranslWeak&          stateTransl,
	const std::string&                params)
{
	assert(nullptr != core_);

	core_->LoadFromString(parser, str, stateTransl, params);
}


void ExplicitTreeAut::LoadFromString(
	VATA::Parsing::AbstrParser&      parser,
	const std::string&               str,
	StateDict&                       stateDict,
	const std::string&               params)
{
	assert(nullptr != core_);

	core_->LoadFromString(parser, str, stateDict, params);
}


void ExplicitTreeAut::LoadFromAutDesc(
	const VATA::Util::AutDescription&   desc,
	StateDict&                          stateDict,
	const std::string&                  params)
{
	assert(nullptr != core_);

	core_->LoadFromAutDesc(desc, stateDict, params);
}


std::string ExplicitTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&     serializer,
	const std::string&                        params) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer, params);
}


std::string ExplicitTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&     serializer,
	const StateDict&                          stateDict,
	const std::string&                        params) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer, stateDict, params);
}


std::string ExplicitTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&  serializer,
	const StateBackTranslStrict&           stateTransl,
	const std::string&                     params) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer, stateTransl, params);
}


AutDescription ExplicitTreeAut::DumpToAutDesc(
	const std::string&                        params) const
{
	assert(nullptr != core_);

	return core_->DumpToAutDesc(params);
}


AutDescription ExplicitTreeAut::DumpToAutDesc(
	const StateDict&                          stateDict,
	const std::string&                        params) const
{
	assert(nullptr != core_);

	return core_->DumpToAutDesc(stateDict, params);
}


AutDescription ExplicitTreeAut::DumpToAutDesc(
	const StateBackTranslStrict&           stateTransl,
	const std::string&                     params) const
{
	assert(nullptr != core_);

	return core_->DumpToAutDesc(stateTransl, params);
}


void ExplicitTreeAut::CopyTransitionsFrom(
	const ExplicitTreeAut&      src,
	AbstractCopyF&              fctor)
{
	assert(nullptr != core_);
	assert(nullptr != src.core_);

	core_->CopyTransitionsFrom(*src.core_, fctor);
}


ExplicitTreeAut ExplicitTreeAut::ReindexStates(
	AbstractReindexF&           fctor,
	bool                        addFinalStates) const
{
	assert(nullptr != core_);

	return ExplicitTreeAut(core_->ReindexStates(fctor, addFinalStates));
}


void ExplicitTreeAut::ReindexStates(
	ExplicitTreeAut&            dst,
	AbstractReindexF&           fctor,
	bool                        addFinalStates) const
{
	assert(nullptr != core_);
	assert(nullptr != dst.core_);

	core_->ReindexStates(*dst.core_, fctor, addFinalStates);
}


ExplicitTreeAut ExplicitTreeAut::ReindexStates(
	StateToStateTranslWeak&     stateTransl) const
{
	assert(nullptr != core_);

	return ExplicitTreeAut(core_->ReindexStates(stateTransl));
}


ExplicitTreeAut ExplicitTreeAut::RemoveUnreachableStates(
	AutBase::StateToStateMap*             pTranslMap) const
{
	assert(nullptr != core_);

	return ExplicitTreeAut(core_->RemoveUnreachableStates(pTranslMap));
}


ExplicitTreeAut ExplicitTreeAut::RemoveUselessStates(
	AutBase::StateToStateMap*             pTranslMap) const
{
	assert(nullptr != core_);

	return ExplicitTreeAut(core_->RemoveUselessStates(pTranslMap));
}


ExplicitTreeAut ExplicitTreeAut::GetCandidateTree() const
{
	assert(nullptr != core_);

	return ExplicitTreeAut(core_->GetCandidateTree());
}


ExplicitTreeAut ExplicitTreeAut::Union(
	const ExplicitTreeAut&                lhs,
	const ExplicitTreeAut&                rhs,
	AutBase::StateToStateMap*             pTranslMapLhs,
	AutBase::StateToStateMap*             pTranslMapRhs)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return ExplicitTreeAut(
		CoreAut::Union(*lhs.core_, *rhs.core_, pTranslMapLhs, pTranslMapRhs));
}


ExplicitTreeAut ExplicitTreeAut::UnionDisjointStates(
	const ExplicitTreeAut&                lhs,
	const ExplicitTreeAut&                rhs)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return ExplicitTreeAut(
		CoreAut::UnionDisjointStates(*lhs.core_, *rhs.core_));
}


ExplicitTreeAut ExplicitTreeAut::Intersection(
	const ExplicitTreeAut&            lhs,
	const ExplicitTreeAut&            rhs,
	AutBase::ProductTranslMap*        pTranslMap)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return ExplicitTreeAut(
		CoreAut::Intersection(*lhs.core_, *rhs.core_, pTranslMap));
}


AutBase::StateBinaryRelation ExplicitTreeAut::ComputeSimulation(
	const VATA::SimParam&                  params) const
{
	assert(nullptr != core_);

	return core_->ComputeSimulation(params);
}


bool ExplicitTreeAut::CheckInclusion(
	const ExplicitTreeAut&                 smaller,
	const ExplicitTreeAut&                 bigger,
	const VATA::InclParam&                 params)
{
	assert(nullptr != smaller.core_);
	assert(nullptr != bigger.core_);

	return CoreAut::CheckInclusion(*smaller.core_, *bigger.core_, params);
}


bool ExplicitTreeAut::CheckInclusion(
	const ExplicitTreeAut&                 smaller,
	const ExplicitTreeAut&                 bigger)
{
	assert(nullptr != smaller.core_);
	assert(nullptr != bigger.core_);

	return CoreAut::CheckInclusion(*smaller.core_, *bigger.core_);
}


ExplicitTreeAut ExplicitTreeAut::Reduce() const
{
	assert(nullptr != core_);

	return ExplicitTreeAut(core_->Reduce());
}


ExplicitTreeAut ExplicitTreeAut::TranslateSymbols(
	AbstractSymbolTranslateF&       transl) const
{
	assert(nullptr != core_);

	return ExplicitTreeAut(core_->TranslateSymbols(transl));
}


std::string ExplicitTreeAut::ToString(const Transition& trans) const
{
	assert(nullptr != core_);

	return core_->ToString(trans);
}
