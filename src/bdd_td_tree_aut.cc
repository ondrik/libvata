/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of a BDD-based top-down tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/bdd_td_tree_aut.hh>

#include "bdd_td_tree_aut_core.hh"
#include "loadable_aut.hh"


using VATA::BDDTopDownTreeAut;


BDDTopDownTreeAut::BDDTopDownTreeAut() :
	core_(new CoreAut(CoreAut::ParentAut()))
{ }

BDDTopDownTreeAut::BDDTopDownTreeAut(
	const BDDTopDownTreeAut&         aut) :
	core_(new CoreAut(*aut.core_))
{ }

BDDTopDownTreeAut::BDDTopDownTreeAut(
	CoreAut&&                    core) :
	core_(new CoreAut(std::move(core)))
{ }

BDDTopDownTreeAut& BDDTopDownTreeAut::operator=(
	const BDDTopDownTreeAut&         aut)
{
	if (this != &aut)
	{
		assert(nullptr != core_);

		*core_ = *aut.core_;
	}

	return *this;
}


BDDTopDownTreeAut::AlphabetType& BDDTopDownTreeAut::GetAlphabet() const
{
	assert(nullptr != core_);

	return core_->GetAlphabet();
}


BDDTopDownTreeAut::BDDTopDownTreeAut(
	BDDTopDownTreeAut&&              aut) :
	core_(std::move(aut.core_))
{
	aut.core_ = nullptr;
}

BDDTopDownTreeAut& BDDTopDownTreeAut::operator=(
	BDDTopDownTreeAut&&         aut)
{
	assert(this != &aut);

	assert(nullptr != core_);
	assert(nullptr != aut.core_);

	core_ = std::move(aut.core_);
	// aut.core_ set to nullptr in std::move()

	return *this;
}


BDDTopDownTreeAut::~BDDTopDownTreeAut()
{ }


void BDDTopDownTreeAut::AddTransition(
	const StateTuple&       children,
	const SymbolType&       symbol,
	const StateType&        parent)
{
	assert(nullptr != core_);

	core_->AddTransition(children, symbol, parent);
}

void BDDTopDownTreeAut::SetStateFinal(
	const StateType&               state)
{
	assert(nullptr != core_);

	core_->SetStateFinal(state);
}


bool BDDTopDownTreeAut::IsStateFinal(const StateType& state) const
{
	assert(nullptr != core_);

	return core_->IsStateFinal(state);
}


const BDDTopDownTreeAut::FinalStateSet& BDDTopDownTreeAut::GetFinalStates() const
{
	assert(nullptr != core_);

	return core_->GetFinalStates();
}


BDDTopDownTreeAut BDDTopDownTreeAut::ReindexStates(
	StateToStateTranslWeak&     stateTrans) const
{
	assert(nullptr != core_);

	return BDDTopDownTreeAut(core_->ReindexStates(stateTrans));
}


BDDTopDownTreeAut BDDTopDownTreeAut::Union(
	const BDDTopDownTreeAut&      lhs,
	const BDDTopDownTreeAut&      rhs,
	AutBase::StateToStateMap*     pTranslMapLhs,
	AutBase::StateToStateMap*     pTranslMapRhs)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return BDDTopDownTreeAut(
		CoreAut::Union(*lhs.core_, *rhs.core_, pTranslMapLhs, pTranslMapRhs));
}


BDDTopDownTreeAut BDDTopDownTreeAut::UnionDisjointStates(
	const BDDTopDownTreeAut&      lhs,
	const BDDTopDownTreeAut&      rhs)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return BDDTopDownTreeAut(
		CoreAut::UnionDisjointStates(*lhs.core_, *rhs.core_));
}


BDDTopDownTreeAut BDDTopDownTreeAut::Intersection(
	const BDDTopDownTreeAut&      lhs,
	const BDDTopDownTreeAut&      rhs,
	AutBase::ProductTranslMap*    pTranslMap)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return BDDTopDownTreeAut(
		CoreAut::Intersection(*lhs.core_, *rhs.core_, pTranslMap));
}


BDDTopDownTreeAut BDDTopDownTreeAut::RemoveUnreachableStates() const
{
	assert(nullptr != core_);

	return BDDTopDownTreeAut(core_->RemoveUnreachableStates());
}


BDDTopDownTreeAut BDDTopDownTreeAut::RemoveUselessStates() const
{
	assert(nullptr != core_);

	return BDDTopDownTreeAut(core_->RemoveUselessStates());
}


bool BDDTopDownTreeAut::CheckInclusion(
	const BDDTopDownTreeAut&    smaller,
	const BDDTopDownTreeAut&    bigger,
	const VATA::InclParam&      params)
{
	assert(nullptr != smaller.core_);
	assert(nullptr != bigger.core_);

	return CoreAut::CheckInclusion(*smaller.core_, *bigger.core_, params);
}


void BDDTopDownTreeAut::LoadFromString(
	VATA::Parsing::AbstrParser&      parser,
	const std::string&               str,
	const std::string&               params)
{
	assert(nullptr != core_);

	core_->LoadFromString(parser, str, params);
}


void BDDTopDownTreeAut::LoadFromString(
	VATA::Parsing::AbstrParser&      parser,
	const std::string&               str,
	StateDict&                       stateDict,
	const std::string&               params)
{
	assert(nullptr != core_);

	core_->LoadFromString(parser, str, stateDict, params);
}


void BDDTopDownTreeAut::LoadFromString(
	VATA::Parsing::AbstrParser&      parser,
	const std::string&               str,
	StringToStateTranslWeak&         stateTransl,
	const std::string&               params)
{
	assert(nullptr != core_);

	core_->LoadFromString(parser, str, stateTransl, params);
}


std::string BDDTopDownTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&      serializer,
	const std::string&                         params) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer, params);
}


std::string BDDTopDownTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&      serializer,
	const StateDict&                           stateDict,
	const std::string&                         params) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer, stateDict, params);
}


std::string BDDTopDownTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&  serializer,
	const StateBackTranslStrict&           stateTransl,
	const std::string&                     params) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer, stateTransl, params);
}
