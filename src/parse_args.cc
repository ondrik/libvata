/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of argument parsing code.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>

// local headers
#include "parse_args.hh"

enum
{
	PARSING_COMMAND,
	PARSING_LOAD_FILE,
	PARSING_LOAD_2_FILES_1,
	PARSING_LOAD_2_FILES_2,
	PARSING_END
};


inline size_t translateFormat(const std::string& str)
{
	if (str == "timbuk")
	{
		return FORMAT_TIMBUK;
	}
	else
	{
		throw std::runtime_error("Unsupported format: " + str);
	}
}


Arguments parseArguments(int argc, char* argv[])
{
	// Assertions
	assert(argv != static_cast<char**>(0));

	// initialize parser state
	size_t parserState        = PARSING_COMMAND;
	bool parsedRepresentation = false;
	bool parsedInputFormat    = false;
	bool parsedOutputFormat   = false;

	// initialize the structure
	Arguments args;
	args.command         = COMMAND_HELP;
	args.representation  = REPRESENTATION_BDD;
	args.inputFormat     = FORMAT_TIMBUK;
	args.outputFormat    = FORMAT_TIMBUK;
	args.operands        = 0;

	while (argc > 0)
	{	// until we parse all arguments
		std::string currentArg = argv[0];
		if (currentArg[0] == '-')
		{	// in case there is a flag
			if ((currentArg == "-h") || currentArg == "--help")
			{
				args.command = COMMAND_HELP;
				parserState = PARSING_END;
			}
			else	if (currentArg == "-r")
			{
				if (parsedRepresentation)
				{
					throw std::runtime_error("The \'-r\' flag specified more times.");
				}

				parsedRepresentation = true;

				--argc;
				++argv;

				if (argc == 0)
				{
					throw std::runtime_error("The \'-r\' flag needs an argument.");
				}

				currentArg = argv[0];
				if (currentArg == "bdd")
				{
					args.representation = REPRESENTATION_BDD;
				}
				else
				{
					throw std::runtime_error("Unsupported representation: " + currentArg);
				}
			}
			else if (currentArg == "-I")
			{
				if (parsedInputFormat)
				{
					throw std::runtime_error("Invalid use of the \'-I\' flag.");
				}

				parsedInputFormat = true;

				--argc;
				++argv;

				if (argc == 0)
				{
					throw std::runtime_error("The \'-I\' flag needs an argument.");
				}

				args.inputFormat = translateFormat(argv[0]);
			}
			else if (currentArg == "-O")
			{
				if (parsedOutputFormat)
				{
					throw std::runtime_error("Invalid use of the \'-O\' flag.");
				}

				parsedOutputFormat = true;

				--argc;
				++argv;

				if (argc == 0)
				{
					throw std::runtime_error("The \'-O\' flag needs an argument.");
				}

				args.outputFormat = translateFormat(argv[0]);
			}
			else if (currentArg == "-F")
			{
				if (parsedInputFormat || parsedOutputFormat)
				{
					throw std::runtime_error("Invalid use of the \'-F\' flag.");
				}

				parsedInputFormat = true;
				parsedOutputFormat = true;

				--argc;
				++argv;

				if (argc == 0)
				{
					throw std::runtime_error("The \'-F\' flag needs an argument.");
				}

				args.inputFormat = translateFormat(argv[0]);
				args.outputFormat = args.inputFormat;
			}
			else
			{
				throw std::runtime_error("Invalid flag: " + currentArg);
			}
		}
		else
		{	// in case there is a non-flag
			if (parserState == PARSING_COMMAND)
			{
				if (currentArg == "help")
				{
					args.command = COMMAND_HELP;
					parserState = PARSING_END;
				}
				else if (currentArg == "load")
				{
					args.command   = COMMAND_LOAD;
					args.operands  = 1;

					parserState = PARSING_LOAD_FILE;
				}
				else if (currentArg == "union")
				{
					args.command   = COMMAND_UNION;
					args.operands  = 2;

					parserState = PARSING_LOAD_2_FILES_1;
				}
				else
				{
					throw std::runtime_error("Unknown command: " + currentArg);
				}
			}
			else if (parserState == PARSING_LOAD_FILE)
			{
				args.fileName1 = currentArg;
				parserState = PARSING_END;
			}
			else if (parserState == PARSING_LOAD_2_FILES_1)
			{
				args.fileName1 = currentArg;
				parserState = PARSING_LOAD_2_FILES_2;
			}
			else if (parserState == PARSING_LOAD_2_FILES_2)
			{
				args.fileName2 = currentArg;
				parserState = PARSING_END;
			}
			else
			{
				throw std::runtime_error("Invalid command line arguments: " + currentArg);
			}
		}

		--argc;
		++argv;
	}

	if (parserState != PARSING_END)
	{
		throw std::runtime_error("Invalid input arguments.");
	}

	return args;
}



