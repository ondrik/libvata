/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of argument parsing code.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/util/convert.hh>

// local headers
#include "parse_args.hh"

enum ParsingEnum
{
	PARSING_COMMAND,
	PARSING_LOAD_FILE,
	PARSING_LOAD_2_FILES_1,
	PARSING_LOAD_2_FILES_2,
	PARSING_END
};


namespace
{
	inline FormatEnum translateFormat(const std::string& str)
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

	OptionElement processOption(const std::string& strOption)
	{
		if (strOption.empty())
		{
			throw std::runtime_error("Malformed options: \'" + strOption + "\'");
		}

		size_t equalPos;
		if ((equalPos = strOption.find('=')) != std::string::npos)
		{	// if there is a '='
			if ((equalPos == 0) || (equalPos == strOption.length() - 1))
			{
				throw std::runtime_error("Malformed option: \'" + strOption + "\'");
			}

			std::string option = strOption.substr(0, equalPos);
			std::string value  = strOption.substr(equalPos + 1);

			return std::make_pair(option, value);
		}
		else
		{	// if there is not a '='
			return std::make_pair(strOption, "");
		}
	}
}


Arguments parseArguments(int argc, char* argv[])
{
	// Assertions
	assert(argv != nullptr);

	// initialize parser state
	ParsingEnum parserState   = PARSING_COMMAND;
	bool parsedRepresentation = false;
	bool parsedInputFormat    = false;
	bool parsedOutputFormat   = false;
	bool parsedShowTime       = false;
	bool parsedDontOutputRes  = false;
	bool parsedPruneUnreach   = false;
	bool parsedPruneUseless   = false;
	bool parsedOptions        = false;
	bool parsedVerbose        = false;

	// initialize the structure
	Arguments args;
	args.command              = COMMAND_HELP;
	args.representation       = REPRESENTATION_EXPLICIT;
	args.inputFormat          = FORMAT_TIMBUK;
	args.outputFormat         = FORMAT_TIMBUK;
	args.operands             = 0;
	args.fileName1            = "";
	args.fileName2            = "";
	args.showTime             = false;
	args.dontOutputResult     = false;
	args.pruneUnreachable     = false;
	args.pruneUseless         = false;
	args.options              = { };
	args.verbose              = false;

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
			else	if (currentArg == "-t")
			{
				if (parsedShowTime)
				{
					throw std::runtime_error("The \'-t\' flag specified more times.");
				}

				parsedShowTime = true;
				args.showTime = true;
			}
			else	if (currentArg == "-v")
			{
				if (parsedVerbose)
				{
					throw std::runtime_error("The \'-v\' flag specified more times.");
				}

				parsedVerbose = true;
				args.verbose = true;
			}
			else	if (currentArg == "-p")
			{
				if (parsedPruneUnreach)
				{
					throw std::runtime_error("The \'-p\' flag specified more times.");
				}

				parsedPruneUnreach = true;
				args.pruneUnreachable = true;
			}
			else	if (currentArg == "-s")
			{
				if (parsedPruneUseless)
				{
					throw std::runtime_error("The \'-s\' flag specified more times.");
				}

				parsedPruneUseless = true;
				args.pruneUseless = true;
			}
			else	if (currentArg == "-n")
			{
				if (parsedDontOutputRes)
				{
					throw std::runtime_error("The \'-n\' flag specified more times.");
				}

				parsedDontOutputRes = true;
				args.dontOutputResult = true;
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
				if (currentArg == "bdd-td")
				{
					args.representation = REPRESENTATION_BDD_TD;
				}
				else if (currentArg == "bdd-bu")
				{
					args.representation = REPRESENTATION_BDD_BU;
				}
				else if (currentArg == "expl")
				{
					args.representation = REPRESENTATION_EXPLICIT;
				}
				else if (currentArg == "expl_fa")
				{
					args.representation = REPRESENTATION_EXPLICIT_FA;
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
			else	if (currentArg == "-o")
			{
				if (parsedOptions)
				{
					throw std::runtime_error("The \'-o\' flag specified more times.");
				}

				parsedOptions = true;

				--argc;
				++argv;

				if (argc == 0)
				{
					throw std::runtime_error("The \'-o\' flag needs an argument.");
				}

				currentArg = argv[0];
				size_t lastPos = 0;
				size_t newPos;
				while ((newPos = currentArg.find(',', lastPos)) != std::string::npos)
				{
					OptionElement option = processOption(
						currentArg.substr(lastPos, newPos - lastPos));

					if (!args.options.insert(option).second)
					{
						throw std::runtime_error("Option for \'" + option.first +
							"\' specified more than once");
					}

					lastPos = newPos + 1;
				}

				OptionElement option = processOption(
					currentArg.substr(lastPos, newPos - lastPos));

				if (!args.options.insert(option).second)
				{
					throw std::runtime_error("Option for \'" + option.first +
						"\' specified more than once");
				}
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
				else if (currentArg == "witness")
				{
					args.command   = COMMAND_WITNESS;
					args.operands  = 1;

					parserState = PARSING_LOAD_FILE;
				}
				else if (currentArg == "cmpl")
				{
					args.command   = COMMAND_COMPLEMENT;
					args.operands  = 1;

					parserState = PARSING_LOAD_FILE;
				}
				else if (currentArg == "union")
				{
					args.command   = COMMAND_UNION;
					args.operands  = 2;

					parserState = PARSING_LOAD_2_FILES_1;
				}
				else if (currentArg == "isect")
				{
					args.command   = COMMAND_INTERSECTION;
					args.operands  = 2;

					parserState = PARSING_LOAD_2_FILES_1;
				}
				else if (currentArg == "sim")
				{
					args.command   = COMMAND_SIM;
					args.operands  = 1;

					parserState = PARSING_LOAD_FILE;
				}
				else if (currentArg == "red")
				{
					args.command   = COMMAND_RED;
					args.operands  = 1;

					parserState = PARSING_LOAD_FILE;
				}
				else if (currentArg == "incl")
				{
					args.command   = COMMAND_INCLUSION;
					args.operands  = 2;

					parserState = PARSING_LOAD_2_FILES_1;
				}
				else if (currentArg == "equiv")
				{
					args.command   = COMMAND_EQUIV;
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



