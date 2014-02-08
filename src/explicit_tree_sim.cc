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


StateBinaryRelation ExplicitTreeAutCore::ComputeUpwardSimulation(
	const SimParam&          params) const
{
	if (params.GetNumStates() != static_cast<size_t>(-1))
	{
		return this->ComputeUpwardSimulation(params.GetNumStates());
	}
	else
	{
		throw NotImplementedException(__func__);
	}
}


StateBinaryRelation ExplicitTreeAutCore::ComputeDownwardSimulation(
	const SimParam&          params) const
{
	if (params.GetNumStates() != static_cast<size_t>(-1))
	{
		return this->ComputeDownwardSimulation(params.GetNumStates());
	}
	else
	{
		throw NotImplementedException(__func__);
	}
}


StateBinaryRelation ExplicitTreeAutCore::ComputeSimulation(
	const VATA::SimParam&                  params) const
{
	switch (params.GetRelation())
	{
		case SimParam::e_sim_relation::TA_UPWARD:
		{
			return this->ComputeUpwardSimulation(params);
		}
		case SimParam::e_sim_relation::TA_DOWNWARD:
		{
			return this->ComputeDownwardSimulation(params);
		}
		default:
		{
			throw std::runtime_error("Unknown simulation parameters: " + params.toString());
		}
	}
}

StateBinaryRelation ExplicitTreeAutCore::ComputeDownwardSimulation(
	size_t            size) const
{
	return this->TranslateDownward().computeSimulation(size);
}

