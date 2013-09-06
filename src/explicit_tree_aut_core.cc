/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of the core of an explicitly represented tree automaton.
 *
 *****************************************************************************/


#include "explicit_tree_aut_core.hh"
#include "explicit_tree_sim.hh"
#include "explicit_tree_transl.hh"
#include "explicit_tree_unreach.hh"


using VATA::AutBase;
using VATA::ExplicitTreeAutCore;
using VATA::Util::Convert;


// global tuple cache definition
ExplicitTreeAutCore::TupleCache ExplicitTreeAutCore::globalTupleCache_;

// global alphabet
ExplicitTreeAutCore::AlphabetType ExplicitTreeAutCore::globalAlphabet_ =
	AlphabetType(new AlphabetType::element_type());

ExplicitTreeAutCore::ExplicitTreeAutCore(
	TupleCache&          tupleCache,
	AlphabetType&        alphabet) :
	cache_(tupleCache),
	finalStates_(),
	transitions_(StateToTransitionClusterMapPtr(new StateToTransitionClusterMap())),
	alphabet_(alphabet)
{ }


ExplicitTreeAutCore::ExplicitTreeAutCore(
	const ExplicitTreeAutCore&    aut) :
	cache_(aut.cache_),
	finalStates_(aut.finalStates_),
	transitions_(aut.transitions_),
	alphabet_(aut.alphabet_)
{ }


ExplicitTreeAutCore::ExplicitTreeAutCore(
	ExplicitTreeAutCore&&         aut) :
	cache_(aut.cache_),
	finalStates_(std::move(aut.finalStates_)),
	transitions_(std::move(aut.transitions_)),
	alphabet_(std::move(aut.alphabet_))
{ }


ExplicitTreeAutCore::ExplicitTreeAutCore(
	const ExplicitTreeAutCore&    aut,
	TupleCache&                   tupleCache) :
	cache_(tupleCache),
	finalStates_(aut.finalStates_),
	transitions_(aut.transitions_),
	alphabet_(aut.alphabet_)
{ }


ExplicitTreeAutCore& ExplicitTreeAutCore::operator=(
	const ExplicitTreeAutCore&    rhs)
{
	if (this != &rhs)
	{
		finalStates_ = rhs.finalStates_;
		transitions_ = rhs.transitions_;
		alphabet_    = rhs.alphabet_;
		// NOTE: we don't care about cache_
	}

	return *this;
}


ExplicitTreeAutCore& ExplicitTreeAutCore::operator=(
	ExplicitTreeAutCore&&         rhs)
{
	assert(this != &rhs);

	finalStates_ = std::move(rhs.finalStates_);
	transitions_ = std::move(rhs.transitions_);
	alphabet_    = std::move(rhs.alphabet_);
	// NOTE: we don't care about cache_

	return *this;
}


ExplicitTreeAutCore ExplicitTreeAutCore::Reduce() const
{
	typedef Util::TwoWayDict<
		StateType,
		StateType,
		std::unordered_map<StateType, StateType>,
		std::unordered_map<StateType, StateType>
	> StateMap;

	size_t stateCnt = 0;

	StateMap stateMap;
	Util::TranslatorWeak<StateMap> stateTranslator(
		stateMap, [&stateCnt](const StateType&){ return stateCnt++; }
	);

	this->BuildStateIndex(stateTranslator);

	AutBase::StateBinaryRelation sim = this->ComputeDownwardSimulation(
		stateMap.size(), Util::TranslatorStrict<StateMap>(stateMap)
	);

	return this->CollapseStates(
			sim, Util::TranslatorStrict<StateMap::MapBwdType>(stateMap.GetReverseMap())
		).RemoveUnreachableStates(sim, Util::TranslatorStrict<StateMap>(stateMap)
	);
}


std::string ExplicitTreeAutCore::DumpToString(
	VATA::Serialization::AbstrSerializer&     serializer,
	const std::string&                        params) const
{
	return this->DumpToString(
		serializer,
		[](const StateType& state){return Convert::ToString(state);},
		params);
}


std::string ExplicitTreeAutCore::DumpToString(
	VATA::Serialization::AbstrSerializer&     serializer,
	const StateDict&                          stateDict,
	const std::string&                        params) const
{
	struct SymbolTranslPrinter
	{
		const SymbolBackTranslStrict& translator;

		explicit SymbolTranslPrinter(
			const SymbolBackTranslStrict&       transl) :
			translator(transl)
		{ }

		const StringRank& operator()(
			const SymbolType&                       sym) const
		{
			return translator[sym];
		}
	};

	SymbolTranslPrinter printer(this->GetAlphabet()->GetSymbolBackTransl());

	return this->dumpToStringInternal(
		serializer,
		StateBackTranslStrict(stateDict.GetReverseMap()),
		printer,
		params);
}


#if 0
std::string ExplicitTreeAutCore::DumpToString(
	VATA::Serialization::AbstrSerializer&  serializer,
	const StateBackTranslStrict&           stateTransl,
	const SymbolBackTranslStrict&          symbolTransl,
	const std::string&                     /* params */) const
{
	AutDescription desc;

	for (const StateType& s : finalStates_)
	{
		desc.finalStates.insert(stateTransl(s));
	}

	for (const Transition& t : *this)
	{
		std::vector<std::string> tupleStr;

		for (const StateType& s : t.children())
		{
			tupleStr.push_back(stateTrans(s));
		}

		AutDescription::Transition trans(
			tupleStr,
			symbolTrans(t.symbol()).symbolStr,
			stateTrans(t.parent()));

		desc.transitions.insert(trans);
	}

	return serializer.Serialize(desc);
}
#endif


AutBase::StateBinaryRelation ExplicitTreeAutCore::ComputeUpwardSimulation(
	size_t             size) const
{
	std::vector<std::vector<size_t>> partition;

	AutBase::StateBinaryRelation relation;

	return this->TranslateUpward(
		partition, relation, Util::Identity(size)
	).computeSimulation(partition, relation, size);
}
