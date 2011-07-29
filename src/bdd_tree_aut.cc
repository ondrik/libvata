/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Source file for a BDD-based tree automaton.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_tree_aut.hh>

using VATA::BDDTreeAut;
using VATA::Parsing::AbstrParser;
using VATA::Util::AutDescription;


void BDDTreeAut::copyStates(const BDDTreeAut& src)
{
	// Assertions
	assert(isValid());
	assert(src.isValid());
	assert(haveDisjointStateSets(*this, src));

	states_.insert(states_.end(), src.states_.begin(), src.states_.end());
	finalStates_.insert(finalStates_.end(), src.finalStates_.begin(),
		src.finalStates_.end());

	assert(isValid());
}


void BDDTreeAut::LoadFromString(AbstrParser& parser, const std::string& str)
{
	// Assertions
	assert(hasEmptyStateSet());

	// TODO: enable output of (two-way) translation map

	typedef std::tr1::unordered_map<std::string, StateType> StringToStateMap;

	StringToStateMap transMap;
	AutDescription desc = parser.ParseString(str);

	for (AutDescription::StateSet::const_iterator itStates = desc.states.begin();
		itStates != desc.states.end(); ++itStates)
	{
		StateType inState = AddState();
		states_.push_back(inState);
		transMap.insert(std::make_pair(*itStates, inState));
	}

	assert(isValid());
}


std::string BDDTreeAut::DumpToString(
	VATA::Serialization::AbstrSerializer& serializer)
{
	AutDescription desc;

//		desc.states = states_;
//		desc.finalStates = finalStates_;

	return serializer.Serialize(desc);
}


bool BDDTreeAut::haveDisjointStateSets(const BDDTreeAut& lhs,
	const BDDTreeAut& rhs)
{
	// Assertions
	assert(lhs.isValid());
	assert(lhs.isValid());

	assert(false);
}
