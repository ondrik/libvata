// example6.cc - advanced manipulation of explicit TAs

// VATA headers
#include <vata/explicit_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

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

	State s = 2;
	for (const Transition trans : aut[s])
	{	// for every transition in 'aut' from state 's' (top-down), print the internal representation
		std::cout << "Transition from state " << s << ": " << trans;
		std::cout << " [parent state = " << trans.GetParent();
		std::cout << ", symbol = " << trans.GetSymbol();
		std::cout << ", children states = " << Convert::ToString(trans.GetChildren());
		std::cout << "]\n";
	}

	std::cout << "And now all transitions with mapping to the input automaton:\n";
	for (const Transition trans : aut)
	{	// for every transition in 'aut', print the internal representation
		std::cout << "Transition: " << trans;
		std::cout << " [parent state = " << stateDict.TranslateBwd(trans.GetParent());
		std::cout << ", symbol = " << trans.GetSymbol();

		// how to do Haskell map in C++ ...
		std::vector<std::string> childrenStr(trans.GetChildren().size());
		std::transform(
			trans.GetChildren().cbegin(),
			trans.GetChildren().cend(),
			childrenStr.begin(),
			[&stateDict](const State s){return stateDict.TranslateBwd(s);});

		std::cout << ", children states = " << Convert::ToString(childrenStr);
		std::cout << "]\n";
	}

	// // create the serializer for the Timbuk format
	// VATA::Serialization::AbstrSerializer* serializer =
	// 	new VATA::Serialization::TimbukSerializer();
  //
	// // dump the automaton
	// std::cout << aut.DumpToString(*serializer, stateDict);
}
