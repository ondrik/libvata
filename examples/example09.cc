// example9.cc - explicit downward simulation computation for TAs

// VATA headers
#include <vata/explicit_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

const char* autStr =
	"Ops\n"
	"Automaton     aut\n"
	"States        q r p s t u\n"
	"Final States  q r\n"
	"Transitions\n"
	"b          -> q\n"
	"a(p)       -> r\n"
	"c(r, s, t) -> u\n";

using Automaton     = VATA::ExplicitTreeAut;
using Convert       = VATA::Util::Convert;
using SimParam      = VATA::SimParam;
using Rel           = Automaton::StateDiscontBinaryRelation;

int main()
{
	// create the parser for the Timbuk format
	std::unique_ptr<VATA::Parsing::AbstrParser> parser(
		new VATA::Parsing::TimbukParser());

	// create the dictionary for translating state names to internal state numbers
	VATA::AutBase::StateDict stateDict;

	// create and load the automaton
	Automaton aut;
	aut.LoadFromString(*parser, autStr, stateDict);

	VATA::SimParam sp;
	sp.SetRelation(SimParam::e_sim_relation::TA_DOWNWARD);
	sp.SetNumStates(stateDict.size());
	Rel sim = aut.ComputeSimulation(sp);

	std::cout << aut.DumpToString(*(new VATA::Serialization::TimbukSerializer())) << "\n";
	std::cout << sim << "\n";
}
