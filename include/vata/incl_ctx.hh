/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2014  Ondrej Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the InclContext structure
 *
 *****************************************************************************/

#ifndef _INCL_CTX_HH_
#define _INCL_CTX_HH_

namespace VATA
{
	class InclContext
	{
	private:

		std::string description_ = "";

	public:

		void SetDescription(const std::string& desc)
		{
			description_ = desc;
		}

		const std::string& GetDescription() const
		{
			return description_;
		}
	};
}

#endif /* _INCL_CTX_HH_ */
