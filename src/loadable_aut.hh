/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the loadable automaton template.
 *
 *****************************************************************************/

#ifndef _VATA_LOADABLE_AUT_HH_
#define _VATA_LOADABLE_AUT_HH_

#include <vata/util/aut_description.hh>
#include <vata/util/convert.hh>

namespace VATA
{
	template <
		class>
	class LoadableAut;
}


GCC_DIAG_OFF(effc++)
template <
	class TBaseAut>
class VATA::LoadableAut : public TBaseAut
{
GCC_DIAG_ON(effc++)

public:   // data types

	using ParentAut        = TBaseAut;
	using StateDict        = typename TBaseAut::StateDict;
 	using AutDescription   = typename TBaseAut::AutDescription;
	using StateType        = typename TBaseAut::StateType;
	using StringSymbolType = typename TBaseAut::StringSymbolType;

	using StringToStateTranslWeak = typename TBaseAut::StringToStateTranslWeak;
	using StateBackTranslStrict   = typename TBaseAut::StateBackTranslStrict;

private:  // data types

	using Convert      = VATA::Util::Convert;

public:   // public methods

	// inherit all constructors
	using TBaseAut::TBaseAut;

//	/**
//	 * @brief  Implicit conversion from @p TBaseAut
//	 */
//	LoadableAut(const TBaseAut& baseAut) :
//		TBaseAut(baseAut)
//	{ }

	/**
	 * @brief  Implicit conversion from @p TBaseAut
	 */
	LoadableAut(TBaseAut&& baseAut) :
		TBaseAut(std::move(baseAut))
	{ }

	void LoadFromString(
		VATA::Parsing::AbstrParser&     parser,
		const std::string&              str,
		const std::string&              params = "")
	{
		this->LoadFromAutDesc(
			parser.ParseString(str),
			params);
	}


	void LoadFromString(
		VATA::Parsing::AbstrParser&     parser,
		const std::string&              str,
		StateDict&                      stateDict,
		const std::string&              params = "")
	{
		this->LoadFromAutDesc(
			parser.ParseString(str),
			stateDict,
			params);
	}


	template <
		class StateTranslFunc>
	void LoadFromString(
		VATA::Parsing::AbstrParser&     parser,
		const std::string&              str,
		StateTranslFunc                 stateTransl,
		const std::string&              params = "")
	{
		this->LoadFromAutDesc(
			parser.ParseString(str),
			stateTransl,
			params);
	}


	void LoadFromAutDesc(
		const AutDescription&         desc,
		const std::string&            params = "")
	{
		StateDict stateDict;

		this->LoadFromAutDesc(desc, stateDict, params);
	}


	void LoadFromAutDesc(
		const AutDescription&         desc,
		StateDict&                    stateDict,
		const std::string&            params = "")
	{
		StateType state(0);

		this->LoadFromAutDesc(
			desc,
			StringToStateTranslWeak(stateDict,
				[&state](const std::string&){return state++;}),
			params);
	}


	template <
		class StateTranslFunc>
	void LoadFromAutDesc(
		const AutDescription&         desc,
		StateTranslFunc               stateTransl,
		const std::string&            params = "")
	{
		this->loadFromAutDescInternal(
			desc,
			stateTransl,
			this->GetAlphabet()->GetSymbolTransl(),
			params);
	}


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const std::string&                         params = "") const
	{
		return this->DumpToString(
			serializer,
			[](const StateType& state){return Convert::ToString(state);},
			params);
	}


	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&      serializer,
		const StateDict&                           stateDict,
		const std::string&                         params = "") const
	{
		return this->DumpToString(
			serializer,
			StateBackTranslStrict(stateDict.GetReverseMap()),
			params);
	}


	template <
		class StateBackTranslFunc>
	std::string DumpToString(
		VATA::Serialization::AbstrSerializer&    serializer,
		StateBackTranslFunc                      stateBackTransl,
		const std::string&                       params = "") const
	{
		return this->dumpToStringInternal(
			serializer,
			stateBackTransl,
			this->GetAlphabet(),
			params);
	}
};

#endif
