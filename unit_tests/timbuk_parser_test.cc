/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
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
#include "aut_db.hh"


/******************************************************************************
 *                                  Constants                                 *
 ******************************************************************************/

const fs::path BASE_DIR = "../..";
const fs::path AUT_DIR = BASE_DIR / "automata";
const fs::path FAIL_AUT_DIR = AUT_DIR / "fail_timbuk";


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

	for (auto autTest : TIMBUK_AUTOMATA)
	{
		TimbukParser::AutDescription desc = parser.ParseString(autTest);
		std::string dumpedStr = serializer.Serialize(desc);
		TimbukParser::AutDescription secondTimeParsed = parser.ParseString(dumpedStr);

		BOOST_CHECK_MESSAGE(desc == secondTimeParsed, "Error while checking \n" +
			std::string(autTest));
	}
}

BOOST_AUTO_TEST_CASE(incorrect_format)
{
	if (!fs::exists(FAIL_AUT_DIR) || !fs::is_directory(FAIL_AUT_DIR))
	{
		BOOST_FAIL("Cannot find the " + FAIL_AUT_DIR.string() + " directory");
	}

	TimbukParser parser;

	for (auto dirEntryIt = fs::directory_iterator(FAIL_AUT_DIR);
		dirEntryIt != fs::directory_iterator(); ++dirEntryIt)
	{	// for each entry in the directory
		if (fs::is_regular_file(*dirEntryIt))
		{	// if it is a file
			std::string autStr = VATA::Util::ReadFile(dirEntryIt->path().string());

			BOOST_CHECK_THROW(parser.ParseString(autStr), std::exception);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()

