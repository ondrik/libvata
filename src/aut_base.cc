/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of the base class of automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/aut_base.hh>

using VATA::AutBase;

AutBase::StateType* AutBase::pNextState_ = nullptr;

