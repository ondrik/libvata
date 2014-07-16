/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The header file of the weak translator class.
 *
 *****************************************************************************/

#ifndef _VATA_TRANSL_WEAK_HH_
#define _VATA_TRANSL_WEAK_HH_

#include <functional>

// VATA headers
#include <vata/vata.hh>
#include <vata/util/abstract_transl.hh>


namespace VATA
{
	namespace Util
	{
		template <
			class Cont>
		class TranslatorWeak;

		template <
			class Cont>
		class TranslatorWeak2;
	}
}

/**
 * @brief  Weak translator
 *
 */
template <
	class Cont>
class VATA::Util::TranslatorWeak :
	public AbstractTranslator<typename Cont::key_type, typename Cont::mapped_type>
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

	TranslatorWeak(
		Container&               container,
		ResultAllocFuncType      resultAllocFunc) :
		container_(container),
		resultAllocFunc_(resultAllocFunc)
	{ }

	virtual ResultType operator()(const InputType& value) override
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

	virtual ResultType operator()(const InputType& value) const override
	{
		typename Container::const_iterator itCont;
		if ((itCont = container_.find(value)) != container_.end())
		{	// in case the value is known
			return itCont->second;
		}
		else
		{
			throw std::runtime_error("Cannot insert value into const translator.");
		}
	}
};

/**
 * @brief  Weak translator (ver 2)
 *
 */
template
<
	class Cont
>
class VATA::Util::TranslatorWeak2 :
	public AbstractTranslator<typename Cont::key_type, typename Cont::mapped_type>
{
private:  // data types

	typedef Cont Container;
	typedef typename Container::key_type InputType;
	typedef typename Container::mapped_type ResultType;

	typedef std::function<ResultType(const InputType&)> ResultAllocFuncType;

private:  // data members

	Container& container_;
	ResultAllocFuncType resultAllocFunc_;

public:   // methods

	TranslatorWeak2(
		Container&               container,
		ResultAllocFuncType      resultAllocFunc) :
		container_(container),
		resultAllocFunc_(resultAllocFunc)
	{ }

	virtual ResultType operator()(const InputType& value) override
	{
		auto p = container_.insert(std::make_pair(value, ResultType()));

		if (p.second)
		{	// in case there is no translation for the value
			p.first->second = resultAllocFunc_(p.first->first);
		}

		return p.first->second;
	}

	virtual ResultType operator()(const InputType& value) const override
	{
		typename Container::const_iterator itCont;
		if ((itCont = container_.find(value)) != container_.end())
		{	// in case the value is known
			return itCont->second;
		}
		else
		{
			throw std::runtime_error("Cannot insert value into const translator.");
		}
	}

	const Container& GetContainer() const
	{
		return container_;
	}
};

#endif
