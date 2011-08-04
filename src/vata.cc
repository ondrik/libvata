/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The command-line interface to the VATA library.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_tree_aut.hh>
#include <vata/util/convert.hh>

// standard library headers
#include <cstdlib>

// local headers
#include "parse_args.hh"


using VATA::BDDTreeAut;
using VATA::Util::Convert;


const char VATA_USAGE_STRING[] =
	"usage: vata [-r <representation>] [(-F|-I|-O) <format>] <command> [<args>]\n"
	;

const char VATA_USAGE_COMMANDS[] =
	"help\n"
	"load\n"
	"union\n"
	;


void printHelp(bool full = false)
{
	std::cout << VATA_USAGE_STRING;

	if (full)
	{	// in case full help is wanted
		std::cout << VATA_USAGE_COMMANDS;
	}
}

template <class Aut>
void performLoad(const Arguments& args)
{

}


template <class Aut>
int executeCommand(const Arguments& args)
{
	if (args.command == COMMAND_LOAD)
	{
		performLoad<Aut>(args);
	}
	else
	{
		std::cerr << "Internal error: invalid command\n";
		printHelp(false);
		return EXIT_FAILURE;
	}



	return EXIT_SUCCESS;
}


int main(int argc, char* argv[])
{
	// Assertions
	assert(argc > 0);
	assert(argv != static_cast<char**>(0));

	if (argc == 1)
	{	// in case no arguments were given
		printHelp(true);
		return EXIT_SUCCESS;
	}

	--argc;
	++argv;
	Arguments args;

	try
	{
		args = parseArguments(argc, argv);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An error occured: " << ex.what() << "\n";
		printHelp(false);

		return EXIT_FAILURE;
	}

	if (args.command == COMMAND_HELP)
	{
		printHelp(true);
		return EXIT_SUCCESS;
	}

	if (args.representation == REPRESENTATION_BDD)
	{
		return executeCommand<BDDTreeAut>(args);
	}
	else
	{
		std::cerr << "Internal error: invalid representation\n";
		return EXIT_FAILURE;
	}
}
