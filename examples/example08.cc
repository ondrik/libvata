// example8.cc - loading of small automata [buggy]

// VATA headers
#include <vata/explicit_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>

const char* autStr =
	"Ops           a:0\n"
	"Automaton     aut\n"
	"States        q0\n"
	"Final States  q0\n"
	"Transitions\n"
	"a          -> q0\n";

using Automaton     = VATA::ExplicitTreeAut;
using Transition    = Automaton::Transition;
using Convert       = VATA::Util::Convert;
using State         = Automaton::StateType;

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

	Automaton::CheckInclusion(aut, aut);
}
