/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a serializer of automata to Timbuk format.
 *
 *****************************************************************************/

#ifndef _VATA_TIMBUK_SERIALIZER_HH_
#define _VATA_TIMBUK_SERIALIZER_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/serialization/abstr_serializer.hh>

namespace VATA
{
	namespace Serialization
	{
		class TimbukSerializer;
	}
}

/**
 * @brief  Class for a serializer of automata into the Timbuk format
 *
 * This class is a serializer of automata into the Timbuk format.
 */
class VATA::Serialization::TimbukSerializer :
	public VATA::Serialization::AbstrSerializer
{
public:   // data types

	typedef VATA::Util::AutDescription AutDescription;

private:  // data members

	std::string name_;

public:   // methods

	TimbukSerializer() :
		name_("anonymous")
	{ }

	inline void SetName(const std::string& name)
	{
		name_ = name;
	}

	virtual std::string Serialize(const AutDescription& desc);
};

#endif
