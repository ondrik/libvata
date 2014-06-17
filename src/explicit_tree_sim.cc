/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Jiri Simacek <isimacek@fit.vutbr.cz>
 *
 *  Description:
 *    Implemention of simulation computation for explicitly represented tree
 *    automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/aut_base.hh>

#include "explicit_tree_aut_core.hh"
#include "explicit_tree_transl.hh"

using VATA::AutBase;
using VATA::ExplicitTreeAutCore;

using StateBinaryRelation  = AutBase::StateBinaryRelation;
using StateDiscontBinaryRelation  = AutBase::StateDiscontBinaryRelation;


StateDiscontBinaryRelation ExplicitTreeAutCore::ComputeSimulation(
	const VATA::SimParam&                  /* params */) const
{
	assert(false);
	// switch (params.GetRelation())
	// {
	// 	case SimParam::e_sim_relation::TA_UPWARD:
	// 	{
	// 		return this->ComputeUpwardSimulation(params, transl);
	// 	}
	// 	case SimParam::e_sim_relation::TA_DOWNWARD:
	// 	{
	// 		return this->ComputeDownwardSimulation(params, transl);
	// 	}
	// 	default:
	// 	{
	// 		throw std::runtime_error("Unknown simulation parameters: " + params.toString());
	// 	}
	// }
}


StateBinaryRelation ExplicitTreeAutCore::ComputeUpwardSimulation(
	const SimParam&                        params,
	StateToStateTranslStrict&              transl) const
{
	if (params.GetNumStates() != static_cast<size_t>(-1))
	{
		return this->ComputeUpwardSimulation(params.GetNumStates(), transl);
	}
	else
	{
		throw NotImplementedException(__func__);
	}
}


AutBase::StateBinaryRelation ExplicitTreeAutCore::ComputeUpwardSimulation(
	size_t                                 size,
	StateToStateTranslStrict&              /* transl */) const
{
	assert(false);
	std::vector<std::vector<size_t>> partition;

	AutBase::StateBinaryRelation relation;

	return this->TranslateUpward(
		partition, relation, Util::Identity(size)
	).computeSimulation(partition, relation, size);
}


StateBinaryRelation ExplicitTreeAutCore::ComputeDownwardSimulation(
	const SimParam&                        params,
	StateToStateTranslStrict&              transl) const
{
	if (params.GetNumStates() != static_cast<size_t>(-1))
	{
		return this->ComputeDownwardSimulation(params.GetNumStates(), transl);
	}
	else
	{
		throw NotImplementedException(__func__);
	}
}


StateBinaryRelation ExplicitTreeAutCore::ComputeDownwardSimulation(
	size_t                                 size,
	StateToStateTranslStrict&              /* transl */) const
{
	assert(false);
	return this->TranslateDownward().computeSimulation(size);
}
