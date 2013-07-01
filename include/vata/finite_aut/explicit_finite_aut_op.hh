/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Header file for operations on finite automata.
 *
 *****************************************************************************/


#ifndef _VATA_EXPLICIT_FINITE_AUT_OP_HH_
#define _VATA_EXPLICIT_FINITE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/transl_weak.hh>
#include <vata/explicit_lts.hh>

#include <vata/finite_aut/explicit_finite_aut.hh>
#include <vata/finite_aut/explicit_finite_isect.hh>
#include <vata/finite_aut/explicit_finite_unreach.hh>
#include <vata/finite_aut/explicit_finite_useless.hh>
#include <vata/finite_aut/explicit_finite_reverse.hh>
#include <vata/finite_aut/explicit_finite_compl.hh>
#include <vata/finite_aut/explicit_finite_candidate.hh>
#include <vata/finite_aut/explicit_finite_transl.hh>

#include <vata/finite_aut/explicit_finite_incl.hh>

#include <vata/finite_aut/explicit_finite_incl_fctor.hh>
#include <vata/finite_aut/explicit_finite_incl_fctor_opt.hh>

#include <vata/finite_aut/explicit_finite_congr_fctor.hh>
#include <vata/finite_aut/explicit_finite_congr_fctor_opt.hh>
#include <vata/finite_aut/explicit_finite_congr_equiv_fctor.hh>
#include <vata/finite_aut/explicit_finite_congr_fctor_cache_opt.hh>

#include <vata/finite_aut/util/map_to_list.hh>
#include <vata/finite_aut/util/macrostate_cache.hh>
#include <vata/finite_aut/util/congr_product.hh>

namespace VATA {

	/*
	 * Creates union of two automata. It just reindexs
	 * existing states of both automata to a new one.
	 * Reindexing of states is not done in this function, this
	 * function just prepares translators.
	 */
	template <class SymbolType>
	ExplicitFiniteAut<SymbolType> Union(const ExplicitFiniteAut<SymbolType>& lhs,
		const ExplicitFiniteAut<SymbolType>& rhs,
		AutBase::StateToStateMap* pTranslMapLhs = nullptr,
		AutBase::StateToStateMap* pTranslMapRhs = nullptr) {


		typedef AutBase::StateType StateType;
		typedef AutBase::StateToStateTranslator StateToStateTranslator;

		/*
		 * If the maps are not given
		 * it creates own new maps
		 */
		AutBase::StateToStateMap translMapLhs;
		AutBase::StateToStateMap translMapRhs;

		if (!pTranslMapLhs) {
			pTranslMapLhs = &translMapLhs;
		}

		if (!pTranslMapRhs) {
			pTranslMapRhs = &translMapRhs;
		}

		// New translation function
		StateType stateCnt = 0;
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
		StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);

		ExplicitFiniteAut<SymbolType> res;

		lhs.ReindexStates(res, stateTransLhs);
		rhs.ReindexStates(res, stateTransRhs);


		return res;
	}

	template <class SymbolType>
	ExplicitFiniteAut<SymbolType> UnionDisjointStates(
			const ExplicitFiniteAut<SymbolType> &lhs,
			const ExplicitFiniteAut<SymbolType> &rhs)
	{
		ExplicitFiniteAut<SymbolType> res(lhs);

		// Use uniqueCluster function, not explicitly transitions_,
		// because of the possibility of the need of creating the
		// new clusterMap
		res.uniqueClusterMap()->insert(rhs.transitions_->begin(),
			rhs.transitions_->end());
		assert(lhs.transitions_->size() + rhs.transitions_->size() == res.transitions_->size());

		res.startStates_.insert(rhs.startStates_.begin(),
			rhs.startStates_.end());

		res.startStateToSymbols_.insert(rhs.startStateToSymbols_.begin(),
			rhs.startStateToSymbols_.end());

		res.finalStates_.insert(rhs.finalStates_.begin(),
			rhs.finalStates_.end());
		return res;
	}

	template <class SymbolType,
		class Rel,
		class Index = Util::IdentityTranslator<AutBase::StateType>>
	ExplicitFiniteAut<SymbolType> CollapseStates(
			const ExplicitFiniteAut<SymbolType> &aut,
			const Rel & rel,
			const Index &index = Index()) {

		std::vector<size_t> representatives;

		// Creates vector, which contains equivalences classes of states of
		// aut automaton
		// If relation is identity, newly created automaton will be same
		rel.buildClasses(representatives);

		std::vector<AutBase::StateType> rebinded(representatives.size());

		// Transl will contain new numbers (indexes) for input states
		Util::RebindMap2(rebinded, representatives, index);
		ExplicitFiniteAut<SymbolType> res;
		aut.ReindexStates(res, rebinded);

		return res;
	}

	/******************************************************
	 * Functions prototypes
	 */
 template <class SymbolType>
 ExplicitFiniteAut<SymbolType> Intersection(
			const ExplicitFiniteAut<SymbolType> &lhs,
			const ExplicitFiniteAut<SymbolType> &rhs,
			AutBase::ProductTranslMap* pTranslMap = nullptr);

	template <class SymbolType>
	ExplicitFiniteAut<SymbolType> RemoveUnreachableStates(
			const ExplicitFiniteAut<SymbolType> &aut,
			VATA::AutBase::StateToStateMap* pTranslMap = nullptr);


	template <class SymbolType>
	ExplicitFiniteAut<SymbolType> RemoveUselessStates(
			const ExplicitFiniteAut<SymbolType> &aut,
			VATA::AutBase::StateToStateMap* pTranslMap = nullptr);

	template <class SymbolType>
	ExplicitFiniteAut<SymbolType> Reverse(
		const ExplicitFiniteAut<SymbolType> &aut,
		AutBase::ProductTranslMap* pTranslMap = nullptr);


	 template <class SymbolType, class Dict>
	 ExplicitFiniteAut<SymbolType> Complement(
			const ExplicitFiniteAut<SymbolType> &aut,
			const Dict &alphabet);


	template <class SymbolType>
	ExplicitFiniteAut<SymbolType> GetCandidateTree(
			const ExplicitFiniteAut<SymbolType>& aut);


	// Translator for simulation
	template <class SymbolType, class Index>
	ExplicitLTS Translate(const ExplicitFiniteAut<SymbolType>& aut,
		std::vector<std::vector<size_t>>& partition,
		Util::BinaryRelation& relation,
		const Index& stateIndex);


	/***************************************************
	 * Simulation functions
	 */
	template <class SymbolType, class Index>
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut, const size_t& size, const Index& index) {

		AutBase::StateBinaryRelation relation;
		std::vector<std::vector<size_t>> partition(1);
		return Translate(aut, partition, relation, index).computeSimulation(size);

	}

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut, const size_t& size) {

		AutBase::StateBinaryRelation relation;
		std::vector<std::vector<size_t>> partition(1);
		return Translate(aut, partition, relation).computeSimulation(partition,relation,size);
	}

	// Automaton has not been sanitized
	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeDownwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut) {

		return ComputeDownwardSimulation(aut, AutBase::SanitizeAutForSimulation(aut));
	}

	/*****************************************************************
	 * Upward simulation just for compability
	 */
	template <class SymbolType, class Index>
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut, const size_t& size, const Index& index) {

		return ComputeDownwardSimulation(aut, size);
	}

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut, const size_t& size) {

		return ComputeDownwardSimulation(aut, size);
	}

	template <class SymbolType>
	AutBase::StateBinaryRelation ComputeUpwardSimulation(
		const ExplicitFiniteAut<SymbolType>& aut) {

		return ComputeDownwardSimulation(aut, AutBase::SanitizeAutForSimulation(aut));

	}

	/*
	 * Checks language equivalence.
	 * Uses the inclusion wrapper function, because it works
	 * on similiar principle, but special functor for equivalence checking
	 * is given
	 */
	template <class SymbolType, class Rel>
	bool CheckEquivalenceBreadth(
		const ExplicitFiniteAut<SymbolType>& smaller,
		const ExplicitFiniteAut<SymbolType>& bigger,
		const Rel& preorder) {
		/*
		typedef typename ExplicitFiniteAut<SymbolType>::StateSet StateSet;
		typedef typename std::pair<StateSet*,StateSet*> ProductState;
		typedef ProductStateSetBreadth<StateSet,ProductState> ProductSet;
		*/
		typedef typename ExplicitFiniteAut<SymbolType>::StateSet StateSet;
		typedef typename std::pair<StateSet*,StateSet*> ProductState;
		typedef ProductStateSetBreadth<StateSet,ProductState> ProductSet;
		typedef ExplicitFACongrEquivFunctor<SymbolType,Rel,ProductSet> FunctorType;
		return CheckFiniteAutInclusion<SymbolType,Rel,FunctorType>(smaller, bigger, preorder);
	}

	template <class SymbolType, class Rel>
	bool CheckEquivalenceDepth(
		const ExplicitFiniteAut<SymbolType>& smaller,
		const ExplicitFiniteAut<SymbolType>& bigger,
		const Rel& preorder) {
		/*
		typedef typename ExplicitFiniteAut<SymbolType>::StateSet StateSet;
		typedef typename std::pair<StateSet*,StateSet*> ProductState;
		typedef ProductStateSetBreadth<StateSet,ProductState> ProductSet;
		*/
		typedef typename ExplicitFiniteAut<SymbolType>::StateSet StateSet;
		typedef typename std::pair<StateSet*,StateSet*> ProductState;
		typedef ProductStateSetDepth<StateSet,ProductState> ProductSet;
		typedef ExplicitFACongrEquivFunctor<SymbolType,Rel,ProductSet> FunctorType;
		return CheckFiniteAutInclusion<SymbolType,Rel,FunctorType>(smaller, bigger, preorder);
	}
}
#endif
