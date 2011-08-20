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

const std::string TIMBUK_SUFFIX = "_timbuk";

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
	if (!fs::exists(AUT_DIR) || !fs::is_directory(AUT_DIR))
	{
		BOOST_FAIL("Cannot find the " + AUT_DIR.string() + " directory");
	}

	TimbukParser parser;
	TimbukSerializer serializer;

	for (auto topDirEntryIt = fs::directory_iterator(AUT_DIR);
		topDirEntryIt != fs::directory_iterator(); ++topDirEntryIt)
	{	// for each entry in the directory
		auto topDirEntry = *topDirEntryIt;
		if (fs::is_directory(topDirEntry))
		{	// if it is a directory
			std::string dirPath = topDirEntry.path().string();

			if (dirPath.rfind(TIMBUK_SUFFIX) !=
				(dirPath.size() - TIMBUK_SUFFIX.length()))
			{	// in case there are not timbuk automata in the directory
				continue;
			}

			if (dirPath == FAIL_AUT_DIR)
			{	// in case it is the bad directory
				continue;
			}

			BOOST_MESSAGE("Parsing automata in " + dirPath);

			for (auto dirIt = fs::directory_iterator(dirPath);
				dirIt != fs::directory_iterator(); ++dirIt)
			{	// for each entry in the lower directory
				if (fs::is_regular_file(*dirIt))
				{	// if it is a file
					std::string autStr = VATA::Util::ReadFile(dirIt->path().string());

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
							+ dirIt->path().string() + "\": " + ex.what());
					}
				}
			}
		}
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

