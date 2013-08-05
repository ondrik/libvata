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
#include <vata/ta_expl/explicit_tree_comp_down.hh>
#include <vata/ta_expl/explicit_tree_transl.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/util.hh>
#include <vata/incl_param.hh>
//#include <vata/util/convert.hh>

namespace VATA
{



	template <class Dict, class Rel>
	ExplicitTreeAut ComplementWithPreorder(
		const ExplicitTreeAut&                 aut,
		const Dict&                            alphabet,
		const Rel&                             preorder)
	{
		ExplicitTreeAut res;

		ExplicitDownwardComplementation::Compute(res, aut, alphabet, preorder);

		return res.RemoveUselessStates();
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
