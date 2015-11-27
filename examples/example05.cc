// example4.cc - constructing a BDD bottom-up automaton using the API

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/bdd_td_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

// Checking inclusion of the following pair of automata
//
//  Automaton     aut1
//  States        q0 q1
//  Final States  q1
//  Transitions
//  0000          -> q0
//  11X0(q0, q0)  -> q1

//  Automaton     aut2
//  States        q2 q3
//  Final States  q3
//  Transitions
//  0000          -> q0
//  11X0(q0, q0)  -> q1
//  0XXX(q1, q1)  -> q1

using Automaton = VATA::BDDBottomUpTreeAut;  // uncomment for BDD BU automaton
//using Automaton = VATA::BDDTopDownTreeAut;   // uncomment for BDD TD automaton

int main()
{
	// create the automaton
	Automaton aut1;
	aut1.SetStateFinal(1);

	aut1.AddTransition(
		Automaton::StateTuple(),
		Automaton::SymbolType("0000000000000000"),
		0);
	aut1.AddTransition(
		Automaton::StateTuple({1, 1}),
		Automaton::SymbolType("11X011X011X011X0"),
		1);

	// the following would be nice... BUT IS NOT WORKING NOW!
	// We need to construct the automaton from the beginning
	#if 0
		Automaton aut2 = aut1;
		aut1.AddTransition(
			Automaton::StateTuple({1, 1}),
			Automaton::SymbolType("0XXX0XXX0XXX0XXX"),
			1);
	#endif

	Automaton aut2;
	aut1.AddTransition(
		Automaton::StateTuple(),
		Automaton::SymbolType("0000000000000000"),
		0);
	aut1.AddTransition(
		Automaton::StateTuple({1, 1}),
		Automaton::SymbolType("11X011X011X011X0"),
		1);
	aut1.AddTransition(
		Automaton::StateTuple({1, 1}),
		Automaton::SymbolType("0XXX0XXX0XXX0XXX"),
		1);

	// test inclusion
	std::cout << "aut1 <= aut2 : " << Automaton::CheckInclusion(aut1, aut2) << "\n";
}
