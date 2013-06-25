/*****************************************************************************
 *  VATA Finite Automata Library
 *
 *  Copyright (c) 2013  Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *  Functor for checking inclusion using antichain algorithm for explicitly 
 *  represented finite automata and uses some optimization.
 *
 *****************************************************************************/


#ifndef EXPLICIT_FINITE_AUT_INCL_FCTOR_OPT_HH_
#define EXPLICIT_FINITE_AUT_INCL_FCTOR_OPT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include <vata/util/antichain1c.hh>
#include <vata/finite_aut/explicit_finite_aut.hh>

#include <vata/finite_aut/explicit_finite_abstract_fctor.hh>

// standard libraries
#include <vector>

namespace VATA {
  template <class SymbolType, class Rel, class Comparator> class ExplicitFAInclusionFunctorOpt;
}

GCC_DIAG_OFF(effc++)
template <class SymbolType, class Rel, class Comparator>
class VATA::ExplicitFAInclusionFunctorOpt : 
  public ExplicitFAAbstractFunctor <SymbolType,Rel> {
GCC_DIAG_ON(effc++)

public : // data types
  typedef ExplicitFAAbstractFunctor<SymbolType,Rel> AbstractFunctor;
  typedef typename AbstractFunctor::ExplicitFA ExplicitFA;

  typedef typename AbstractFunctor::StateType StateType;
  typedef typename AbstractFunctor::StateSet StateSet;

  typedef typename AbstractFunctor::Antichain1Type Antichain1Type;

  typedef StateType SmallerElementType;
  typedef StateSet BiggerElementType;

  typedef VATA::Util::Antichain2Cv2<SmallerElementType,BiggerElementType> 
    AntichainType;

  typedef AntichainType ProductStateSetType;
  typedef ProductStateSetType ProductNextType;

  typedef typename AbstractFunctor::IndexType IndexType;

private: // data memebers
  AntichainType& antichain_;
  AntichainType& next_;
  Antichain1Type& singleAntichain_;

  const ExplicitFA& smaller_;
  const ExplicitFA& bigger_;

  IndexType& index_;
  IndexType& inv_;

  Rel preorder_; // Simulation or identity

  Comparator comparator_;

public: // constructor
  ExplicitFAInclusionFunctorOpt(AntichainType& antichain, AntichainType& next,
      Antichain1Type& singleAntichain,
      const ExplicitFA& smaller, 
      const ExplicitFA& bigger,
      IndexType& index,
      IndexType& inv,
      Rel preorder) :
    antichain_(antichain),
    next_(next),
    singleAntichain_(singleAntichain),
    smaller_(smaller),
    bigger_(bigger),
    index_(index),
    inv_(inv),
    preorder_(preorder),
    comparator_(preorder)
  {}

public: // public functions

  // Initialize the antichain from init states of given automata
  void Init() {
    bool macroFinal=false;
    StateSet procMacroState; 

    // Create macro state of initial states
    for (StateType startState : bigger_.startStates_) {
      procMacroState.insert(startState);
      macroFinal |= bigger_.IsStateFinal(startState);
    }

    // Check the initial states
    for (StateType smallState : smaller_.startStates_) {
      this->inclNotHold_ |= smaller_.IsStateFinal(smallState) && !macroFinal;
      this->AddNewPairToAntichain(smallState,procMacroState);
    }
  }

  /*
   * Make post set of the product states
   * for given product state (r,R) and
   * add states to the antichain
   */
  void MakePost(StateType procState, StateSet& procMacroState) {
    auto iteratorSmallerSymbolToState = smaller_.transitions_->find(procState);
    if (iteratorSmallerSymbolToState == smaller_.transitions_->end()) {
      return;
    }
    // Iterate through the all symbols in the transitions for the given state
    for (auto& smallerSymbolToState : *(iteratorSmallerSymbolToState->second)) {
      for (const StateType& newSmallerState : smallerSymbolToState.second) {
        StateSet newMacroState;

        // Create post macrostate for given symbol
        bool IsMacroAccepting = this->CreatePostOfMacroState(
            newMacroState,procMacroState,smallerSymbolToState.first,
            bigger_);

        this->inclNotHold_ |= smaller_.IsStateFinal(newSmallerState) && 
          !IsMacroAccepting; 

        if (this->inclNotHold_) {
          return;
        }

        this->AddNewPairToAntichain(newSmallerState,newMacroState);
      }
    }
    procMacroState.clear();
  }

private: // private functions
  /*
   * Add a new product state to the antichains sets
   */
  void AddNewPairToAntichain(StateType state, StateSet &set) {
    //lss is subset of rss -> return TRUE
    auto lte = [this](const StateSet& lss, const StateSet& rss) {
      return this->comparator_.lte(lss,rss);
    };

    auto gte = [this](const StateSet& lss, const StateSet& rss) {
      return this->comparator_.gte(lss,rss);
    };

    

    // Check whether the antichain does not already 
    // contains given product state
    std::vector<StateType> candidates;
    // Get candidates for given state
    comparator_.getCandidate(candidates,state,singleAntichain_);
    if (!antichain_.contains(candidates,set,lte)) {
      antichain_.refine(candidates,set,gte);
      antichain_.insert(state,set);
      AddToSingleAC(state);
      AddToNext(state,set);
    }
  }

  /*
   * Add state to the small antichain
   */
  void AddToSingleAC(StateType state) {
    std::vector<StateType> tempStateSet = {state};
    if (!singleAntichain_.contains(tempStateSet)) {
      singleAntichain_.insert(state);
    }
  }

  /*
   * Add product state to the next set
   */
  void AddToNext(StateType state, StateSet& set) {
    //lss is subset of rss -> return TRUE
    auto lte = [this](const StateSet& lss, const StateSet& rss) {
      return this->comparator_.lte(lss,rss);
    };

    auto gte = [this](const StateSet& lss, const StateSet& rss) {
      return this->comparator_.gte(lss,rss);
    };    std::vector<StateType> tempStateSet = {state};
    if (!next_.contains(tempStateSet,set,lte)) {
      next_.refine(tempStateSet,set,gte);
      next_.insert(state,set);
    }
  }


private: // Private inline functions
  /*
   * Copy one set to another 
   */
  inline void CopySet(StateSet& fullset, StateSet& emptyset) {
    emptyset.insert(fullset.begin(),fullset.end());
  }

};

#endif
