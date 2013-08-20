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
ExplicitTreeAutCore::StringToSymbolDict* ExplicitTreeAutCore::pSymbolDict_ = nullptr;

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
	StringToStateDict&                stateDict)
{
	typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
		StateTranslator;
	typedef VATA::Util::TranslatorWeak<StringToSymbolDict>
		SymbolTranslator;

	StateType stateCnt = 0;

	SymbolTranslator symTrans(
		this->GetSymbolDict(),
		[this](const StringRank&){return this->AddSymbol();});

	this->LoadFromAutDesc(
		/* automaton description */ desc,
		/* state translator */ StateTranslator(stateDict,
			[&stateCnt](const std::string&){return stateCnt++;}),
		/* symbol translator */ symTrans
		);
}


void ExplicitTreeAutCore::LoadFromString(
	VATA::Parsing::AbstrParser&       parser,
	const std::string&                str,
	StringToStateDict&                stateDict)
{
	this->LoadFromAutDesc(parser.ParseString(str), stateDict);
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
	> StateDict;

	size_t stateCnt = 0;

	StateDict stateDict;
	Util::TranslatorWeak<StateDict> stateTranslator(
		stateDict, [&stateCnt](const StateType&){ return stateCnt++; }
	);

	this->BuildStateIndex(stateTranslator);

	AutBase::StateBinaryRelation sim = this->ComputeDownwardSimulation(
		stateDict.size(), Util::TranslatorStrict<StateDict>(stateDict)
	);

	return this->CollapseStates(
			sim, Util::TranslatorStrict<StateDict::MapBwdType>(stateDict.GetReverseMap())
		).RemoveUnreachableStates(sim, Util::TranslatorStrict<StateDict>(stateDict)
	);
}


std::string ExplicitTreeAutCore::DumpToString(
	VATA::Serialization::AbstrSerializer&     serializer) const
{
	return this->DumpToString(serializer,
		[](const StateType& state){return Convert::ToString(state);},
		SymbolBackTranslatorStrict(this->GetSymbolDict().GetReverseMap()));
}


std::string ExplicitTreeAutCore::DumpToString(
	VATA::Serialization::AbstrSerializer&     serializer,
	const StringToStateDict&                  stateDict) const
{
	struct SymbolTranslatorPrinter
	{
		const SymbolBackTranslatorStrict& translator;

		explicit SymbolTranslatorPrinter(
			const SymbolBackTranslatorStrict&       transl) :
			translator(transl)
		{ }

		const StringRank& operator()(
			const SymbolType&                       sym) const
		{
			return translator[sym];
		}
	};

	SymbolTranslatorPrinter printer(
		SymbolBackTranslatorStrict(GetSymbolDict().GetReverseMap()));

	return this->DumpToString(serializer,
		StateBackTranslatorStrict(stateDict.GetReverseMap()),
		printer);
}


AutBase::StateBinaryRelation ExplicitTreeAutCore::ComputeUpwardSimulation(
	size_t             size) const
{
	std::vector<std::vector<size_t>> partition;

	AutBase::StateBinaryRelation relation;

	return this->TranslateUpward(
		partition, relation, Util::Identity(size)
	).computeSimulation(partition, relation, size);
}
