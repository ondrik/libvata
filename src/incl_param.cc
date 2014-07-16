/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of the InclParam structure
 *
 *****************************************************************************/

// VATA headers
#include <vata/incl_param.hh>
#include <vata/util/convert.hh>


using VATA::Util::Convert;

std::string VATA::InclParam::toString() const
{
	std::string result;

	result += "Algorithm: ";
	switch (this->GetAlgorithm())
	{
		case e_algorithm::antichains:  result += "Antichains"; break;
		case e_algorithm::congruences: result += "Congruence"; break;
		default: assert(false);
	}

	result += "\n";
	result += "Direction: ";
	switch (this->GetDirection())
	{
		case e_direction::downward: result += "Downward"; break;
		case e_direction::upward:   result += "Upward";   break;
		default: assert(false);
	}

	result += "\n";
	result += "Downward inclusion caching implications: ";
	result += Convert::ToString(this->GetUseDownwardCacheImpl()) + "\n";

	result += "Recursive algorithm: ";
	result += Convert::ToString(this->GetUseRecursion()) + "\n";

	result += "Use simulation: ";
	result += Convert::ToString(this->GetUseSimulation()) + "\n";

	return result;
}
