/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Checking language inclusion for explicitly represented finite automata.
 *
 *****************************************************************************/


#ifndef EXPLICIT_FINITE_AUT_INCL_HH_
#define EXPLICIT_FINITE_AUT_INCL_HH_

#include <iostream>

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>
#include <vata/incl_param.hh>

#include <vata/finite_aut/explicit_finite_aut.hh>
#include <vata/finite_aut/explicit_finite_congr_fctor_cache_opt.hh>
#include <vata/finite_aut/explicit_finite_incl_fctor_cache.hh>

#include <vata/finite_aut/util/comparators.hh>
#include <vata/finite_aut/util/map_to_list.hh>
#include <vata/finite_aut/util/macrostate_cache.hh>
#include <vata/finite_aut/util/congr_product.hh>

namespace VATA
{
	template<class SymbolType, class Rel, class Functor>
	bool CheckFiniteAutInclusion(
		const ExplicitFiniteAut<SymbolType>& smaller,
		const ExplicitFiniteAut<SymbolType>& bigger,
		const Rel& preorder);

	template <class SymbolType>
	bool CheckInclusion(
		const ExplicitFiniteAut<SymbolType>&    smaller,
		const ExplicitFiniteAut<SymbolType>&    bigger,
		const InclParam&                  params);

}

/*
 * Get just two automata, first sanitization is
 * made then the inclusion check is called
 */
template <class SymbolType>
bool VATA::CheckInclusion(
	const VATA::ExplicitFiniteAut<SymbolType>&    smaller,
	const VATA::ExplicitFiniteAut<SymbolType>&    bigger,
	const VATA::InclParam&                  params)
{
	if ((nullptr == &smaller) || (nullptr == &bigger))
	{ 
		throw std::runtime_error("Smaller or bigger automata is not defined.");
	}

	VATA::ExplicitFiniteAut<SymbolType> newSmaller;
	VATA::ExplicitFiniteAut<SymbolType> newBigger;
	typename VATA::AutBase::StateType states;

	if (!params.GetUseSimulation())
	{
		newSmaller = smaller;
		newBigger = bigger;

		states = VATA::AutBase::SanitizeAutsForInclusion(newSmaller, newBigger);
	}

	switch (params.GetOptions())
	{
		case InclParam::ANTICHAINS_NOSIM:
		{
			typedef VATA::Util::Identity Rel;
			typedef VATA::ExplicitFAStateSetComparatorIdentity<SymbolType,Rel> Comparator;
			typedef VATA::ExplicitFAInclusionFunctorCache<SymbolType,Rel,Comparator> FunctorType;

			return VATA::CheckFiniteAutInclusion<SymbolType,Rel,FunctorType>(newSmaller, 
					newBigger, VATA::Util::Identity(states));
		}
		case InclParam::ANTICHAINS_SIM:
		{
			typedef VATA::AutBase::StateBinaryRelation Rel;
			typedef VATA::ExplicitFAStateSetComparatorSimulation<SymbolType,Rel> Comparator;
			typedef VATA::ExplicitFAInclusionFunctorCache<SymbolType,Rel,Comparator> FunctorType;

			return VATA::CheckFiniteAutInclusion<SymbolType,Rel,FunctorType>(newSmaller, newBigger, params.GetSimulation());
		}
		case InclParam::CONGR_BREADTH_NOSIM:
		{
			typedef VATA::Util::Identity Rel;
			typedef typename VATA::ExplicitFiniteAut<SymbolType>::StateSet StateSet;
			typedef typename std::pair<StateSet*,StateSet*> ProductState;
			typedef VATA::ProductStateSetBreadth<StateSet,ProductState> ProductSet;
			typedef VATA::ExplicitFACongrFunctorCacheOpt<SymbolType,Rel,ProductSet> FunctorType;
			
			return VATA::CheckFiniteAutInclusion<SymbolType,Rel,FunctorType>(newSmaller, newBigger, VATA::Util::Identity(states));
		}
		case InclParam::CONGR_DEPTH_NOSIM:
		{
			typedef VATA::Util::Identity Rel;
			typedef typename VATA::ExplicitFiniteAut<SymbolType>::StateSet StateSet;
			typedef typename std::pair<StateSet*,StateSet*> ProductState;
			typedef VATA::ProductStateSetDepth<StateSet,ProductState> ProductSet;
			typedef VATA::ExplicitFACongrFunctorCacheOpt<SymbolType,Rel,ProductSet> FunctorType;
			
			return VATA::CheckFiniteAutInclusion<SymbolType,Rel,FunctorType>(newSmaller, newBigger, VATA::Util::Identity(states));
		}
		default:
		{
			throw std::runtime_error("Unimplemented inclusion:\n" +
				params.toString());
		}
	}

//		AutBase::StateType states =
//			AutBase::SanitizeAutsForInclusion(smaller, bigger);
//		VATA::Util::Identity ident(states);

//		return CheckUpwardInclusion(smaller, bigger,states);
//		TODO: this was... like... bullshit, right?
//
		throw std::runtime_error("Unimplemented");
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
