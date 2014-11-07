// example7.cc - how to use state and symbol translators

// VATA headers
#include <vata/explicit_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>

#include <memory>

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

using Automaton         = VATA::ExplicitTreeAut;
using Transition        = Automaton::Transition;
using Convert           = VATA::Util::Convert;
using State             = Automaton::StateType;
using OnTheFlyAlphabet  = Automaton::OnTheFlyAlphabet;

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
		std::cout << " [parent state = " << stateDict.TranslateBwd(trans.GetParent());
		std::cout << ", symbol = " <<
			aut.GetAlphabet()->GetSymbolBackTransl()->operator()(trans.GetSymbol());

		// just to show that the translation is ok... we translate backward and
		// forward and check that we get the same guy
		assert(trans.GetParent() ==
			stateDict.TranslateFwd(
			stateDict.TranslateBwd(
				trans.GetParent())));

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

	std::shared_ptr<OnTheFlyAlphabet> alph;
	if (!(alph = std::dynamic_pointer_cast<OnTheFlyAlphabet>(aut.GetAlphabet())))
	{	// the alphabet should be ExplicitTreeAut::OnTheFlyAlphabet
		assert(false);
	}

	for (auto stringSymbolPair : alph->GetSymbolDict())
	{	// for every transition in 'aut', print the internal representation
		std::cout << "Symbol " << stringSymbolPair.first << " ---> "
			<< stringSymbolPair.second << "\n";
	}
}

