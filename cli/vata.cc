/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    The command-line interface to the VATA library.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/explicit_finite_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/convert.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/util.hh>

// standard library headers
#include <cstdlib>
#include <fstream>

// local headers
#include "parse_args.hh"
#include "operations.hh"


using VATA::AutBase;
using VATA::TreeAutBase;
using VATA::SymbolicTreeAutBase;
using VATA::BDDBottomUpTreeAut;
using VATA::BDDTopDownTreeAut;
using VATA::ExplicitTreeAut;
using VATA::ExplicitFiniteAut;
using VATA::Parsing::AbstrParser;
using VATA::Parsing::TimbukParser;
using VATA::Serialization::AbstrSerializer;
using VATA::Serialization::TimbukSerializer;
using VATA::Util::Convert;
using VATA::Util::TwoWayDict;

using StateDict                = AutBase::StateDict;
using StringToStateTranslWeak  = AutBase::StringToStateTranslWeak;
using StateBackTranslStrict    = AutBase::StateBackTranslStrict;
using StateToStateTranslStrict = AutBase::StateToStateTranslStrict;


const char VATA_USAGE_STRING[] =
	"VATA: VATA Tree Automata library interface\n"
	"usage: vata [-r <representation>] [(-I|-O|-F) <format>] [-h|--help] [-t] [-n]\n"
	"            [(-p|-s)] [-o <options>] <command> [<args>]\n"
	;

const char VATA_USAGE_COMMANDS[] =
	"\nThe following commands are supported:\n"
	"    help                    Display this message\n"
	"    load    <file>          Load automaton from <file>\n"
	"    witness <file>          Get a witness for automaton in <file>\n"
	"    cmpl    <file>          Complement automaton from <file> [experimental]\n"
	"    union <file1> <file2>   Compute union of automata from <file1> and <file2>\n"
	"    isect <file1> <file2>   Compute intersection of automata from <file1> and\n"
	"                            <file2>\n"
	"    sim <file>              Computes a simulation relation for the automaton in\n"
	"                            <file>. Options:\n"
	"\n"
	"          'dir=down' : downward simulation (default)\n"
	"          'dir=up'   : upward simulation\n"
	"\n"
	"    red <file>              Reduces the automaton in <file> using simulation\n"
	"                            relation. Options:\n"
	"\n"
	"          'dir=down' : downward simulation (default)\n"
	"          'dir=up'   : upward simulation\n"
	"\n"
	"    equiv <file1> <file2>   Checks language equivalence of finite automata from <file1>\n"
	"                            and <file2>, i.e., whether L(<file1>) is a equal\n"
	"                            to L(<file2>). Options\n"
	"          'order=depth': use depth-first search for congruence algorithm (default)\n"
	"          'order=breadth': use breadth-first search for congruence algorithm\n"
	"\n"
	"    incl <file1> <file2>    Checks language inclusion of automata from <file1>\n"
	"                            and <file2>, i.e., whether L(<file1>) is a subset\n"
	"                            of L(<file2>). Options:\n"
	"\n"
	"          'alg=antichains' : use an antichain-based algorithm (default)\n"
	"          'alg=congr'      : use a bisimulation up-to congruence algorithm\n"
	"          'dir=down' : downward inclusion checking\n"
	"          'dir=up'   : upward inclusion checking (default)\n"
	"          'sim=yes'  : use corresponding simulation\n"
	"          'sim=no'   : do not use simulation (default)\n"
	"          'order=depth': use depth-first search for congruence algorithm (default)\n"
	"          'order=breadth': use breadth-first search for congruence algorithm\n"
	"          'optC=yes' : use optimised cache for downward direction\n"
	"          'optC=no'  : without optimised cache (default)\n"
	"          'rec=no'   : recursive version of the algorithm (default)\n"
	"          'rec=yes'  : non-recursive version of the algorithm\n"
	"          'timeS=yes': include time of simulation computation (default)\n"
	"          'timeS=no' : do not include time of simulation computation\n"
	;

const char VATA_USAGE_FLAGS[] =
	"\nOptions:\n"
	"    -h, --help              Display this message\n"
	"    -r <representation>     Use <representation> for internal storage of\n"
	"                            automata. The following representations are\n"
	"                            supported:\n"
	"                               'bdd-td'   : binary decision diagrams,\n"
	"                                            top-down\n"
	"                               'bdd-bu'   : binary decision diagrams,\n"
	"                                            bottom-up\n"
	"                               'expl'     : explicit (default)\n"
	"                               'expl_fa'  : explicit finite automata\n"
	"\n"
	"    (-I|-O|-F) <format>     Specify format for input (-I), output (-O), or\n"
	"                            both (-F). The following formats are supported:\n"
	"                               'timbuk'  : Timbuk format (default)\n"
	"\n"
	"    -t                      Print the time the operation took to error output\n"
	"                            stream\n"
	"    -v                      Be verbose\n"
	"    -n                      Do not output the result automaton\n"
	"    -p                      Prune unreachable states first\n"
	"    -s                      Prune useless states first (note that this is\n"
	"                            stronger than -p)\n"
	"    -o <opt>=<v>,<opt>=<v>  Options in the form of a comma-separated\n"
	"                            <option>=<value> list\n"
	;

const size_t BDD_SIZE = 16;

timespec startTime;

void printHelp(bool full = false)
{
	std::cout << VATA_USAGE_STRING;

	if (full)
	{	// in case full help is wanted
		std::cout << VATA_USAGE_COMMANDS;
		std::cout << VATA_USAGE_FLAGS;
		std::cout << "\n\n";
	}
}


template <class Aut>
int performOperation(
	const Arguments&        args,
	AbstrParser&            parser,
	AbstrSerializer&        serializer)
{
	Aut autInput1;
	Aut autInput2;
	Aut autResult;
	bool boolResult = false;
	VATA::AutBase::StateBinaryRelation relResult;

	StateDict stateDict1;
	StateDict stateDict2;

	AutBase::StateToStateMap translMap1;

	if (args.operands >= 1)
	{
		autInput1.LoadFromString(
			parser,
			VATA::Util::ReadFile(args.fileName1),
			stateDict1);
	}

	if (args.operands >= 2)
	{
		autInput2.LoadFromString(
			parser,
			VATA::Util::ReadFile(args.fileName2),
			stateDict2);
	}

	if ((args.command == COMMAND_LOAD) ||
		(args.command == COMMAND_UNION) ||
		(args.command == COMMAND_COMPLEMENT) ||
		(args.command == COMMAND_INTERSECTION) ||
		(args.command == COMMAND_RED))
	{
		if (args.pruneUseless)
		{
			if (args.operands >= 1)
			{
				autInput1 = autInput1.RemoveUselessStates();
			}

			if (args.operands >= 2)
			{
				autInput2 = autInput2.RemoveUselessStates();
			}
		}
		else if (args.pruneUnreachable)
		{
			if (args.operands >= 1)
			{
				autInput1 = autInput1.RemoveUnreachableStates();
			}

			if (args.operands >= 2)
			{
				autInput2 = autInput2.RemoveUnreachableStates();
			}
		}
	}

	AutBase::StateToStateMap opTranslMap1;
	AutBase::StateToStateMap opTranslMap2;
	AutBase::ProductTranslMap prodTranslMap;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &startTime);     // set the timer

	timespec finishTime;

	// process command
	if (args.command == COMMAND_LOAD)
	{
		autResult = autInput1;
	}
	else if (args.command == COMMAND_WITNESS)
	{
		autResult = autInput1.GetCandidateTree();
	}
	else if (args.command == COMMAND_COMPLEMENT)
	{
		autResult = autInput1.Complement(autInput1.GetAlphabet());
	}
	else if (args.command == COMMAND_UNION)
	{
		autResult = Aut::Union(autInput1, autInput2, &opTranslMap1, &opTranslMap2);
	}
	else if (args.command == COMMAND_INTERSECTION)
	{
		autResult = Aut::Intersection(autInput1, autInput2, &prodTranslMap);
	}
	else if (args.command == COMMAND_INCLUSION)
	{
		boolResult = CheckInclusion(autInput1, autInput2, args);
	}
	else if (args.command == COMMAND_EQUIV)
	{
		boolResult = CheckEquiv(autInput1, autInput2, args);
	}
	else if (args.command == COMMAND_SIM)
	{
		relResult = ComputeSimulation(autInput1, args, stateDict1, translMap1);
	}
	else if (args.command == COMMAND_RED)
	{
		autResult = ComputeReduction(autInput1, args);
	}
	else
	{
		throw std::runtime_error("Internal error: invalid command");
	}

	// get the finish time
	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &finishTime))
	{
		throw std::runtime_error("Could not get the finish time");
	}
	double opTime = (finishTime.tv_sec - startTime.tv_sec)
		+ 1e-9 * (finishTime.tv_nsec - startTime.tv_nsec);

	if (args.showTime)
	{
		std::cerr << opTime << "\n";
	}

	if (!args.dontOutputResult)
	{	// in case output is not forbidden
		if ((args.command == COMMAND_LOAD) ||
			(args.command == COMMAND_WITNESS) ||
			(args.command == COMMAND_RED))
		{
			std::cout << autResult.DumpToString(serializer, stateDict1);
		}

		if (args.command == COMMAND_COMPLEMENT &&
			args.representation != REPRESENTATION_EXPLICIT_FA)
		{
			std::cout << autResult.DumpToString(serializer);
		}
		else if (args.command == COMMAND_COMPLEMENT)
		{
			std::cout << autResult.DumpToString(serializer, stateDict1);
		}

		if (args.command == COMMAND_UNION)
		{
			stateDict1 = VATA::Util::CreateUnionStringToStateMap(
				stateDict1, stateDict2, &opTranslMap1, &opTranslMap2);
		}

		if (args.command == COMMAND_INTERSECTION)
		{
			stateDict1 = VATA::Util::CreateProductStringToStateMap(
				stateDict1, stateDict2, prodTranslMap);
		}

		if ((args.command == COMMAND_UNION) ||
			(args.command == COMMAND_INTERSECTION))
		{
			std::cout << autResult.DumpToString(serializer, stateDict1);
		}
		if ((args.command == COMMAND_INCLUSION) || (args.command == COMMAND_EQUIV))
		{
			std::cout << boolResult << "\n";
		}

		if (args.command == COMMAND_SIM)
		{
			// std::cout << autInput1.PrintSimulationMapping(
			// 		StateBackTranslStrict(stateDict1.GetReverseMap()),
			// 		StateToStateTranslStrict(translMap1))
			// 	<< std::endl;

			TwoWayDict<
				AutBase::StateType,
				AutBase::StateType,
				AutBase::StateToStateMap
			> stateToIndexDict(translMap1);

			assert(stateDict1.size() == stateToIndexDict.size());
			size_t i = 0;
			for (const auto indexToStatePair : stateToIndexDict.GetReverseMap())
			{
				// check that the indices are correct
				assert(indexToStatePair.first == i);

				StateDict::ConstIteratorBwd it;
				if ((it = stateDict1.FindBwd(indexToStatePair.second)) == stateDict1.EndBwd())
				{
					assert(false);
				}

				std::cout << i << ": " << it->second << ", ";
				++i;
			}

			assert(stateDict1.size() == i);

			std::cout << "\n";
			std::cout << relResult << "\n";
		}
	}

	return EXIT_SUCCESS;
}


template <class Aut>
int executeCommand(const Arguments& args)
{
	std::unique_ptr<AbstrParser> parser(nullptr);
	std::unique_ptr<AbstrSerializer> serializer(nullptr);

	// create the input parser
	if (args.inputFormat == FORMAT_TIMBUK)
	{
		parser.reset(new TimbukParser());
	}
	else
	{
		throw std::runtime_error("Internal error: invalid input format");
	}

	// create the output serializer
	if (args.outputFormat == FORMAT_TIMBUK)
	{
		serializer.reset(new TimbukSerializer());
	}
	else
	{
		throw std::runtime_error("Internal error: invalid output format");
	}

	return performOperation<Aut>(args, *(parser.get()), *(serializer.get()));
}


int main(int argc, char* argv[])
{
	// Assertions
	assert(argc > 0);
	assert(argv != nullptr);

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
		std::cerr << "An error occured while parsing arguments: "
			<< ex.what() << "\n";
		printHelp(false);

		return EXIT_FAILURE;
	}

	if (args.command == COMMAND_HELP)
	{
		printHelp(true);
		return EXIT_SUCCESS;
	}

	AutBase::StateDict stateDict;

	try
	{
		if (args.representation == REPRESENTATION_BDD_TD)
		{
			return executeCommand<BDDTopDownTreeAut>(args);
		}
		else if (args.representation == REPRESENTATION_BDD_BU)
		{
			return executeCommand<BDDBottomUpTreeAut>(args);
		}
		else if (args.representation == REPRESENTATION_EXPLICIT)
		{
			return executeCommand<ExplicitTreeAut>(args);
		}
		else if (args.representation == REPRESENTATION_EXPLICIT_FA)
		{
			return executeCommand<ExplicitFiniteAut>(args);
		}
		else
		{
			std::cerr << "Internal error: invalid representation\n";
			return EXIT_FAILURE;
		}
	}
	catch (std::exception& ex)
	{
		std::cerr << "An error occured: " << ex.what() << "\n";
		return EXIT_FAILURE;
	}
}
