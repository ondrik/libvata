/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Test suite for explicit tree automaton
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/explicit_tree_aut.hh>

#include "log_fixture.hh"

/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path UNREACHABLE_TIMBUK_FILE =
	AUT_DIR / "td_unreachable_removal_timbuk.txt";

/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

class AutTypeFixture
{
protected:// data types

	typedef VATA::ExplicitTreeAut AutType;

protected:// data members

	AutType::SymbolType nextSymbol_;

protected:// methods

	AutTypeFixture() :
		nextSymbol_(0)
	{ }

	virtual ~AutTypeFixture() { }
};

#include "tree_aut_test.hh"

BOOST_AUTO_TEST_CASE(aut_down_simulation)
{
	testDownwardSimulation();
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_nonrec_nosim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::downward);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_nonrec_sim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::downward);
	ip.SetUseSimulation(true);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_rec_sim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::downward);
	ip.SetUseRecursion(true);
	ip.SetUseSimulation(true);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_down_inclusion_opt_rec_sim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::downward);
	ip.SetUseDownwardCacheImpl(true);
	ip.SetUseRecursion(true);
	ip.SetUseSimulation(true);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_up_inclusion)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::upward);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(aut_up_inclusion_sim)
{
	VATA::InclParam ip;
	ip.SetDirection(InclParam::e_direction::upward);
	ip.SetUseSimulation(true);
	testInclusion(ip);
}

BOOST_AUTO_TEST_CASE(iterators)
{
	this->runOnAutomataSet(
		[](const AutType& aut, const StateDict& stateDict, const std::string& filename)
		{
			BOOST_MESSAGE("Checking iterators for " + filename + "...");
			for (const Transition& trans : aut)
			{
				BOOST_REQUIRE_MESSAGE(aut.ContainsTransition(trans),
					"Inconsistent iterator output: " + aut.ToString(trans) +
					" is claimed not to be in aut");
			}
		});
}

BOOST_AUTO_TEST_CASE(iterators_dereference)
{
	this->runOnAutomataSet(
		[](const AutType& aut, const StateDict& stateDict, const std::string& filename)
		{
			BOOST_MESSAGE("Checking iterators dereference for " + filename + "...");
			for (AutType::const_iterator it = aut.begin(); it != aut.end(); ++it)
			{
				BOOST_REQUIRE_MESSAGE(aut.ContainsTransition(*it),
					"Inconsistent iterator output: " + aut.ToString(*it) +
					" is claimed not to be in aut");
			}
		});
}

BOOST_AUTO_TEST_CASE(accept_iterators)
{
	this->runOnAutomataSet(
		[](const AutType& aut, const StateDict& stateDict, const std::string& filename)
		{
			BOOST_MESSAGE("Checking accepting transitions iterators for " + filename + "...");
			for (const Transition& trans : aut.GetAcceptTrans())
			{
				BOOST_REQUIRE_MESSAGE(aut.ContainsTransition(trans),
					"Inconsistent iterator output: " + aut.ToString(trans) +
					" is claimed not to be in aut");

				BOOST_REQUIRE_MESSAGE(aut.IsStateFinal(trans.GetParent()),
					"Inconsistent iterator output: " + aut.ToString(trans) +
					" is not accepting");
			}
		});
}

BOOST_AUTO_TEST_CASE(accept_iterators_dereference)
{
	this->runOnAutomataSet(
		[](const AutType& aut, const StateDict& stateDict, const std::string& filename)
		{
			BOOST_MESSAGE("Checking accepting transitions iterators dereference for " + filename + "...");
			for (AutType::AcceptTrans::const_iterator it = aut.GetAcceptTrans().begin();
				it != aut.GetAcceptTrans().end(); ++it)
			{
				BOOST_REQUIRE_MESSAGE(aut.ContainsTransition(*it),
					"Inconsistent iterator output: " + aut.ToString(*it) +
					" is claimed not to be in aut");

				BOOST_REQUIRE_MESSAGE(aut.IsStateFinal(it->GetParent()),
					"Inconsistent iterator output: " + aut.ToString(*it) +
					" is not accepting");
			}
		});
}

// tests AutType::begin(state), AutType::end(state)
BOOST_AUTO_TEST_CASE(iterators_for_state)
{
	this->runOnAutomataSet(
		[](const AutType& aut, const StateDict& stateDict, const std::string& filename)
		{
			BOOST_MESSAGE("Checking state iterators for " + filename + "...");

			std::set<Transition> accTransitions1;
			std::set<Transition> accTransitions2;

			// fill in accTransitions1:
			for (const StateType& state : aut.GetFinalStates())
			{
				for (const Transition& trans : aut[state])
				{
					accTransitions1.insert(trans);
				}
			}

			// fill in accTransitions2
			for (const Transition& trans : aut.GetAcceptTrans())
			{
				accTransitions2.insert(trans);
			}

			BOOST_REQUIRE_MESSAGE(accTransitions1 == accTransitions2,
				"The sets of accepting transitions do not match: " +
				Convert::ToString(accTransitions1) + " and " +
				Convert::ToString(accTransitions2));
		});
}

BOOST_AUTO_TEST_CASE(reindex_states_functor)
{
	class IncrementReindexF : public VATA::AbstractReindexF
	{
	private:

		size_t offset_;

	public:

		explicit IncrementReindexF(const size_t& offset) :
			offset_(offset)
		{ }

		virtual StateType operator[](const StateType& state)
		{
			return state + offset_;
		}
	};

	auto testfileContent = ParseTestFile(LOAD_TIMBUK_FILE.string());

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 1, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string filename = (AUT_DIR / testcase[0]).string();
		BOOST_MESSAGE("Functor-reindexing of states of automaton " + filename + "...");
		std::string autStr = VATA::Util::ReadFile(filename);

		StateType state(0);

		AutType aut;
		StateDict stateDict;
		StringToStateTranslWeak stateTransl(
			/* state dictionary */ stateDict,
			/* generator of new states */ [&state](const std::string&){return state++;});

		readAut(aut, stateTransl, autStr);

		IncrementReindexF fctor(state);
		AutType newAut = aut.ReindexStates(fctor);

		for (const Transition& trans : newAut)
		{
			BOOST_REQUIRE_MESSAGE(trans.GetParent() >= state, "Parent state inconsistent!");

			for (const StateType& child : trans.GetChildren())
			{
				BOOST_REQUIRE_MESSAGE(child >= state, "Child state inconsistent!");
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(inherited_alphabet_type)
{
	auto testfileContent = ParseTestFile(LOAD_TIMBUK_FILE.string());

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 1, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string filename = (AUT_DIR / testcase[0]).string();
		BOOST_MESSAGE("Checking DirectAlphabet on automaton " + filename + "...");
		std::string autStr = VATA::Util::ReadFile(filename);

		StateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);

		// now let's change to the DirectAlphabet alphabet
		AutType::AlphabetType directAlph(new AutType::DirectAlphabet);
		aut.SetAlphabet(directAlph);

		std::string autOut = dumpAut(aut, stateDict);
		AutDescription descOut = parser_.ParseString(autOut);

		AutDescription descManual;
		for (const Transition& trans : aut)
		{
			descManual.states.insert(stateDict.TranslateBwd(trans.GetParent()));

			AutDescription::StateTuple tuple;
			for (const StateType& state : trans.GetChildren())
			{
				descManual.states.insert(stateDict.TranslateBwd(state));
				tuple.push_back(stateDict.TranslateBwd(state));
			}

			descManual.symbols.insert(AutDescription::Symbol(
				Convert::ToString(trans.GetSymbol()),
				trans.GetChildren().size()));

			descManual.transitions.insert(AutDescription::Transition(
				tuple,
				Convert::ToString(trans.GetSymbol()),
				stateDict.TranslateBwd(trans.GetParent())));
		}

		for (const StateType& finState : aut.GetFinalStates())
		{
			descManual.finalStates.insert(stateDict.TranslateBwd(finState));
			descManual.states.insert(stateDict.TranslateBwd(finState));
		}

		BOOST_CHECK_MESSAGE(descManual == descOut,
			std::string("\n\nInvalid output.") +
			"===========\n\nGot:\n===========\n" + autOut + "\n===========");
	}
}

BOOST_AUTO_TEST_CASE(translate_symbols)
{
	class SymbolTranslPlus42 : public VATA::ExplicitTreeAut::AbstractSymbolTranslateF
	{
		virtual SymbolType operator()(const SymbolType& sym) override
		{
			return 42 + sym;
		}
	};

	auto testfileContent = ParseTestFile(LOAD_TIMBUK_FILE.string());

	for (auto testcase : testfileContent)
	{
		BOOST_REQUIRE_MESSAGE(testcase.size() == 1, "Invalid format of a testcase: " +
			Convert::ToString(testcase));

		std::string filename = (AUT_DIR / testcase[0]).string();
		BOOST_MESSAGE("Checking TranslateSymbols on automaton " + filename + "...");
		std::string autStr = VATA::Util::ReadFile(filename);

		StateDict stateDict;
		AutType aut;
		readAut(aut, stateDict, autStr);

		// now let's change to the DirectAlphabet alphabet
		AutType::AlphabetType directAlph(new AutType::DirectAlphabet);
		aut.SetAlphabet(directAlph);

		// do the translation
		SymbolTranslPlus42 symbolTransl;
		AutType translAut = aut.TranslateSymbols(symbolTransl);

		// get the AutDescription of the translated automaton
		AutDescription descTransl = translAut.DumpToAutDesc(stateDict);

		AutDescription descManual;
		for (const Transition& trans : aut)
		{
			descManual.states.insert(stateDict.TranslateBwd(trans.GetParent()));

			AutDescription::StateTuple tuple;
			for (const StateType& state : trans.GetChildren())
			{
				descManual.states.insert(stateDict.TranslateBwd(state));
				tuple.push_back(stateDict.TranslateBwd(state));
			}

			descManual.symbols.insert(AutDescription::Symbol(
				Convert::ToString(42 + trans.GetSymbol()),
				trans.GetChildren().size()));

			descManual.transitions.insert(AutDescription::Transition(
				tuple,
				Convert::ToString(42 + trans.GetSymbol()),
				stateDict.TranslateBwd(trans.GetParent())));
		}

		for (const StateType& finState : aut.GetFinalStates())
		{
			descManual.finalStates.insert(stateDict.TranslateBwd(finState));
			descManual.states.insert(stateDict.TranslateBwd(finState));
		}

		BOOST_CHECK_MESSAGE(descManual == descTransl,
			std::string("\n\nInvalid output.") + "\n\nExpected:\n===========\n" +
			TimbukSerializer().Serialize(descManual) + "\nGot:\n===========\n" +
			TimbukSerializer().Serialize(descTransl) + "\n===========");
	}
}

BOOST_AUTO_TEST_SUITE_END()
