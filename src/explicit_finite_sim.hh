/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2014  Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for simulation computation for explicitly represented finite
 *    automata.
 *
 *****************************************************************************/

#ifndef _VATA_EXPLICIT_FINITE_SIM_HH_
#define _VATA_EXPLICIT_FINITE_SIM_HH_

template <class Index>
AutBase::StateBinaryRelation ExplicitFiniteAutCore::ComputeForwardSimulation(
	size_t              size,
	const Index&        index) const
{
	AutBase::StateBinaryRelation relation;
	std::vector<std::vector<size_t>> partition(1);

	return this->TranslateToLTS(partition, relation, index).computeSimulation(size);
}

#endif
