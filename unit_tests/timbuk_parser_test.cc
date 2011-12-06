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


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path LOAD_TIMBUK_FILE =
	AUT_DIR / "load_timbuk.txt";


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

BOOST_AUTO_TEST_CASE(correct_format)
{
	TimbukParser parser;
	TimbukSerializer serializer;

	auto testfileContent = ParseTestFile(LOAD_TIMBUK_FILE.string());
	for (auto testcase : testfileContent)
	{
		std::string filename = (AUT_DIR / testcase[0]).string();
		BOOST_MESSAGE("Parsing automaton " + filename + "...");
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
			BOOST_MESSAGE("Parsing automaton " + filename + "...");
			std::string autStr = VATA::Util::ReadFile(filename);

			BOOST_CHECK_THROW(parser.ParseString(autStr), std::exception);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()

