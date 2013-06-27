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

// VATA headers
#include <vata/vata.hh>
#include <vata/util/antichain2c_v2.hh>

#include <vata/finite_aut/explicit_finite_aut.hh>
#include <vata/finite_aut/explicit_finite_congr_fctor_cache.hh>
#include <vata/finite_aut/explicit_finite_congr_fctor_cache_opt.hh>
#include <vata/finite_aut/explicit_finite_incl_fctor.hh>
#include <vata/finite_aut/explicit_finite_incl_fctor_cache.hh>

#include <vata/finite_aut/util/comparators.hh>

namespace VATA
{
	template<class SymbolType, class Rel, class Functor>
	bool CheckFiniteAutInclusion(
		const ExplicitFiniteAut<SymbolType>& smaller,
		const ExplicitFiniteAut<SymbolType>& bigger,
		const Rel& preorder);


	template<class SymbolType, class Rel, class Functor>
	bool CheckFiniteAutInclusion (
		const ExplicitFiniteAut<SymbolType>& smaller,
		const ExplicitFiniteAut<SymbolType>& bigger,
		const Rel& preorder);

	template <class SymbolType, class Rel>
	bool CheckUpwardInclusionWithPreorder(
		const ExplicitFiniteAut<SymbolType>& smaller,
		const ExplicitFiniteAut<SymbolType>& bigger,
		const Rel& preorder) {

		typedef ExplicitFAStateSetComparatorIdentity<SymbolType,Rel> Comparator;
		// There is possible to set macro to use one of the optimization
#ifdef OPT_AC
		typedef ExplicitFAInclusionFunctorOpt<SymbolType,Rel,Comparator> FunctorType;
#elif CACHE_AC
		typedef ExplicitFAInclusionFunctorCache<SymbolType,Rel,Comparator> FunctorType;
#else
		typedef ExplicitFAInclusionFunctor<SymbolType,Rel> FunctorType;
#endif
		return CheckFiniteAutInclusion<SymbolType,Rel,FunctorType>(smaller, bigger, preorder);
	}

	// Special for simulation
	template <class SymbolType, class Rel>
	bool CheckUpwardInclusionWithSim(
		const ExplicitFiniteAut<SymbolType>& smaller,
		const ExplicitFiniteAut<SymbolType>& bigger,
		const Rel& preorder) {

		typedef ExplicitFAStateSetComparatorSimulation<SymbolType,Rel> Comparator;
#ifdef OPT_AC
		typedef ExplicitFAInclusionFunctorOpt<SymbolType,Rel,Comparator> FunctorType;
#elif CACHE_AC
		typedef ExplicitFAInclusionFunctorCache<SymbolType,Rel,Comparator> FunctorType;
#else
		typedef ExplicitFAInclusionFunctor<SymbolType,Rel> FunctorType;
#endif
		return CheckFiniteAutInclusion<SymbolType,Rel,FunctorType>(smaller, bigger, preorder);
	}

	/*
	 * Congruence function
	 * preorder not used yet, but one time will be
	 */
	template <class SymbolType, class Rel>
	bool CheckInclusionWithCongr(
		const ExplicitFiniteAut<SymbolType>& smaller,
		const ExplicitFiniteAut<SymbolType>& bigger,
		const Rel& preorder) {
#ifdef CACHE_OPT_CONGR
		typedef ExplicitFACongrFunctorCacheOpt<SymbolType,Rel> FunctorType;
#elif OPT_CONGR
		typedef ExplicitFACongrFunctorOpt<SymbolType,Rel> FunctorType;
#elif CACHE_CONGR
		typedef ExplicitFACongrFunctorCache<SymbolType,Rel> FunctorType;
#else
		typedef ExplicitFACongrFunctor<SymbolType,Rel> FunctorType;
#endif
		return CheckFiniteAutInclusion<SymbolType,Rel,FunctorType>(smaller, bigger, preorder);
	}

	/*
	 * Get just two automata, first sanitization is
	 * made then the inclusion check is called
	 */
	template <class SymbolType>
	bool CheckInclusion(
		const ExplicitFiniteAut<SymbolType>&    smaller,
		const ExplicitFiniteAut<SymbolType>&    bigger,
		const VATA::InclParam*                  params = nullptr)
	{
		if (nullptr == params)
		{ }

		if ((nullptr == &smaller) || (nullptr == &bigger))
		{ }

		// TODO: implement these things
		assert(false);

//		AutBase::StateType states =
//			AutBase::SanitizeAutsForInclusion(smaller, bigger);
//		VATA::Util::Identity ident(states);

//		return CheckUpwardInclusion(smaller, bigger,states);
//		TODO: this was... like... bullshit, right?
//
		throw std::runtime_error("Unimplemented");
	}
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
