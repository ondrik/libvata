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


StateDiscontBinaryRelation ExplicitTreeAutCore::ComputeUpwardSimulation(
	const SimParam&                        params) const
{
	assert(SimParam::e_sim_relation::TA_UPWARD == params.GetRelation());
	if (params.GetNumStates() != static_cast<size_t>(-1))
	{
		return this->ComputeUpwardSimulation(params.GetNumStates());
	}
	else
	{
		throw NotImplementedException(__func__);
	}
}


StateDiscontBinaryRelation ExplicitTreeAutCore::ComputeUpwardSimulation(
	size_t                                 size) const
{
	std::vector<std::vector<size_t>> partition;

	AutBase::StateBinaryRelation relation;

	StateToStateMap translMap;
	size_t stateCnt = 0;
	StateToStateTranslWeak transl(translMap, [&stateCnt](const StateType&)
		{return stateCnt++;});

	ExplicitLTS lts = this->TranslateUpward(partition, relation, VATA::Util::Identity(size), transl);
	StateBinaryRelation ltsSim = lts.computeSimulation(partition, relation, size);
	return StateDiscontBinaryRelation(ltsSim, translMap);
}


StateDiscontBinaryRelation ExplicitTreeAutCore::ComputeDownwardSimulation(
	const SimParam&                        params) const
{
	assert(SimParam::e_sim_relation::TA_DOWNWARD == params.GetRelation());
	if (params.GetNumStates() != static_cast<size_t>(-1))
	{
		return this->ComputeDownwardSimulation(params.GetNumStates());
	}
	else
	{
		// TODO: here, we should be able to compute the number of states in the TA
		throw NotImplementedException(__func__);
	}
}


StateDiscontBinaryRelation ExplicitTreeAutCore::ComputeDownwardSimulation(
	size_t                                 size) const
{
	StateToStateMap translMap;
	size_t stateCnt = 0;
	StateToStateTranslWeak transl(translMap, [&stateCnt](const StateType&)
		{return stateCnt++;});

	ExplicitLTS lts = this->TranslateDownward(size, transl);
	StateBinaryRelation ltsSim = lts.computeSimulation(size);
	return StateDiscontBinaryRelation(ltsSim, translMap);
}
