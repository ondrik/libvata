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
	template <
		class Aut>
	class InclContext
	{
	public:   // data types

		using AutomatonType = Aut;

	private:  // data members

		std::string description_ = "";

		/**
		 * The witness to refuted inclusion check: an automaton the language of
		 *  which is a subset of L(smaller) \ L(bigger)
		 */
		AutomatonType witness_ = AutomatonType();

	public:   // methods

		void SetDescription(const std::string& desc)
		{
			description_ = desc;
		}

		const std::string& GetDescription() const
		{
			return description_;
		}

		const AutomatonType& GetWitness() const
		{
			return witness_;
		}

		void SetWitness(const AutomatonType& witness)
		{
			witness_ = witness;
		}

		template <
			class Aut2>
		void CopyGeneralFrom(const InclContext<Aut2>& ctx)
		{
			this->SetDescription(ctx.GetDescription());
		}
	};
}

#endif /* _INCL_CTX_HH_ */
