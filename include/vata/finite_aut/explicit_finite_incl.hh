/*****************************************************************************
 *  VATA Finite Automata Library
 *
 *  Copyright (c) 2013  Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *  Checking language inclusion for explicitly represented finite automata.
 *
 *****************************************************************************/


#ifndef EXPLICIT_FINITE_AUT_INCL_HH_
#define EXPLICIT_FINITE_AUT_INCL_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>
#include <vata/finite_aut/explicit_finite_incl_fctor.hh>

namespace VATA {

  template<class SymbolType, class Rel, class Functor>
  bool CheckFiniteAutInclusion(
    const ExplicitFiniteAut<SymbolType>& smaller, 
    const ExplicitFiniteAut<SymbolType>& bigger, 
    const Rel& preorder);
}

/*
 * Function wrapping inclusion checking
 */
template<class SymbolType, class Rel, class Functor>
bool VATA::CheckFiniteAutInclusion(
  const VATA::ExplicitFiniteAut<SymbolType>& smaller, 
  const VATA::ExplicitFiniteAut<SymbolType>& bigger, 
  const Rel& preorder) {
 
  typedef Functor InclFunc;

  typedef typename InclFunc::SmallerElementType SmallerElementType;
  typedef typename InclFunc::BiggerElementType BiggerElementType;

  typedef typename InclFunc::ProductStateSetType ProductStateSetType;
  typedef typename InclFunc::ProductNextType ProductNextType;
  typedef typename InclFunc::Antichain1Type Antichain1Type; 

  typedef typename InclFunc::IndexType IndexType;

  ProductStateSetType antichain;
  ProductNextType next;
  Antichain1Type singleAntichain;

  IndexType index;
  IndexType inv;

  preorder.buildIndex(index,inv);

  InclFunc inclFunc(antichain,next,singleAntichain,
      smaller,bigger,index,inv,preorder);

  // Initialization of antichain sets from initial states of automata
  inclFunc.Init();

  if (!inclFunc.DoesInclusionHold()) {
    return false;
  }

  // actually processed macro state
  BiggerElementType procMacroState; 
  SmallerElementType procState;

  while(inclFunc.DoesInclusionHold() && next.get(procState,procMacroState)) {
    inclFunc.MakePost(procState,procMacroState);
  }
  return inclFunc.DoesInclusionHold();
}
#endif
