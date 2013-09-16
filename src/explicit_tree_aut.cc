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


ExplicitTreeAut::Transition ExplicitTreeAut::Iterator::operator*() const
{
	assert(nullptr != coreIter_);

	return **coreIter_;
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


void ExplicitTreeAut::AddTransition(
	const StateTuple&         children,
	const SymbolType&         symbol,
	const StateType&          state)
{
	assert(nullptr != core_);

	core_->AddTransition(children, symbol, state);
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


AutBase::StateBinaryRelation ExplicitTreeAut::ComputeDownwardSimulation(
	size_t            size) const
{
	assert(nullptr != core_);

	return core_->ComputeDownwardSimulation(size);
}


AutBase::StateBinaryRelation ExplicitTreeAut::ComputeUpwardSimulation(
	size_t            size) const
{
	assert(nullptr != core_);

	return core_->ComputeUpwardSimulation(size);
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


std::string ExplicitTreeAut::ToString(const Transition& trans) const
{
	assert(nullptr != core_);

	return core_->ToString(trans);
}
