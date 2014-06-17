/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2014  Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implemention of simulation computation for explicitly represented finite
 *    automata.
 *
 *****************************************************************************/

#include "explicit_finite_aut_core.hh"
#include "explicit_finite_translate.hh"

using VATA::AutBase;
using VATA::ExplicitFiniteAutCore;

AutBase::StateDiscontBinaryRelation ExplicitFiniteAutCore::ComputeSimulation(
	const SimParam&     /* params */) const
{
	assert(false);
	// switch (params.GetRelation())
	// {
	// 	case SimParam::e_sim_relation::FA_FORWARD:
	// 	{
	// 		return this->ComputeForwardSimulation(params);
	// 	}
	// 	case SimParam::e_sim_relation::FA_BACKWARD:
	// 	{
	// 		return this->ComputeBackwardSimulation(params);
	// 	}
	// 	default:
	// 	{
	// 		throw std::runtime_error("Unknown simulation parameters: " + params.toString());
	// 	}
	// }
}

AutBase::StateBinaryRelation ExplicitFiniteAutCore::ComputeForwardSimulation(
	const SimParam&     params) const
{
	if (params.GetNumStates() != static_cast<size_t>(-1))
	{
		return this->ComputeForwardSimulation(params.GetNumStates());
	}
	else
	{
		throw NotImplementedException(__func__);
	}
}


AutBase::StateBinaryRelation ExplicitFiniteAutCore::ComputeForwardSimulation(
	size_t              size) const
{
	return this->TranslateToLTS().computeSimulation(size);
}


AutBase::StateBinaryRelation ExplicitFiniteAutCore::ComputeBackwardSimulation(
	const SimParam&     params) const
{
	if (params.GetNumStates() != static_cast<size_t>(-1))
	{
		return this->ComputeBackwardSimulation(params.GetNumStates());
	}
	else
	{
		throw NotImplementedException(__func__);
	}
}


AutBase::StateBinaryRelation ExplicitFiniteAutCore::ComputeBackwardSimulation(
	size_t              /* size */) const
{
	throw NotImplementedException(__func__);
}
