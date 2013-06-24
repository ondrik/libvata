/*****************************************************************************
 *  VATA Finite Automata Library
 *
 *  Copyright (c) 2013  Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *  Description:
 *  Functor for checking inclusion using antichain algorithm for explicitly 
 *  represented finite.
 *
 *****************************************************************************/


#ifndef EXPLICIT_FINITE_AUT_INCL_FCTOR_HH_
#define EXPLICIT_FINITE_AUT_INCL_FCTOR_HH_

#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include <vata/util/antichain1c.hh>
#include "explicit_finite_aut.hh"

#include "explicit_finite_abstract_fctor.hh"

// standard libraries
#include <vector>

namespace VATA {
  template <class SymbolType, class Rel> class ExplicitFAInclusionFunctor;
  template <class SymbolType,class Rel> class ExplicitFAStateSetComparator;
}

GCC_DIAG_OFF(effc++) // non virtual destructors warnings supress
template<class SymbolType, class Rel>
class VATA::ExplicitFAStateSetComparator {
GCC_DIAG_ON(effc++) 

public:
  typedef ExplicitFiniteAut<SymbolType> ExplicitFA;
  typedef typename ExplicitFA::StateSet StateSet;

private: // private data members
  Rel preorder_;

public:
  ExplicitFAStateSetComparator(Rel preorder) : preorder_(preorder) {}
public: // public methods
  // lss is subset of rss
  inline bool lte(const StateSet& lss, const StateSet& rss) {
      bool res = true;

      // refraktoring, ktery udela special tridu pro preorder
      //if (lss.size() > rss.size()) {
      //  return false;
      //}
      for (auto ls : lss) {
        bool tempres = false;
        for (auto rs : rss) {
          if (preorder_.get(ls,rs)) {
            tempres |= true;
            break;
          }
        }
        res &= tempres;
        if (!res) {
          return false;
        }
      }
      //return true;
      return res;
  }

  // rss is subset of lss
  inline bool gte(const StateSet& lss, const StateSet& rss) {
    return lte(rss,lss);
  }
};

template <class SymbolType, class Rel>
class VATA::ExplicitFAInclusionFunctor : 
  public ExplicitFAAbstractFunctor <SymbolType,Rel> {

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

  typedef ExplicitFAStateSetComparator<SymbolType,Rel> Comparator;

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
  ExplicitFAInclusionFunctor(AntichainType& antichain, AntichainType& next,
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

    // Get candidates for given state
    std::vector<StateType> candidates;
    for (StateType candidate : singleAntichain_.data()) {
      if (preorder_.get(candidate,state)) {
        candidates.push_back(candidate);
      }
    }

    // Check whether the antichain does not already 
    // contains given product state
    std::vector<StateType> tempStateSet = {state};
    if (!antichain_.contains(tempStateSet,set,lte)) {
      antichain_.refine(tempStateSet,set,gte);
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
    };    
    std::vector<StateType> tempStateSet = {state};
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
