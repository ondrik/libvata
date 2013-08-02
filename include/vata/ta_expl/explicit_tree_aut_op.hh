/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for operations on explicit tree automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_TREE_AUT_OP_HH_
#define _VATA_EXPLICIT_TREE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_lts.hh>
#include <vata/ta_expl/explicit_tree_aut.hh>
#include <vata/ta_expl/explicit_tree_isect.hh>
#include <vata/ta_expl/explicit_tree_useless.hh>
#include <vata/ta_expl/explicit_tree_unreach.hh>
#include <vata/ta_expl/explicit_tree_candidate.hh>
#include <vata/ta_expl/explicit_tree_comp_down.hh>
#include <vata/ta_expl/explicit_tree_transl.hh>
#include <vata/ta_expl/explicit_tree_incl_down.hh>
#include <vata/ta_expl/explicit_tree_incl_up.hh>
#include <vata/down_tree_incl_fctor.hh>
#include <vata/down_tree_opt_incl_fctor.hh>
#include <vata/tree_incl_down.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/util.hh>
#include <vata/incl_param.hh>
//#include <vata/util/convert.hh>

namespace VATA
{
	/**
	 * @brief  Unites a pair of automata
	 *
	 * Function for the union of two automata. It takes a pair of automata,
	 * renames their states and then merges them into a single automaton. States
	 * are renamed by the default dictionary or by a user defined dictionary, so
	 * they may be overlapping.
   *
   * @param[in]      lhs             Left automaton for union
   * @param[in]      rhs             Right automaton for union
   * @param[in,out]  pTranslMapLhs   Dictionary for renaming left automaton
   * @param[in,out]  pTranslMapRhs   Dictionary for renaming right automaton
	 *
	 * @returns  An automaton accepting the union of languages of @p lhs and @p
	 * rhs
   */
	ExplicitTreeAut Union(
		const ExplicitTreeAut&                lhs,
		const ExplicitTreeAut&                rhs,
		AutBase::StateToStateMap*             pTranslMapLhs = nullptr,
		AutBase::StateToStateMap*             pTranslMapRhs = nullptr)
	{
		typedef AutBase::StateType StateType;
		typedef AutBase::StateToStateTranslator StateToStateTranslator;

		AutBase::StateToStateMap translMapLhs;
		AutBase::StateToStateMap translMapRhs;

		if (!pTranslMapLhs)
		{
			pTranslMapLhs = &translMapLhs;
		}

		if (!pTranslMapRhs)
		{
			pTranslMapRhs = &translMapRhs;
		}

		StateType stateCnt = 0;
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
		StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);

		ExplicitTreeAut res(lhs.cache_);

		lhs.ReindexStates(res, stateTransLhs);
		rhs.ReindexStates(res, stateTransRhs);

		return res;
	}


	/**
	 * @brief  Unites two automata with disjoint sets of states
	 *
	 * Unites two automata. Note that these automata need to have disjoint sets of
	 * states, otherwise the result is undefined.
	 *
   * @param[in]      lhs             Left automaton for union
   * @param[in]      rhs             Right automaton for union
	 *
	 * @returns  An automaton accepting the union of languages of @p lhs and @p
	 * rhs
   */
	ExplicitTreeAut UnionDisjointStates(
		const ExplicitTreeAut&           lhs,
		const ExplicitTreeAut&           rhs)
	{
		ExplicitTreeAut res(lhs);

		assert(rhs.transitions_);

		res.uniqueClusterMap()->insert(rhs.transitions_->begin(), rhs.transitions_->end());
		assert(lhs.transitions_->size() + rhs.transitions_->size() == res.transitions_->size());

		res.finalStates_.insert(rhs.finalStates_.begin(), rhs.finalStates_.end());
		assert(lhs.finalStates_.size() + rhs.finalStates_.size() == res.finalStates_.size());

		return res;
	}


	/**
	 * @brief  Intersection of languages of a pair of automata
	 *
	 * This function creates an automaton that accepts the languages defined as
	 * the intersection of langauges of a pair of automata.
	 *
   * @param[in]   lhs             Left automaton
   * @param[in]   rhs             Right automaton
   * @param[out]  pTranslMapLhs   Dictionary for the result
	 *
	 * @returns  An automaton accepting the intersection of languages of @p lhs
	 * and @p rhs
   */
	ExplicitTreeAut Intersection(
		const ExplicitTreeAut&            lhs,
		const ExplicitTreeAut&            rhs,
		AutBase::ProductTranslMap*        pTranslMap);


	ExplicitTreeAut GetCandidateTree(
		const ExplicitTreeAut&             aut);


	ExplicitTreeAut RemoveUselessStates(
		const ExplicitTreeAut&             aut,
		AutBase::StateToStateMap*          pTranslMap);


	ExplicitTreeAut RemoveUnreachableStates(
		const ExplicitTreeAut&             aut,
		AutBase::StateToStateMap*          pTranslMap);


	template <
		class Rel,
		class Index>
	ExplicitTreeAut RemoveUnreachableStates(
		const ExplicitTreeAut&            aut,
		const Rel&                        rel,
		const Index&                      index);






	template <class Dict, class Rel>
	ExplicitTreeAut ComplementWithPreorder(
		const ExplicitTreeAut&                 aut,
		const Dict&                            alphabet,
		const Rel&                             preorder)
	{
		ExplicitTreeAut res;

		ExplicitDownwardComplementation::Compute(res, aut, alphabet, preorder);

		return RemoveUselessStates(res);
	}

	template <class Dict>
	ExplicitTreeAut Complement(
		const ExplicitTreeAut&                aut,
		const Dict&                           alphabet)
	{
		typedef AutBase::StateType StateType;
		typedef std::unordered_map<StateType, StateType> StateDict;

		StateDict stateDict;

		size_t stateCnt = 0;
		Util::TranslatorWeak<StateDict> stateTranslator(
			stateDict, [&stateCnt](const StateType&){ return stateCnt++; }
		);

		aut.BuildStateIndex(stateTranslator);

		return ComplementWithPreorder(
			aut,
			alphabet, Util::Identity(stateCnt)
			/* ComputeDownwardSimulation(
				aut, stateDict.size(), Util::TranslatorStrict<StateDict>(stateDict)
			)*/
		);

	}
}

#endif
