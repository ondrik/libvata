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
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/convert.hh>

// standard library headers
#include <cstdlib>

// local headers
#include "parse_args.hh"


using VATA::BDDTreeAut;
using VATA::Util::Convert;
using VATA::Parsing::AbstrParser;
using VATA::Parsing::TimbukParser;
using VATA::Serialization::AbstrSerializer;
using VATA::Serialization::TimbukSerializer;

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
void performLoad(const Arguments& args, AbstrParser& parser,
	AbstrSerializer& serializer)
{
	Aut aut;



	assert(false);
}


template <class Aut>
int executeCommand(const Arguments& args)
{
	std::auto_ptr<AbstrParser> parser(static_cast<AbstrParser*>(0));
	std::auto_ptr<AbstrSerializer> serializer(static_cast<AbstrSerializer*>(0));

	// create the input parser
	if (args.inputFormat == FORMAT_TIMBUK)
	{
		parser.reset(new TimbukParser());
	}
	else
	{
		std::cerr << "Internal error: invalid input format\n";
		return EXIT_FAILURE;
	}

	// create the output serializer
	if (args.outputFormat == FORMAT_TIMBUK)
	{
		serializer.reset(new TimbukSerializer());
	}
	else
	{
		std::cerr << "Internal error: invalid output format\n";
		return EXIT_FAILURE;
	}

	// process command
	if (args.command == COMMAND_LOAD)
	{
		performLoad<Aut>(args, *(parser.get()), *(serializer.get()));
	}
	else
	{
		std::cerr << "Internal error: invalid command\n";
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
