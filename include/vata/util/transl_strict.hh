/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The header file of the strict translator class.
 *
 *****************************************************************************/

#ifndef _VATA_TRANSL_STRICT_HH_
#define _VATA_TRANSL_STRICT_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/abstract_transl.hh>
#include <vata/util/convert.hh>

// Standard library headers
#include <stdexcept>


namespace VATA
{
	namespace Util
	{
		template <
			class Cont>
		class TranslatorStrict;

		template <
			class T>
		class IdentityTranslator;
	}
}

/**
 * @brief  Strict translator
 *
 */
template <
	class Cont>
class VATA::Util::TranslatorStrict :
	public AbstractTranslator<typename Cont::key_type, typename Cont::mapped_type>
{
private:  // data types

	typedef Cont Container;
	typedef typename Container::key_type InputType;
	typedef typename Container::mapped_type ResultType;

private:  // data members

	const Container& container_;

public:   // methods

	TranslatorStrict(const Container& container) :
		container_(container)
	{ }

	virtual ResultType operator()(const InputType& value) const override
	{
		typename Container::const_iterator itCont;
		if ((itCont = container_.find(value)) != container_.end())
		{	// in case the value is known
			return itCont->second;
		}
		else
		{	// in case there is no translation for the value
			throw std::runtime_error("No translation for " +
				VATA::Util::Convert::ToString(value));
		}
	}

	virtual ResultType operator()(const InputType& value) override
	{
		return const_cast<const TranslatorStrict*>(this)->operator()(value);
	}

	const Container& GetContainer() const
	{
		return container_;
	}
};

/**
 * @brief  Identity translator
 *
 */
template
<
	class T
>
class VATA::Util::IdentityTranslator :
	public AbstractTranslator<T, T>
{
public:   // methods

	virtual T operator()(const T& value) override
	{
		return value;
	}

	virtual T operator()(const T& value) const override
	{
		return value;
	}
};

#endif
