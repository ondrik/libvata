/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of the core of a BDD-based top-down tree automaton.
 *
 *****************************************************************************/


#include "bdd_td_tree_aut_core.hh"


using VATA::BDDTDTreeAutCore;


BDDTDTreeAutCore::BDDTDTreeAutCore() :
	finalStates_(),
	transTable_(new TransTable)
{ }


BDDTDTreeAutCore::BDDTDTreeAutCore(
	TransTablePtr                    transTable) :
	finalStates_(),
	transTable_(transTable)
{ }


BDDTDTreeAutCore::BDDTDTreeAutCore(
	const BDDTDTreeAutCore&           aut) :
	finalStates_(aut.finalStates_),
	transTable_(aut.transTable_)
{ }


BDDTDTreeAutCore::BDDTDTreeAutCore(
	BDDTDTreeAutCore&&                aut) :
	finalStates_(std::move(aut.finalStates_)),
	transTable_(std::move(aut.transTable_))
{ }


BDDTDTreeAutCore& BDDTDTreeAutCore::operator=(
	const BDDTDTreeAutCore&           rhs)
{
	if (this != &rhs)
	{
		transTable_ = rhs.transTable_;
		finalStates_ = rhs.finalStates_;
	}

	return *this;
}


BDDTDTreeAutCore& BDDTDTreeAutCore::operator=(
	BDDTDTreeAutCore&&               rhs)
{
	if (this != &rhs)
	{
		finalStates_ = std::move(rhs.finalStates_);
		transTable_ = std::move(rhs.transTable_);
	}

	return *this;
}


void BDDTDTreeAutCore::AddTransition(
	const StateTuple&       children,
	SymbolType              symbol,
	const StateType&        parent)
{
	// Assertions
	assert(symbol.length() == SYMBOL_SIZE);

	addArityToSymbol(symbol, children.size());
	assert(symbol.length() == SYMBOL_TOTAL_SIZE);

	if (transTable_.unique())
	{
		UnionApplyFunctor unioner;

		const TransMTBDD& oldMtbdd = GetMtbdd(parent);
		TransMTBDD addedMtbdd(symbol, StateTupleSet(children), StateTupleSet());
		SetMtbdd(parent, unioner(oldMtbdd, addedMtbdd));
	}
	else
	{	// copy on write
		assert(false);              // fail gracefully
	}
}


std::string BDDTDTreeAutCore::DumpToDot() const
{
	std::vector<const TransMTBDD*> stateVec;
	for (auto stateBddPair : GetStates())
	{
		stateVec.push_back(&GetMtbdd(stateBddPair.first));
	}

	return TransMTBDD::DumpToDot(stateVec);
}


void BDDTDTreeAutCore::ReindexStates(
	BDDTDTreeAutCore&           dstAut,
	StateToStateTranslator&     stateTrans) const
{
	GCC_DIAG_OFF(effc++)    // suppress missing virtual destructor warning
	class RewriteApplyFunctor :
		public VATA::MTBDDPkg::Apply1Functor<RewriteApplyFunctor,
		StateTupleSet, StateTupleSet>
	{
	GCC_DIAG_ON(effc++)
	private:  // data members

		StateToStateTranslator trans_;

	public:   // methods

		RewriteApplyFunctor(StateToStateTranslator& trans) :
			trans_(trans)
		{ }

		StateTupleSet ApplyOperation(const StateTupleSet& value)
		{
			StateTupleSet result;

			for (const StateTuple& tuple : value)
			{ // for every tuple
				StateTuple resTuple;
				for (StateTuple::const_iterator itTup = tuple.begin();
					itTup != tuple.end(); ++itTup)
				{
					resTuple.push_back(trans_(*itTup));
				}

				result.insert(resTuple);
			}

			return result;
		}
	};

	RewriteApplyFunctor rewriter(stateTrans);
	for (auto stateBddPair : GetStates())
	{
		StateType newState = stateTrans(stateBddPair.first);
		dstAut.SetMtbdd(newState, rewriter(stateBddPair.second));
	}

	for (const StateType& fst : GetFinalStates())
	{
		dstAut.SetStateFinal(stateTrans(fst));
	}
}


void BDDTDTreeAutCore::LoadFromAutDesc(
	const AutDescription&         desc,
	StringToStateDict&            stateDict)
{
	typedef VATA::Util::TranslatorWeak<AutBase::StringToStateDict>
		StateTranslator;
	typedef VATA::Util::TranslatorWeak<StringToSymbolDict>
		SymbolTranslator;

	StateType stateCnt = 0;

	this->LoadFromAutDesc(desc,
		StateTranslator(stateDict,
			[&stateCnt](const std::string&){return stateCnt++;}),
		SymbolTranslator(this->GetSymbolDict(),
			[this](const std::string&){return AddSymbol();}));
}


void BDDTDTreeAutCore::LoadFromString(
	VATA::Parsing::AbstrParser&      parser,
	const std::string&               str,
	StringToStateDict&               stateDict)
{
	this->LoadFromAutDesc(parser.ParseString(str), stateDict);
}


std::string BDDTDTreeAutCore::DumpToString(
	VATA::Serialization::AbstrSerializer&      serializer,
	const StringToStateDict&                   stateDict) const
{
	return this->DumpToString(serializer,
		StateBackTranslatorStrict(stateDict.GetReverseMap()),
		SymbolBackTranslatorStrict(this->GetSymbolDict().GetReverseMap()));
}


std::string BDDTDTreeAutCore::DumpToString(
	VATA::Serialization::AbstrSerializer&      serializer) const
{
	return this->DumpToString(serializer,
		[](const StateType& state){return Convert::ToString(state);},
		SymbolBackTranslatorStrict(this->GetSymbolDict().GetReverseMap()));
}
