// example8.cc - how to generate inclusion counterexamples

// VATA headers
#include <vata/explicit_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>

const char* aut1Str =
	"Ops           a:0 b:2 c:2\n"
	"Automaton     aut\n"
	"States        q0 q1 q2\n"
	"Final States  q2\n"
	"Transitions\n"
	"a          -> q0\n"
	"b(q0, q0)  -> q0\n"
	"b(q0, q0)  -> q1\n"
	"b(q0, q1)  -> q1\n"
	"b(q1, q0)  -> q1\n"
	"c(q1, q1)  -> q2\n";

const char* aut2Str =
	"Ops           a:0 b:2 c:2\n"
	"Automaton     aut\n"
	"States        q0 q1 q2\n"
	"Final States  q2\n"
	"Transitions\n"
	"a          -> q0\n"
	"b(q0, q0)  -> q1\n"
	"b(q0, q1)  -> q1\n"
	"b(q1, q0)  -> q1\n"
	"c(q1, q1)  -> q2\n";

using Automaton         = VATA::ExplicitTreeAut;
using Transition        = Automaton::Transition;
using Convert           = VATA::Util::Convert;
using State             = Automaton::StateType;
using OnTheFlyAlphabet  = Automaton::OnTheFlyAlphabet;
using InclParam         = VATA::InclParam;
using InclContext       = Automaton::InclContext;

int main()
{
	// create the parser for the Timbuk format
	std::unique_ptr<VATA::Parsing::AbstrParser> parser(
		new VATA::Parsing::TimbukParser());

	// create the dictionaries for translating state names to internal state numbers
	VATA::AutBase::StateDict stateDict1;
	VATA::AutBase::StateDict stateDict2;

	// create and load the automata
	Automaton aut1;
	aut1.LoadFromString(*parser, aut1Str, stateDict1);
	Automaton aut2;
	aut2.LoadFromString(*parser, aut2Str, stateDict2);

	// check inclusion
	InclParam param;
	InclContext context;
	bool isIncl = Automaton::CheckInclusion(aut1, aut2, param, context);

	std::cout << "Inclusion check result: " << isIncl << "\n";
	if (!isIncl)
	{	// if inclusion does not hold, print a witness
		std::cout << context.GetDescription() << "\n";
	}
}
