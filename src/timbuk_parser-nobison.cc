/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The source code for the parser of Timbuk format.
 *
 *****************************************************************************/

// C++ headers
#include <regex>

// VATA headers
#include <vata/vata.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/util/aut_description.hh>

using VATA::Parsing::AbstrParser;
using VATA::Parsing::TimbukParser;
using VATA::Util::AutDescription;
using VATA::Util::Convert;


/**
 * @brief  Trim whitespaces from a string (both left and right)
 */
static std::string trim(const std::string& str)
{
	std::string result = str;

	// trim from start
	result.erase(result.begin(), std::find_if(result.begin(), result.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));

	// trim from end
	result.erase(std::find_if(result.rbegin(), result.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), result.end());

	return result;
}


/**
 * @brief  Split a string at newlines
 */
static std::vector<std::string> split_lines(const std::string& str)
{
	std::vector<std::string> result;

	std::string::size_type pos = 0;
	std::string::size_type prev = 0;
	while ((pos = str.find("\n", prev)) != std::string::npos)
	{
		result.push_back(str.substr(prev, pos - prev));
		prev = pos + 1;
	}

	// To get the last substring (or only, if delimiter is not found)
	result.push_back(str.substr(prev));

	return result;
}


/**
 * @brief  Read the first word from a string
 *
 * Reads the first word from a string and removes it from there
 */
static std::string read_word(std::string& str)
{
	std::string::iterator end(std::find_if(str.begin(), str.end(),
		std::ptr_fun<int, int>(std::isspace)));
	std::string result(str.begin(), end);

	str.erase(str.begin(), end);
	str = trim(str);
	return result;
}


/**
 * @brief  Parse a token of the form <string>:<number> or <string>
 */
static std::pair<std::string, int> parse_colonned_token(std::string str)
{
	str = trim(str);

	// no space inside
	assert(str.end() == std::find_if(str.begin(), str.end(), std::ptr_fun<int, int>(std::isspace)));

	size_t colon_pos = str.find(":");
	if (std::string::npos == colon_pos)
	{	// no colon found
		return std::make_pair(str, -1);
	}
	else
	{	// colon found
		std::string number_str = str.substr(colon_pos + 1);

		return std::make_pair(str.substr(0, colon_pos), Convert::FromString<int>(number_str));
	}
}

/**
 * @brief  Parse a string with Timbuk definition of an automaton
 */
static AutDescription parse_timbuk(const std::string& str)
{
	AutDescription result;

	bool are_transitions = false;
	bool aut_parsed = false;
	bool ops_parsed = false;
	bool states_parsed = false;
	std::vector<std::string> lines = split_lines(str);
	for (const std::string& line : lines)
	{
		std::string str = trim(line);
		if (str.empty()) { continue; }

		if (!are_transitions)
		{
			std::string first_word = read_word(str);
			if ("Transitions" == first_word)
			{
				are_transitions = true;
				continue;
			}
			else if ("Automaton" == first_word)
			{
				if (aut_parsed)
				{
					throw std::runtime_error(std::string(__FUNCTION__) + "Automaton already parsed!");
				}

				aut_parsed = true;

				result.name = read_word(str);

				if (!str.empty())
				{
					VATA_INFO("check the rest is empty ");
				}
			}
			else if ("Ops" == first_word)
			{
				if (ops_parsed)
				{
					throw std::runtime_error(std::string(__FUNCTION__) + "Ops already parsed!");
				}

				ops_parsed = true;

				while (!str.empty())
				{
					std::string label = read_word(str);
					auto label_num = parse_colonned_token(label);
					AutDescription::Symbol symb(label_num.first, label_num.second);

					result.symbols.insert(symb);
				}
			}
			else if ("States" == first_word)
			{
				if (states_parsed)
				{
					throw std::runtime_error(std::string(__FUNCTION__) + "States already parsed!");
				}

				states_parsed = true;

				while (!str.empty())
				{
					std::string state = read_word(str);
					auto state_num = parse_colonned_token(state);
					result.states.insert(state_num.first);
				}
			}
			else if ("Final" == first_word)
			{
				VATA_INFO("fuk");
			}
			else
			{	// guard
				throw std::runtime_error(std::string(__FUNCTION__) + ": line \"" + line +
					"\" contains an unexpected string");
			}
		}
		else
		{	// processing transitions
			VATA_INFO("Process transitions too");
		}
	}

	if (!are_transitions)
	{
		throw std::runtime_error(std::string(__FUNCTION__) + ": Transitions not specified");
	}

	return result;
}

AutDescription TimbukParser::ParseString(const std::string& str)
{
	AutDescription timbukParse;

	try
	{
		timbukParse = parse_timbuk(str);
	}
	catch (std::exception& ex)
	{
		throw std::runtime_error("Error: \'" + std::string(ex.what()) +
			"\' while parsing \n" + str);
	}

	return timbukParse;
}
