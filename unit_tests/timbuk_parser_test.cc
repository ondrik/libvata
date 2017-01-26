/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Test suite for the Timbuk parser
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/convert.hh>
#include <vata/util/util.hh>

using VATA::Parsing::TimbukParser;
using VATA::Serialization::TimbukSerializer;
using VATA::Util::Convert;

// Boost headers
#define BOOST_ALL_DYN_LINK

#define BOOST_TEST_MODULE TimbukParser
#include <boost/test/unit_test.hpp>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


// testing headers
#include "log_fixture.hh"

// usings
using AutDesc = TimbukParser::AutDescription;


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path LOAD_TIMBUK_FILE =
	AUT_DIR / "load_timbuk.txt";

const std::string AUT_1 =
	"Ops f:2 g h\n"
	"Automaton AUT_1\n"
	"\n"
	"States q1 q_2 _q3 q4_ q5:0\n"
	"Final States q_2:0 q5\n"
	"Transitions        \n"
	"f(q1,q1) -> q_2\n"
	"g(q1,q_2) -> _q3\n"
	"	h -> q5\n"
	;

static AutDesc create_AUT_1()
{
	AutDesc desc;
	desc.name = "AUT_1";
	desc.symbols.insert(
		{
			std::make_pair("f", 2),
			std::make_pair("g", -1),
			std::make_pair("h", -1),
		});
	desc.states.insert({"q1", "q_2", "_q3", "q4_", "q5"});
	desc.finalStates.insert({"q_2", "q5"});
	desc.transitions.insert({
			AutDesc::Transition({"q1", "q1"}, "f", "q_2"),
			AutDesc::Transition({"q1", "q_2"}, "g", "_q3"),
			AutDesc::Transition({}, "h", "q5"),
		});

	return desc;
}

const std::string AUT_2 =
	"Automaton\n"
	"Final States q_2:0 q5\n"
	"Transitions        \n"
	;

static AutDesc create_AUT_2()
{
	AutDesc desc;
	desc.name = "";
	desc.symbols.insert({ });
	desc.states.insert({ });
	desc.finalStates.insert({"q_2", "q5"});
	desc.transitions.insert({ });

	return desc;
}

const std::string AUT_3 =
	"Final States q2\n"
	"Transitions\n"
	;

static AutDesc create_AUT_3()
{
	AutDesc desc;
	desc.name = "";
	desc.symbols.insert({ });
	desc.states.insert({ });
	desc.finalStates.insert({"q2"});
	desc.transitions.insert({ });

	return desc;
}

using TestVector = std::vector<std::tuple<std::string, std::string, AutDesc>>;

TestVector createTestVector()
{
	TestVector testVec =
	{
		std::make_tuple("AUT_1", AUT_1, create_AUT_1()),
		std::make_tuple("AUT_2", AUT_2, create_AUT_2()),
		std::make_tuple("AUT_3", AUT_3, create_AUT_3()),
	};

	return testVec;
}


const std::string BAD_AUT_1 =
	"Automaton BAD_AUT_1 kralik\n"
	"States q1 q_2 _q3 q4_ q5:0\n"
	"Final States q_2:0 q5\n"
	"Transitions\n"
	;

const std::string BAD_AUT_2 =
	"Automaton BAD_AUT_2\n"
	"States\n"
	"Final States\n"
	"Transitions\n"
	"f(q1,q1) -> q_2 q3\n"
	;

const std::string BAD_AUT_3 =
	"Automaton BAD_AUT_3\n"
	"States\n"
	"final States\n"
	"Transitions\n"
	"f(q1,q1) -> q_2 q3\n"
	;

const std::string BAD_AUT_4 =
	"Transitions\n"
	"f(q1 q1) -> q_2\n"
	;

const std::string BAD_AUT_5 =
	"Transitions\n"
	"(q1) -> q_2\n"
	;

const std::string BAD_AUT_6 =
	"Transitions\n"
	"f(q1) ->\n"
	;

const std::string BAD_AUT_7 =
	"Transitions\n"
	" -> h\n"
	;

const std::string BAD_AUT_8 =
	"Transitions\n"
	" -> \n"
	;

const std::string BAD_AUT_9 =
	"Transitions\n"
	"f(q1, q2 -> q3\n"
	;

const std::string BAD_AUT_10 =
	"Transitions\n"
	"f q1, q2) -> q3\n"
	;

const std::string BAD_AUT_11 =
	"Transitions\n"
	"f(q1, q2 q3) -> q3\n"
	;

const std::string BAD_AUT_12 =
	"Transitions\n"
	"q1 -> q2\n"
	;

using BadVector = std::vector<std::tuple<std::string, std::string>>;

BadVector createBadVector()
{
	BadVector badVec =
	{
		std::make_tuple("BAD_AUT_1", BAD_AUT_1),
		std::make_tuple("BAD_AUT_2", BAD_AUT_2),
		std::make_tuple("BAD_AUT_3", BAD_AUT_3),
		std::make_tuple("BAD_AUT_4", BAD_AUT_4),
		std::make_tuple("BAD_AUT_5", BAD_AUT_5),
		std::make_tuple("BAD_AUT_6", BAD_AUT_6),
		std::make_tuple("BAD_AUT_7", BAD_AUT_7),
	};

	return badVec;
}

/******************************************************************************
 *                                  Fixtures                                  *
 ******************************************************************************/

/**
 * @brief  TimbukParser testing fixture
 *
 * Fixture for test of TimbukParser
 */
class TimbukParserFixture : public LogFixture
{ };


/******************************************************************************
 *                              Start of testing                              *
 ******************************************************************************/


BOOST_FIXTURE_TEST_SUITE(suite, TimbukParserFixture)

BOOST_AUTO_TEST_CASE(parser_correct_format_list)
{
	BOOST_TEST_MESSAGE("========= Parser test with correct hard-coded automata =========");
	TimbukParser parser;

	TestVector testVec = createTestVector();

	std::string autName;
	std::string autStr;
	AutDesc autDesc;
	for (auto testcase : testVec)
	{
		std::tie(autName, autStr, autDesc) = testcase;
		BOOST_TEST_MESSAGE("Parsing automaton " + autName + "...");

		try
		{
			AutDesc desc = parser.ParseString(autStr);

			BOOST_CHECK_MESSAGE(desc.StrictlyEqual(autDesc),
				"Error while checking " + autName + ".\nExpected: \n" +
				autDesc.ToString() + "\nObtained: \n" +
				desc.ToString());
				// "Error while checking \n" + std::string(autStr));
		}
		catch (std::exception& ex)
		{
			BOOST_FAIL("Caught exception while parsing automaton \""
				+ autName + "\": " + ex.what());
		}
	}
}

BOOST_AUTO_TEST_CASE(parser_incorrect_format_list)
{
	BOOST_TEST_MESSAGE("========= Parser test with incorrect hard-coded automata =========");
	TimbukParser parser;

	BadVector badVec = createBadVector();

	std::string autName;
	std::string autStr;
	for (auto testcase : badVec)
	{
		std::tie(autName, autStr) = testcase;
		BOOST_TEST_MESSAGE("Parsing automaton " + autName + "...");

		BOOST_CHECK_THROW(parser.ParseString(autStr), std::exception);
	}
}


BOOST_AUTO_TEST_CASE(correct_format)
{
	BOOST_TEST_MESSAGE("========= Parser test with correct loaded automata =========");

	TimbukParser parser;
	TimbukSerializer serializer;

	auto testfileContent = ParseTestFile(LOAD_TIMBUK_FILE.string());
	for (auto testcase : testfileContent)
	{
		std::string filename = (AUT_DIR / testcase[0]).string();
		BOOST_TEST_MESSAGE("Parsing automaton " + filename + "...");
		std::string autStr = VATA::Util::ReadFile(filename);

		try
		{
			TimbukParser::AutDescription desc = parser.ParseString(autStr);

			std::string dumpedStr = serializer.Serialize(desc);
			TimbukParser::AutDescription secondTimeParsed =
				parser.ParseString(dumpedStr);

			BOOST_CHECK_MESSAGE(desc == secondTimeParsed,
				"Error while checking \n" + std::string(autStr));
		}
		catch (std::exception& ex)
		{
			BOOST_FAIL("Caught exception while parsing file \""
				+ filename + "\": " + ex.what());
		}
	}
}

BOOST_AUTO_TEST_CASE(incorrect_format)
{
	BOOST_TEST_MESSAGE("========= Parser test with incorrect loaded automata =========");

	if (!fs::exists(FAIL_TIMBUK_AUT_DIR) || !fs::is_directory(FAIL_TIMBUK_AUT_DIR))
	{
		BOOST_FAIL("Cannot find the " + FAIL_TIMBUK_AUT_DIR.string() + " directory");
	}

	TimbukParser parser;

	for (auto dirEntryIt = fs::directory_iterator(FAIL_TIMBUK_AUT_DIR);
		dirEntryIt != fs::directory_iterator(); ++dirEntryIt)
	{	// for each entry in the directory
		if (fs::is_regular_file(*dirEntryIt))
		{	// if it is a file
			std::string filename = dirEntryIt->path().string();
			BOOST_TEST_MESSAGE("Parsing automaton " + filename + "...");
			std::string autStr = VATA::Util::ReadFile(filename);

			BOOST_CHECK_THROW(parser.ParseString(autStr), std::exception);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()

