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

namespace VATA {
	template <class Rel,
		class Index = Util::IdentityTranslator<AutBase::StateType>>
	ExplicitFiniteAut CollapseStates(
			const ExplicitFiniteAut &aut,
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
		ExplicitFiniteAut res;
		aut.ReindexStates(res, rebinded);

		return res;
	}

	/******************************************************
	 * Functions prototypes
	 */

	 template <class SymbolType, class Dict>
	 ExplicitFiniteAut Complement(
			const ExplicitFiniteAut &aut,
			const Dict &alphabet);


	template <class SymbolType>
	ExplicitFiniteAut GetCandidateTree(
			const ExplicitFiniteAut& aut);
}
#endif
