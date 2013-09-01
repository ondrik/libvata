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


// global tuple cache definition
ExplicitTreeAutCore::TupleCache ExplicitTreeAutCore::globalTupleCache_;

// pointer to symbol dictionary
ExplicitTreeAutCore::SymbolDict* ExplicitTreeAutCore::pSymbolDict_ = nullptr;

// pointer to next symbol counter
ExplicitTreeAutCore::SymbolType* ExplicitTreeAutCore::pNextSymbol_ = nullptr;


ExplicitTreeAutCore::ExplicitTreeAutCore(
	TupleCache&          tupleCache) :
	cache_(tupleCache),
	finalStates_(),
	transitions_(StateToTransitionClusterMapPtr(new StateToTransitionClusterMap()))
{ }


ExplicitTreeAutCore::ExplicitTreeAutCore(
	const ExplicitTreeAutCore&    aut) :
	cache_(aut.cache_),
	finalStates_(aut.finalStates_),
	transitions_(aut.transitions_)
{ }


ExplicitTreeAutCore::ExplicitTreeAutCore(
	ExplicitTreeAutCore&&         aut) :
	cache_(aut.cache_),
	finalStates_(std::move(aut.finalStates_)),
	transitions_(std::move(aut.transitions_))
{ }


ExplicitTreeAutCore::ExplicitTreeAutCore(
	const ExplicitTreeAutCore&    aut,
	TupleCache&                   tupleCache) :
	cache_(tupleCache),
	finalStates_(aut.finalStates_),
	transitions_(aut.transitions_)
{ }


ExplicitTreeAutCore& ExplicitTreeAutCore::operator=(
	const ExplicitTreeAutCore&    rhs)
{
	if (this != &rhs)
	{
		finalStates_ = rhs.finalStates_;
		transitions_ = rhs.transitions_;
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
	// NOTE: we don't care about cache_

	return *this;
}


void ExplicitTreeAutCore::LoadFromAutDesc(
	const AutDescription&             desc,
	StateDict&                        stateDict,
	SymbolDict&                       symbolDict,
	const std::string&                params)
{
	StateType state(0);
	SymbolType symbol = ExplicitTreeAut::GetZeroSymbol();

	this->LoadFromAutDescWithStateSymbolTransl(
		desc,
		StringToStateTranslWeak(stateDict,
			[&state](const std::string&){return state++;}),
		StringSymbolToSymbolTranslWeak(symbolDict,
			[&symbol](const StringSymbolType&){return symbol++;}),
		params);
}


void ExplicitTreeAutCore::LoadFromAutDesc(
	const AutDescription&             desc,
	StateDict&                        stateDict,
	const std::string&                params)
{
	this->LoadFromAutDesc(
		/* automaton description */ desc,
		/* state dictionary */ stateDict,
		/* symbol translator */ this->GetSymbolDict(),
		/* parameters */ params);
}


void ExplicitTreeAutCore::LoadFromString(
	VATA::Parsing::AbstrParser&       parser,
	const std::string&                str,
	StateDict&                        stateDict,
	SymbolDict&                       symbolDict,
	const std::string&                params)
{
	this->LoadFromAutDesc(
		parser.ParseString(str),
		stateDict,
		symbolDict,
		params);
}


ExplicitTreeAutCore::AlphabetType ExplicitTreeAutCore::GetAlphabet()
{
	AlphabetType alphabet;
	for (auto stringRankAndSymbolPair : ExplicitTreeAutCore::GetSymbolDict())
	{
		alphabet.push_back(std::make_pair(
			stringRankAndSymbolPair.second,
			stringRankAndSymbolPair.first.rank));
	}

	return alphabet;
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
		SymbolBackTranslStrict(this->GetSymbolDict().GetReverseMap()),
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

	SymbolTranslPrinter printer(
		SymbolBackTranslStrict(GetSymbolDict().GetReverseMap()));

	return this->DumpToString(
		serializer,
		StateBackTranslStrict(stateDict.GetReverseMap()),
		printer,
		params);
}


std::string ExplicitTreeAutCore::DumpToString(
	VATA::Serialization::AbstrSerializer&  serializer,
	const StateDict&                       stateDict,
	const SymbolDict&                      symbolDict,
	const std::string&                     params) const
{
	return this->DumpToString(
		serializer,
		StateBackTranslStrict(stateDict.GetReverseMap()),
		SymbolBackTranslStrict(symbolDict.GetReverseMap()),
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
