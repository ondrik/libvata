/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
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
	COMMAND_INTERSECTION,
	COMMAND_INCLUSION
};

enum RepresentationEnum
{
	REPRESENTATION_BDD
};

enum FormatEnum
{
	FORMAT_TIMBUK
};

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
		pruneUnreachable(false)
	{ }

};

Arguments parseArguments(int argc, char* argv[]);

#endif

