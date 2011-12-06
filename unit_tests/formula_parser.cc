/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of FormulaParser class.
 *
 *****************************************************************************/

#include "formula_parser.hh"

// VATA headers
#include <vata/util/convert.hh>
#include <vata/vata.hh>

using VATA::Util::Convert;

FormulaParser::ParserResultBooleanType
	FormulaParser::ParseExpressionBoolean(std::string input)
{
	boost::trim(input);
	if (input.empty())
	{	// in case the input is empty
		return VariableListType();
	}

	std::vector<std::string> splitInput;
	boost::algorithm::split(splitInput, input, boost::is_any_of("*"));

	if (splitInput.empty())
	{	// in case the splitting failed
		throw std::runtime_error("boost::algorithm::split() returned "
			"an invalid result");
	}

	VariableListType varList;
	for (std::vector<std::string>::const_iterator it = splitInput.begin();
		it != splitInput.end(); ++it)
	{
		std::string str = boost::trim_copy(*it);

		if (str.length() == 0)
		{
			throw std::invalid_argument(__func__ + std::string(": invalid argument"));
		}

		VariableOccurenceType var("", true);

		if (str[0] == '~')
		{	// in case the variable is complemented
			var.second = false;
			str = str.substr(1);
		}

		var.first = str;

		varList.push_back(var);
	}

	return varList;
}


FormulaParser::ParserResultUnsignedType
	FormulaParser::ParseExpressionUnsigned(std::string input)
{
	boost::trim(input);
	std::vector<std::string> splitInput;
	boost::algorithm::split(splitInput, input, boost::is_any_of("="));

	if (splitInput.size() != 2)
	{	// in case the input looks strange
		throw std::invalid_argument(__func__ + std::string(": invalid argument"));
	}

	return ParserResultUnsignedType(Convert::FromString<unsigned>(splitInput[1]),
		ParseExpressionBoolean(splitInput[0]));
}


FormulaParser::ParserResultUnsignedVecType
	FormulaParser::ParseExpressionUnsignedVec(std::string input)
{
	boost::trim(input);
	std::vector<std::string> splitInput;
	boost::algorithm::split(splitInput, input, boost::is_any_of("="));

	if (splitInput.size() != 2)
	{	// in case the input looks strange
		throw std::invalid_argument(__func__ + std::string(": invalid argument "
			"(not in the \"<formula> = <values>\" format)"));
	}

	std::string& secondPart = splitInput[1];
	boost::trim(secondPart);

	if ((secondPart[0] != '{') || (secondPart[secondPart.size() - 1] != '}'))
	{	// in case the input is malformated
		throw std::invalid_argument(__func__ + std::string(": invalid argument "
			"(values are not in the \"{ <list> }\" format)"));
	}

	secondPart = secondPart.substr(1, secondPart.length() - 2);
	boost::trim(secondPart);

	if (secondPart.empty())
	{	// in case the set is empty
		return ParserResultUnsignedVecType(std::vector<unsigned>(),
			ParseExpressionBoolean(splitInput[0]));
	}

	std::vector<std::string> splitSecondPart;
	boost::algorithm::split(splitSecondPart, secondPart, boost::is_any_of(","));

	if (splitSecondPart.size() < 1)
	{	// in case the input looks strange
		throw std::invalid_argument(__func__ + std::string(": invalid argument "
			"(list of values is not in the \"<value>, ... , <value>\" format)"));
	}


	std::vector<unsigned> resultVec;
	for (std::vector<std::string>::const_iterator itStr
		= splitSecondPart.begin(); itStr != splitSecondPart.end(); ++itStr)
	{	// convert all symbols
		resultVec.push_back(Convert::FromString<unsigned>(*itStr));
	}

	return ParserResultUnsignedVecType(resultVec,
		ParseExpressionBoolean(splitInput[0]));
}


std::string FormulaParser::parserResultToString(
	const ParserResultUnsignedType& prsRes)
{
	std::string result = Convert::ToString(prsRes.first);

	for (VariableListType::const_iterator itVars = prsRes.second.begin();
		itVars != prsRes.second.end(); ++itVars)
	{	// print all variables
		result += " * " + Convert::ToString(((itVars->second)? " " : "~"))
			+ itVars->first;
	}

	return result;
}
