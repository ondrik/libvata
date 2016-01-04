// example14.cc - playing with the complement

// VATA headers
#include <vata/explicit_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>

const char* autStr =
	"Ops\n"
	"Automaton     aut\n"
	"States        p q r z\n"
	"Final States  z\n"
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
	std::string autStr = VATA::Util::ReadFile("./200.timbuk");
	/* If I uncomment the following 3 lines (which just delete the
	alpabet line from the input string), the error thrown below is No
	translation for 0 instead of No translation for 2 */
	#if 0
	std::string prefix = "Ops\n";
	std::string rest = autStr.substr(autStr.find("Automaton"), autStr.length());
	autStr = prefix + rest;
	#endif
	Automaton aut;
	Automaton::AlphabetType onTheFlyAlph(new Automaton::OnTheFlyAlphabet);
	aut.SetAlphabet(onTheFlyAlph);

	size_t stateCnt = 0;
	Automaton::StringToStateTranslWeak stateTr(sDict,
		[&stateCnt](const std::string&){return stateCnt++;});
	aut.LoadFromString(*parser, autStr, stateTr);

	/* Compute dw simulation relation */
	SimParam sp;
	sp.SetRelation(SimParam::e_sim_relation::TA_DOWNWARD);
	sp.SetNumStates(stateCnt);
	Rel sim = aut.ComputeSimulation(sp);

	std::cout << "sim = " << sim << "\n";

	/* Quotient aut with the sim. */
	Automaton::StateToStateMap map;
	for (size_t i = 0; i < stateCnt; ++i)
	{
		for (size_t j = 0; j < stateCnt; ++j)
		{
			if (sim.get(i, j) && sim.get(j, i))
			{
				map.insert(std::make_pair(i, j));
			}
		}
	}

	aut = aut.CollapseStates(map);

	/* Complement the automaton. */
	aut  = aut.Complement();
}
