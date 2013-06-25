/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Translate to LTS for explicitly represented finite automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_AUT_TRANSL_
#define _VATA_EXPLICIT_FINITE_AUT_TRANSL_

// VATA headers
#include <vata/vata.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>

#include <vata/explicit_lts.hh>
#include <vata/util/transl_weak.hh>

namespace VATA {
	template <class SymbolType,
	class Index = Util::IdentityTranslator<AutBase::StateType>>
		ExplicitLTS Translate(const ExplicitFiniteAut<SymbolType>& aut,
		std::vector<std::vector<size_t>>& partition,
		Util::BinaryRelation& relation,
		const Index& stateIndex = Index());
}

/*
 * Function translates given nfa to lts and
 * creates partition and set relation
 */
template <class SymbolType, class Index>
VATA::ExplicitLTS VATA::Translate(
	const VATA::ExplicitFiniteAut<SymbolType>& aut,
	std::vector<std::vector<size_t>>& partition,
	VATA::Util::BinaryRelation& relation,
	const Index& stateIndex) {

	VATA::ExplicitLTS res;

	std::unordered_map<SymbolType, size_t> symbolMap;

	size_t symbolCnt = 0;
	// translator for new representation in lts
	Util::TranslatorWeak2<std::unordered_map<SymbolType, size_t>>
		symbolTranslator(symbolMap, [&symbolCnt](const SymbolType&){ return symbolCnt++; });


	// checks whether are all states final
	auto areAllStatesFinal = [&aut]() -> bool {
		for (auto fs : aut.finalStates_) {
			if (!aut.IsStateStart(fs)) {
				return false;
			}
		}
		return true;
	};

	size_t base;
	// When all states are final just two partitions will be created, otherwise three
	if (aut.finalStates_.size()>0 && aut.finalStates_.size() <= aut.transitions_->size() && !areAllStatesFinal()) {
		base = 3;
	}
	else {
		base = 2;
	}

	partition.clear();
	partition.resize(base);

	// Add all final states to the first parition
	for (auto& finalState : aut.finalStates_) {
		partition[0].push_back(stateIndex[finalState]);
	}

	// Add all transitions to LTS
	for (auto stateToCluster : *aut.transitions_) { // left state of transition
		assert(stateToCluster.second);
		size_t leftStateTranslated = stateIndex[stateToCluster.first];

		// non final states to second parition
		if (!aut.IsStateFinal(stateToCluster.first)) {
			partition[base-2].push_back(leftStateTranslated);
		}

		for (auto symbolToSet : *stateToCluster.second) { // symbol of transition

			for (auto setState : symbolToSet.second) { // right state of transition
				assert(setState);

				res.addTransition(leftStateTranslated, symbolTranslator[symbolToSet.first],setState);
			}
		}
	}


	for (auto& startState : aut.GetStartStates()) { // add start transitions
		for (auto& startSymbol : aut.GetStartSymbols(startState)) {
			res.addTransition(aut.transitions_->size()+aut.GetStartStates().size(),
				symbolTranslator[startSymbol],startState);
		}
	}


	// parition represents start state
	partition[base-1].push_back(stateIndex[aut.transitions_->size()+aut.GetStartStates().size()]);

	relation.resize(partition.size());
	relation.reset(false);

	// 0 accepting, 1 non-accepting, 2 .. environments
	relation.set(0, 0, true);
	if (base == 3) {
		relation.set(1, 0, true);
		relation.set(1, 1, true);
	}

	relation.set(base - 1, base - 1, true); // reflexivity of start state

	res.init();

	return res;
}

#endif
