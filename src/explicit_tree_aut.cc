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
#include <vata/ta_expl/explicit_tree_aut.hh>

#include "explicit_tree_aut_core.hh"

using VATA::AutBase;
using VATA::ExplicitTreeAut;
using VATA::ExplicitTreeAutCore;


void ExplicitTreeAut::SetSymbolDictPtr(StringToSymbolDict* pSymbolDict)
{
	ExplicitTreeAutCore::SetSymbolDictPtr(pSymbolDict);
}


void ExplicitTreeAut::SetNextSymbolPtr(SymbolType* pNextSymbol)
{
	ExplicitTreeAutCore::SetNextSymbolPtr(pNextSymbol);
}


ExplicitTreeAut::ExplicitTreeAut() :
	core_(new ExplicitTreeAutCore())
{ }


ExplicitTreeAut::ExplicitTreeAut(
	ExplicitTreeAutCore&&             core) :
	core_(new ExplicitTreeAutCore(std::move(core)))
{ }


ExplicitTreeAut::ExplicitTreeAut(
	const ExplicitTreeAut&         aut) :
	core_(new ExplicitTreeAutCore(*aut.core_))
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


void ExplicitTreeAut::LoadFromString(
	VATA::Parsing::AbstrParser&       parser,
	const std::string&                str,
	StringToStateDict&                stateDict)
{
	assert(nullptr != core_);

	core_->LoadFromString(parser, str, stateDict);
}


std::string ExplicitTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&     serializer) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer);
}


std::string ExplicitTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&     serializer,
	const StringToStateDict&                  stateDict) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer, stateDict);
}


std::string ExplicitTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&      serializer,
	const StateBackTranslatorStrict&           stateTrans,
	const SymbolBackTranslatorStrict&          symbolTrans) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer, stateTrans, symbolTrans);
}


ExplicitTreeAut ExplicitTreeAut::ReindexStates(
	StateToStateTranslator&     stateTrans) const
{
	assert(nullptr != core_);

	return ExplicitTreeAut(core_->ReindexStates(stateTrans));
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
		ExplicitTreeAutCore::Union(*lhs.core_, *rhs.core_, pTranslMapLhs, pTranslMapRhs));
}


ExplicitTreeAut ExplicitTreeAut::UnionDisjointStates(
	const ExplicitTreeAut&                lhs,
	const ExplicitTreeAut&                rhs)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return ExplicitTreeAut(
		ExplicitTreeAutCore::UnionDisjointStates(*lhs.core_, *rhs.core_));
}


ExplicitTreeAut ExplicitTreeAut::Intersection(
	const ExplicitTreeAut&            lhs,
	const ExplicitTreeAut&            rhs,
	AutBase::ProductTranslMap*        pTranslMap)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return ExplicitTreeAut(
		ExplicitTreeAutCore::Intersection(*lhs.core_, *rhs.core_, pTranslMap));
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

	return ExplicitTreeAutCore::CheckInclusion(*smaller.core_, *bigger.core_, params);
}
