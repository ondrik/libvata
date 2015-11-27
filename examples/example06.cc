// example6.cc - advanced manipulation of explicit TAs

// VATA headers
#include <vata/explicit_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>

const char* autStr =
	"Ops           a:0 b:2 c:2\n"
	"Automaton     aut\n"
	"States        q0 q1 q2\n"
	"Final States  q0 q2\n"
	"Transitions\n"
	"a          -> q0\n"
	"b(q0, q0)  -> q1\n"
	"b(q0, q1)  -> q1\n"
	"b(q1, q0)  -> q1\n"
	"c(q1, q1)  -> q2\n";

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

	for (const Transition trans : aut)
	{	// for every transition in 'aut', print the internal representation
		std::cout << "Transition: " << trans;
		std::cout << " [parent state = " << trans.GetParent();
		std::cout << ", symbol = " << trans.GetSymbol();
		std::cout << ", children states = " << Convert::ToString(trans.GetChildren());
		std::cout << "]\n";
	}

	for (const Transition trans : aut.GetAcceptTrans())
	{	// for every accepting transition in 'aut', print the internal representation
		std::cout << "Accepting transition: " << trans;
		std::cout << " [parent state = " << trans.GetParent();
		std::cout << ", symbol = " << trans.GetSymbol();
		std::cout << ", children states = " << Convert::ToString(trans.GetChildren());
		std::cout << "]\n";
	}

	std::string strState = "q1";
	State s = stateDict.TranslateFwd(strState);
	for (const Transition trans : aut[s])
	{	// for every transition in 'aut' from state 's' (top-down), print the internal representation
		std::cout << "Transition from state " << strState << ": " << trans;
		std::cout << " [parent state = " << trans.GetParent();
		std::cout << ", symbol = " << trans.GetSymbol();
		std::cout << ", children states = " << Convert::ToString(trans.GetChildren());
		std::cout << "]\n";
	}
}
