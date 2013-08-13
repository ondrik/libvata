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

using VATA::BDDTopDownTreeAut;


BDDTopDownTreeAut::BDDTopDownTreeAut() :
	finalStates_(),
	transTable_(new TransTable)
{ }


BDDTopDownTreeAut::BDDTopDownTreeAut(
	TransTablePtr                    transTable) :
	finalStates_(),
	transTable_(transTable)
{ }


BDDTopDownTreeAut::BDDTopDownTreeAut(
	const BDDTopDownTreeAut&           aut) :
	finalStates_(aut.finalStates_),
	transTable_(aut.transTable_)
{ }


BDDTopDownTreeAut& BDDTopDownTreeAut::operator=(
	const BDDTopDownTreeAut&           rhs)
{
	if (this != &rhs)
	{
		transTable_ = rhs.transTable_;
		finalStates_ = rhs.finalStates_;
	}

	return *this;
}


std::string BDDTopDownTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer&      serializer) const
{
	return this->DumpToString(serializer,
		[](const StateType& state){return Convert::ToString(state);},
		SymbolBackTranslatorStrict(this->GetSymbolDict().GetReverseMap()));
}
