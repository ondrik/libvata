/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for argument parsing code.
 *
 *****************************************************************************/

#ifndef _PARSE_ARGS_HH_
#define _PARSE_ARGS_HH_

// VATA headers
#include <vata/vata.hh>

enum CommandEnum
{
	COMMAND_HELP,
	COMMAND_LOAD,
	COMMAND_UNION,
	COMMAND_EQUIV,
	COMMAND_INTERSECTION,
	COMMAND_INCLUSION,
	COMMAND_SIM,
	COMMAND_RED,
	COMMAND_WITNESS,
	COMMAND_COMPLEMENT
};

enum RepresentationEnum
{
	REPRESENTATION_BDD_TD,
	REPRESENTATION_BDD_BU,
	REPRESENTATION_EXPLICIT,
	REPRESENTATION_EXPLICIT_FA // Added FA
};

enum FormatEnum
{
	FORMAT_TIMBUK
};


typedef std::pair<std::string, std::string> OptionElement;
typedef std::map<OptionElement::first_type, OptionElement::second_type> Options;

struct Arguments
{
	CommandEnum command;
	RepresentationEnum representation;
	FormatEnum inputFormat;
	FormatEnum outputFormat;
	size_t operands;
	std::string fileName1;
	std::string fileName2;
	bool showTime;
	bool dontOutputResult;
	bool pruneUnreachable;
	bool pruneUseless;
	Options options;
	bool verbose;

	Arguments() :
		command(),
		representation(),
		inputFormat(),
		outputFormat(),
		operands(0),
		fileName1(),
		fileName2(),
		showTime(false),
		dontOutputResult(false),
		pruneUnreachable(false),
		pruneUseless(false),
		options(),
		verbose(false)
	{ }
};


Arguments parseArguments(int argc, char* argv[]);

#endif

