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

enum
{
	COMMAND_HELP,
	COMMAND_LOAD,
	COMMAND_UNION
};

enum
{
	REPRESENTATION_BDD
};

enum
{
	FORMAT_TIMBUK
};

struct Arguments
{
	size_t command;
	size_t representation;
	size_t inputFormat;
	size_t outputFormat;
	size_t operands;
	std::string fileName1;
	std::string fileName2;
};

Arguments parseArguments(int argc, char* argv[]);

#endif

