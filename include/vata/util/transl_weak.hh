/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The header file of the weak translator class.
 *
 *****************************************************************************/

#ifndef _VATA_TRANSL_WEAK_HH_
#define _VATA_TRANSL_WEAK_HH_

// VATA headers
#include <vata/vata.hh>


namespace VATA
{
	namespace Util
	{
		template
		<
			class Cont
		>
		class TranslatorWeak;
	}
}

/**
 * @brief  Weak translator
 * 
 */
template
<
	class Cont
>
class VATA::Util::TranslatorWeak
{

private:  // data types

	typedef Cont Container;
	typedef typename Container::key_type InputType;
	typedef typename Container::mapped_type ResultType;

	typedef std::function<ResultType(InputType)> ResultAllocFuncType;

private:  // data members

	Container& container_;
	ResultAllocFuncType resultAllocFunc_;

public:   // methods

	TranslatorWeak(Container& container, ResultAllocFuncType resultAllocFunc) :
		container_(container),
		resultAllocFunc_(resultAllocFunc)
	{ }

	inline ResultType operator()(const InputType& value)
	{
		typename Container::const_iterator itCont;
		if ((itCont = container_.find(value)) != container_.end())
		{	// in case the value is known
			return itCont->second;
		}
		else
		{	// in case there is no translation for the value
			ResultType result = resultAllocFunc_(value);
			container_.insert(std::make_pair(value, result));

			return result;
		}
	}
};

#endif
