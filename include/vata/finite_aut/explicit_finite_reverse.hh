/*****************************************************************************
 *  VATA Finite Automata Library
 *
 *  Copyright (c) 2013  Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *  Reversion for explicitly represented finite automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_AUT_REVERSE_HH_
#define _VATA_EXPLICIT_FINITE_AUT_REVERSE_HH_

// VATA headers
#include <vata/vata.hh>
#include "explicit_finite_aut.hh"

namespace VATA {
template <class SymbolType>
  ExplicitFiniteAut<SymbolType> Reverse(
    const ExplicitFiniteAut<SymbolType> &aut,
    AutBase::ProductTranslMap* pTranslMap = nullptr); 
}

template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::Reverse(
  const VATA::ExplicitFiniteAut<SymbolType> &aut,
  AutBase::ProductTranslMap* pTranslMap = nullptr) {

  typedef VATA::ExplicitFiniteAut<SymbolType> ExplicitFA;

  auto transitions_ = aut.transitions_;

  ExplicitFA res;

  res.finalStates_ = aut.startStates_; // set final states
  res.startStates_ = aut.finalStates_; // set start states
  res.startStateToSymbols_ = aut.startStateToSymbols_; // start symbols

  // Changing the order of left and right in each transition
  for (auto stateToCluster : *transitions_) {
    for (auto symbolToSet : *stateToCluster.second) {
      for (auto stateInSet : symbolToSet.second) {
        res.AddTransition(stateInSet,symbolToSet.first,
          stateToCluster.first);
      }
    }
  }

  return res;
}
#endif
