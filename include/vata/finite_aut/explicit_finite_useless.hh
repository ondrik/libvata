/*****************************************************************************
 *  VATA Finite Automata Library
 *
 *  Copyright (c) 2013  Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *  Removing useless states for explicitly represented finite automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_AUT_USELESS_HH_
#define _VATA_EXPLICIT_FINITE_AUT_USELESS_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>
#include <vata/finite_aut/explicit_finite_reverse.hh>
#include <vata/finite_aut/explicit_finite_unreach.hh>

namespace VATA {
  template <class SymbolType>
  ExplicitFiniteAut<SymbolType> RemoveUselessStates(
      const ExplicitFiniteAut<SymbolType> &aut,
      VATA::AutBase::StateToStateMap* pTranslMap = nullptr);
}

template <class SymbolType>
VATA::ExplicitFiniteAut<SymbolType> VATA::RemoveUselessStates(
    const VATA::ExplicitFiniteAut<SymbolType> &aut,
    VATA::AutBase::StateToStateMap* pTranslMap = nullptr) {
 
  // remove useless states by applying following operations
  return Reverse(
     RemoveUnreachableStates(
      Reverse(
        RemoveUnreachableStates(aut,pTranslMap)
      ),pTranslMap
     )
    );
}

#endif
