// example3.cc - loading and dumping an automaton

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/explicit_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

const char* autStr =
	"Ops           a:0 b:2\n"
	"Automaton     aut\n"
	"States        q0 q1\n"
	"Final States  q1\n"
	"Transitions\n"
	"a          -> q0\n"
	"b(q0, q0)  -> q1\n";

typedef VATA::ExplicitTreeAut Automaton;
//typedef VATA::BDDBottomUpTreeAut Automaton;  // uncomment for BDD BU automaton
//typedef VATA::BDDTopDownTreeAut Automaton;   // uncomment for BDD TD automaton

int main()
{
	// create the parser for the Timbuk format
	std::unique_ptr<VATA::Parsing::AbstrParser> parser(
		new VATA::Parsing::TimbukParser());

	// create the dictionary for translating state names to internal state numbers
	VATA::AutBase::StateDict stateDict;

	// create and load the automaton
	Automaton aut;
	VATA::Util::AutDescription desc = parser->ParseString(autStr);
	aut.LoadFromAutDesc(desc, stateDict);

	// create the serializer for the Timbuk format
	VATA::Serialization::AbstrSerializer* serializer =
		new VATA::Serialization::TimbukSerializer();

	// dump the automaton
	std::cout << aut.DumpToString(*serializer, stateDict);
}
