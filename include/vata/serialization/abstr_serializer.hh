/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for abstract serialization class.
 *
 *****************************************************************************/

#ifndef _VATA_ABSTR_SERIALIZER_HH_
#define _VATA_ABSTR_SERIALIZER_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/aut_description.hh>


namespace VATA
{
	namespace Serialization
	{
		class AbstrSerializer;
	}
}


class VATA::Serialization::AbstrSerializer
{
public:   // data types

	typedef VATA::Util::AutDescription AutDescription;

public:   // methods

	virtual std::string Serialize(const AutDescription& desc) = 0;

	virtual ~AbstrSerializer()
	{ }
};


#endif
