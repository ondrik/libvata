// example4.cc - constructing a BDD bottom-up automaton using the API

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

// The following automaton
//  Automaton     aut
//  States        q0 q1
//  Final States  q1
//  Transitions
//  0000000000000000          -> q0
//  11X011X011X011X0(q0, q0)  -> q1

using Automaton = VATA::BDDBottomUpTreeAut;  // uncomment for BDD BU automaton
//using Automaton = VATA::BDDTopDownTreeAut;   // uncomment for BDD TD automaton

int main()
{
	// create the automaton
	Automaton aut;
	aut.SetStateFinal(1);

	aut.AddTransition(
		Automaton::StateTuple(),
		Automaton::SymbolType("0000000000000000"),
		0);
	aut.AddTransition(
		Automaton::StateTuple({0, 0}),
		Automaton::SymbolType("11X011X011X011X0"),
		1);

	// create the serializer for the Timbuk format
	VATA::Serialization::AbstrSerializer* serializer =
		new VATA::Serialization::TimbukSerializer();

	// dump the automaton
	std::cout << aut.DumpToString(*serializer);
}
