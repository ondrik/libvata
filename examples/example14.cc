// example14.cc - playing with the complement

// VATA headers
#include <vata/explicit_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

const char* autStr =
	"Ops\n"
	"Automaton     aut\n"
	"States        \n"
	"Final States  \n"
	"Transitions\n"
	"no(p,q)       -> r\n";

using Automaton     = VATA::ExplicitTreeAut;
using Convert       = VATA::Util::Convert;
using SimParam      = VATA::SimParam;
using Rel           = Automaton::StateDiscontBinaryRelation;

int main()
{
	// create the parser for the Timbuk format
	std::unique_ptr<VATA::Parsing::AbstrParser> parser(
		new VATA::Parsing::TimbukParser());

	VATA::AutBase::StateDict sDict;

	/* Parse aut */
	// std::string autStr = VATA::Util::ReadFile("./200.timbuk");
	Automaton aut;
	Automaton::AlphabetType onTheFlyAlph(new Automaton::OnTheFlyAlphabet);
	aut.SetAlphabet(onTheFlyAlph);

	size_t stateCnt = 0;
	Automaton::StringToStateTranslWeak stateTr(sDict,
		[&stateCnt](const std::string&){return stateCnt++;});
	aut.LoadFromString(*parser, autStr, stateTr);

	/* Quotient aut with identity */
	Automaton::StateToStateMap map;
	for (size_t i = 0; i < stateCnt; ++i)
	{
		map.insert(std::make_pair(i, i));
	}

	aut = aut.CollapseStates(map);

	/* Complement the automaton. */
	aut  = aut.Complement();
}
