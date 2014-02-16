/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of a BDD-based bottom-up tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>

#include "bdd_bu_tree_aut_core.hh"
#include "bdd_td_tree_aut_core.hh"
#include "loadable_aut.hh"


using VATA::BDDBottomUpTreeAut;
using VATA::BDDTopDownTreeAut;
using VATA::BDDTDTreeAutCore;

using VATA::Parsing::AbstrParser;
using VATA::Util::AutDescription;
using VATA::Util::Convert;

using StateBinaryRelation = VATA::BDDBottomUpTreeAut::StateBinaryRelation;


BDDBottomUpTreeAut::BDDBottomUpTreeAut() :
	core_(new CoreAut(CoreAut::ParentAut()))
{ }

BDDBottomUpTreeAut::BDDBottomUpTreeAut(
	CoreAut&&                         core) :
	core_(new CoreAut(std::move(core)))
{ }

BDDBottomUpTreeAut::BDDBottomUpTreeAut(
	const BDDBottomUpTreeAut&         aut) :
	core_(new CoreAut(*aut.core_))
{ }

BDDBottomUpTreeAut& BDDBottomUpTreeAut::operator=(
	const BDDBottomUpTreeAut&         aut)
{
	if (this != &aut)
	{
		assert(nullptr != core_);

		*core_ = *aut.core_;
	}

	return *this;
}

BDDBottomUpTreeAut::BDDBottomUpTreeAut(
	BDDBottomUpTreeAut&&              aut) :
	core_(std::move(aut.core_))
{
	aut.core_ = nullptr;
}

BDDBottomUpTreeAut& BDDBottomUpTreeAut::operator=(
	BDDBottomUpTreeAut&&         aut)
{
	assert(this != &aut);

	assert(nullptr != core_);
	assert(nullptr != aut.core_);

	core_ = std::move(aut.core_);
	// aut.core_ set to nullptr in std::move()

	return *this;
}


BDDBottomUpTreeAut::~BDDBottomUpTreeAut()
{ }


BDDBottomUpTreeAut::AlphabetType& BDDBottomUpTreeAut::GetAlphabet() const
{
	// Assertions
	assert(nullptr != core_);

	return core_->GetAlphabet();
}



void BDDBottomUpTreeAut::AddTransition(
	const StateTuple&      children,
	const SymbolType&      symbol,
	const StateType&       parent)
{
	assert(nullptr != core_);

	core_->AddTransition(children, symbol, parent);
}


void BDDBottomUpTreeAut::SetStateFinal(
	const StateType&               state)
{
	assert(nullptr != core_);

	core_->SetStateFinal(state);
}


bool BDDBottomUpTreeAut::IsStateFinal(
	const StateType&               state) const
{
	assert(nullptr != core_);

	return core_->IsStateFinal(state);
}


const BDDBottomUpTreeAut::FinalStateSet& BDDBottomUpTreeAut::GetFinalStates() const
{
	assert(nullptr != core_);

	return core_->GetFinalStates();
}


void BDDBottomUpTreeAut::LoadFromString(
	VATA::Parsing::AbstrParser&     parser,
	const std::string&              str,
	const std::string&              params)
{
	assert(nullptr != core_);

	core_->LoadFromString(parser, str, params);
}


void BDDBottomUpTreeAut::LoadFromString(
	VATA::Parsing::AbstrParser&     parser,
	const std::string&              str,
	StateDict&                      stateDict,
	const std::string&              params)
{
	assert(nullptr != core_);

	core_->LoadFromString(parser, str, stateDict, params);
}


void BDDBottomUpTreeAut::LoadFromString(
	VATA::Parsing::AbstrParser&     parser,
	const std::string&              str,
	StringToStateTranslWeak&        stateTransl,
	const std::string&              params)
{
	assert(nullptr != core_);

	core_->LoadFromString(parser, str, stateTransl, params);
}


void BDDBottomUpTreeAut::LoadFromAutDesc(
	const AutDescription&         desc,
	StateDict&                    stateDict,
	const std::string&            params)
{
	assert(nullptr != core_);

	core_->LoadFromAutDesc(desc, stateDict, params);
}


std::string BDDBottomUpTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&      serializer,
	const std::string&                         params) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer, params);
}


std::string BDDBottomUpTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&      serializer,
	const StateDict&                           stateDict,
	const std::string&                         params) const
{
	assert(nullptr != core_);

	return core_->DumpToString(serializer, stateDict, params);
}


BDDBottomUpTreeAut BDDBottomUpTreeAut::RemoveUselessStates() const
{
	assert(nullptr != core_);

	return BDDBottomUpTreeAut(core_->RemoveUselessStates());
}


BDDBottomUpTreeAut BDDBottomUpTreeAut::RemoveUnreachableStates() const
{
	assert(nullptr != core_);

	return BDDBottomUpTreeAut(core_->RemoveUnreachableStates());
}


StateBinaryRelation BDDBottomUpTreeAut::ComputeSimulation(
	const SimParam&                 params) const
{
	assert(nullptr != core_);

	return core_->ComputeSimulation(params);
}


BDDBottomUpTreeAut BDDBottomUpTreeAut::ReindexStates(
	StateToStateTranslWeak&       trans)
{
	assert(nullptr != core_);

	return BDDBottomUpTreeAut(core_->ReindexStates(trans));
}


bool BDDBottomUpTreeAut::CheckInclusion(
	const BDDBottomUpTreeAut&    smaller,
	const BDDBottomUpTreeAut&    bigger)
{
	assert(nullptr != smaller.core_);
	assert(nullptr != bigger.core_);

	return CoreAut::CheckInclusion(*smaller.core_, *bigger.core_);
}


bool BDDBottomUpTreeAut::CheckInclusion(
	const BDDBottomUpTreeAut&    smaller,
	const BDDBottomUpTreeAut&    bigger,
	const VATA::InclParam&       params)
{
	assert(nullptr != smaller.core_);
	assert(nullptr != bigger.core_);

	return CoreAut::CheckInclusion(*smaller.core_, *bigger.core_, params);
}


BDDBottomUpTreeAut BDDBottomUpTreeAut::Union(
	const BDDBottomUpTreeAut&         lhs,
	const BDDBottomUpTreeAut&         rhs,
	AutBase::StateToStateMap*         pTranslMapLhs,
	AutBase::StateToStateMap*         pTranslMapRhs)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return BDDBottomUpTreeAut(
		CoreAut::Union(*lhs.core_, *rhs.core_, pTranslMapLhs, pTranslMapRhs));
}


BDDBottomUpTreeAut BDDBottomUpTreeAut::UnionDisjointStates(
	const BDDBottomUpTreeAut&         lhs,
	const BDDBottomUpTreeAut&         rhs)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return BDDBottomUpTreeAut(
		CoreAut::UnionDisjointStates(*lhs.core_, *rhs.core_));
}


BDDBottomUpTreeAut BDDBottomUpTreeAut::Intersection(
	const BDDBottomUpTreeAut&         lhs,
	const BDDBottomUpTreeAut&         rhs,
	AutBase::ProductTranslMap*        pTranslMap)
{
	assert(nullptr != lhs.core_);
	assert(nullptr != rhs.core_);

	return BDDBottomUpTreeAut(
		CoreAut::Intersection(*lhs.core_, *rhs.core_, pTranslMap));
}


BDDTopDownTreeAut BDDBottomUpTreeAut::GetTopDownAut() const
{
	assert(nullptr != core_);

	return BDDTopDownTreeAut(core_->GetTopDownAut());
}


BDDBottomUpTreeAut BDDBottomUpTreeAut::GetCandidateTree() const
{
	throw NotImplementedException(__func__);
}
