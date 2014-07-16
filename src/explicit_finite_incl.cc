/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Checking language inclusion for explicitly represented finite automata.
 *
 *****************************************************************************/

#include <iostream>

// VATA headers
#include <vata/vata.hh>

#include <vata/explicit_finite_aut.hh>

#include "explicit_finite_congr_fctor_cache_opt.hh"
#include "explicit_finite_congr_equiv_fctor.hh"
#include "explicit_finite_incl_fctor_cache.hh"

#include <vata/finite_aut/util/comparators.hh>
#include <vata/finite_aut/util/normal_form_rel.hh>
#include <vata/finite_aut/util/map_to_list.hh>
#include <vata/finite_aut/util/macrostate_cache.hh>
#include <vata/finite_aut/util/congr_product.hh>
#include <vata/util/antichain2c_v2.hh>

namespace VATA
{
	template<class Rel, class Functor>
	bool CheckFiniteAutInclusion(
		const ExplicitFiniteAutCore& smaller,
		const ExplicitFiniteAutCore& bigger,
		const Rel& preorder);

	bool CheckEquivalence(
		const ExplicitFiniteAutCore& smaller,
		const ExplicitFiniteAutCore& bigger,
		const InclParam&                  params);

}

/*
 * Get just two automata, first sanitization is
 * made then the inclusion check is called
 */
bool VATA::ExplicitFiniteAutCore::CheckInclusion(
	const VATA::ExplicitFiniteAutCore&    smaller,
	const VATA::ExplicitFiniteAutCore&    bigger,
	const VATA::InclParam&												params)
{
	VATA::ExplicitFiniteAutCore newSmaller;
	VATA::ExplicitFiniteAutCore newBigger;
	typename AutBase::StateType states = static_cast<typename AutBase::StateType>(-1);

	if (!params.GetUseSimulation())
	{
		newSmaller = smaller;
		newBigger = bigger;

		states = VATA::AutBase::SanitizeAutsForInclusion(newSmaller, newBigger);
	}

	// if a simulation is used, a union has been already done before the simulation
	if (params.GetAlgorithm() == InclParam::e_algorithm::congruences && !params.GetUseSimulation())
	{
		newSmaller = UnionDisjointStates(smaller, bigger);
	}

	switch (params.GetOptions())
	{
		case InclParam::ANTICHAINS_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			typedef VATA::Util::Identity Rel;
			typedef VATA::ExplicitFAStateSetComparatorIdentity<Rel> Comparator;
			typedef VATA::ExplicitFAInclusionFunctorCache<Rel,Comparator> FunctorType;

			return VATA::CheckFiniteAutInclusion<Rel,FunctorType>(newSmaller,
					newBigger, VATA::Util::Identity(states));
		}
		case InclParam::ANTICHAINS_SIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) == states);

			typedef VATA::AutBase::StateBinaryRelation Rel;
			typedef VATA::ExplicitFAStateSetComparatorSimulation<Rel> Comparator;
			typedef VATA::ExplicitFAInclusionFunctorCache<Rel,Comparator> FunctorType;

			return VATA::CheckFiniteAutInclusion<Rel,FunctorType>(smaller, bigger, params.GetSimulation());
		}
		case InclParam::CONGR_BREADTH_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			typedef VATA::Util::Identity Rel;
			typedef typename VATA::ExplicitFiniteAutCore::StateSet StateSet;
			typedef typename std::pair<StateSet*,StateSet*> ProductState;
			typedef VATA::ProductStateSetBreadth<StateSet,ProductState> ProductSet;
			typedef VATA::NormalFormRelPreorder<Rel> NormalFormRel;
			typedef VATA::ExplicitFACongrFunctorCacheOpt<Rel,ProductSet,NormalFormRel> FunctorType;

			return VATA::CheckFiniteAutInclusion<Rel,FunctorType>(newSmaller, newBigger, VATA::Util::Identity(states));
		}
		case InclParam::CONGR_DEPTH_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			typedef VATA::Util::Identity Rel;
			typedef typename VATA::ExplicitFiniteAutCore::StateSet StateSet;
			typedef typename std::pair<StateSet*,StateSet*> ProductState;
			typedef VATA::ProductStateSetDepth<StateSet,ProductState> ProductSet;
			typedef VATA::NormalFormRelPreorder<Rel> NormalFormRel;
			typedef VATA::ExplicitFACongrFunctorCacheOpt<Rel,ProductSet,NormalFormRel> FunctorType;

			return VATA::CheckFiniteAutInclusion<Rel,FunctorType>(newSmaller, newBigger, VATA::Util::Identity(states));
		}
		case InclParam::CONGR_DEPTH_SIM:
		{
			typedef VATA::AutBase::StateBinaryRelation Rel;
			typedef typename VATA::ExplicitFiniteAutCore::StateSet StateSet;
			typedef typename std::pair<StateSet*,StateSet*> ProductState;
			typedef VATA::ProductStateSetDepth<StateSet,ProductState> ProductSet;
			typedef VATA::NormalFormRelSimulation<Rel> NormalFormRel;

			typedef VATA::ExplicitFACongrFunctorCacheOpt<Rel,ProductSet,NormalFormRel> FunctorType;

			return VATA::CheckFiniteAutInclusion<Rel,FunctorType>(smaller, bigger, params.GetSimulation());
		}
		case InclParam::CONGR_DEPTH_EQUIV_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			typedef VATA::Util::Identity Rel;
			typedef typename VATA::ExplicitFiniteAutCore::StateSet StateSet;
			typedef typename std::pair<StateSet*,StateSet*> ProductState;
			typedef VATA::ProductStateSetDepth<StateSet,ProductState> ProductSet;
			typedef VATA::ExplicitFACongrEquivFunctor<Rel,ProductSet> FunctorType;

			return VATA::CheckFiniteAutInclusion<Rel,FunctorType>(newSmaller, newBigger, VATA::Util::Identity(states));
		}
		case InclParam::CONGR_BREADTH_EQUIV_NOSIM:
		{
			assert(static_cast<typename AutBase::StateType>(-1) != states);

			typedef VATA::Util::Identity Rel;
			typedef typename VATA::ExplicitFiniteAutCore::StateSet StateSet;
			typedef typename std::pair<StateSet*,StateSet*> ProductState;
			typedef VATA::ProductStateSetBreadth<StateSet,ProductState> ProductSet;
			typedef VATA::ExplicitFACongrEquivFunctor<Rel,ProductSet> FunctorType;

			return VATA::CheckFiniteAutInclusion<Rel,FunctorType>(newSmaller, newBigger, VATA::Util::Identity(states));
		}
		default:
		{
			throw NotImplementedException("Unimplemented inclusion:\n" +
				params.toString());
		}
	}
}

/*
 * Function wrapping inclusion checking
 */
template<class Rel, class Functor>
bool VATA::CheckFiniteAutInclusion(
	const VATA::ExplicitFiniteAutCore& smaller,
	const VATA::ExplicitFiniteAutCore& bigger,
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
