// example16-symbolic-incl.cc - symbolic inclusion with antichains and simulation

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

using BDDBUAut      = VATA::BDDBottomUpTreeAut;
using BDDTDAut      = VATA::BDDTopDownTreeAut;
using Convert       = VATA::Util::Convert;
using SimParam      = VATA::SimParam;
using InclParam     = VATA::InclParam;
using Rel           = BDDBUAut::StateDiscontBinaryRelation;

// example of an automaton
// it assumes 16-bit symbols
// to change the bit-width, modify the SYMBOL_SIZE constant in include/vata/symbolic.hh
std::string EXAMPLE_AUT =
	"Automaton A0053\n"
	"\n"
	"Final States q1 q2\n"
	"\n"
	"Transitions\n"
	"XXXXXXXXXXXXXXXX -> q3\n"
	"XXXXXXXXXXXXXXX1(q3) -> q1\n"
	"XXXXXXXXXXXXXXXX(q3) -> q2\n";

int main(int argc, char** argv)
{
	if (argc != 3)
	{	// in case no arguments were given
		std::cout << "VATA symbolic tree automaton inclusion with simulation checker\n";
		std::cout << "  usage: " << argv[0] << " <file1> <file2>\n";
		return EXIT_SUCCESS;
	}

	std::string aut1Str = VATA::Util::ReadFile(argv[1]);
	std::string aut2Str = VATA::Util::ReadFile(argv[2]);


	// create the parser for the Timbuk format
	std::unique_ptr<VATA::Parsing::AbstrParser> parser(
		new VATA::Parsing::TimbukParser());

	VATA::AutBase::StateDict sDict;

	BDDBUAut aut1;
	BDDBUAut aut2;

	size_t stateCnt = 0;
	BDDBUAut::StringToStateTranslWeak stateTr(sDict,
		[&stateCnt](const std::string&){return stateCnt++;});
	std::string params = "symbolic";
	aut1.LoadFromString(*parser, aut1Str, stateTr, params);
	aut2.LoadFromString(*parser, aut2Str, stateTr, params);

	// make a union and compute downward simulation (doable on bottom-up aut)
	BDDBUAut unionAut = BDDBUAut::UnionDisjointStates(aut1, aut2);
	SimParam sp;
	sp.SetRelation(SimParam::e_sim_relation::TA_DOWNWARD);
	sp.SetNumStates(stateCnt);
	Rel sim = unionAut.ComputeSimulation(sp);

	// invert the automata (we need top-down auts for the inclusion check)
	BDDTDAut aut1td = aut1.GetTopDownAut();
	BDDTDAut aut2td = aut2.GetTopDownAut();

	// set up params for inclusion testing
	InclParam ip;
	ip.SetAlgorithm(InclParam::e_algorithm::antichains);
	ip.SetDirection(InclParam::e_direction::downward);
	ip.SetUseRecursion(true);
	ip.SetUseSimulation(true);
	ip.SetSimulation(&sim);

	// do the test
	bool result = BDDTDAut::CheckInclusion(aut1td, aut2td, ip);

	std::cerr << ip.toString();
	std::cout << result << "\n";
}

