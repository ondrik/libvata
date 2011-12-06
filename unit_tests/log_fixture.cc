/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of a fixture class that enables logging shared among test
 *    suites.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/util/util.hh>

// Boost headers
#include <boost/test/unit_test.hpp>

// testing header files
#include "log_fixture.hh"

// Boost headers
#include <boost/algorithm/string.hpp>


// initialization of the static variable
bool LogFixture::logInitialized_ = false;


LogFixture::LogFixture()
{
	boost::unit_test::unit_test_log.set_threshold_level(boost::unit_test::log_messages);
}


std::vector<std::string> LogFixture::GetTimbukAutFilenames() const
{
	if (!fs::exists(AUT_DIR) || !fs::is_directory(AUT_DIR))
	{
		BOOST_FAIL("Cannot find the " + AUT_DIR.string() + " directory");
	}

	std::vector<std::string> result;

	for (auto topDirEntryIt = fs::directory_iterator(AUT_DIR);
		topDirEntryIt != fs::directory_iterator(); ++topDirEntryIt)
	{	// for each entry in the directory
		auto topDirEntry = *topDirEntryIt;
		if (fs::is_directory(topDirEntry))
		{	// if it is a directory
			std::string dirPath = topDirEntry.path().string();

			if (!boost::algorithm::ends_with(dirPath, TIMBUK_SUFFIX))
			{	// in case there are not timbuk automata in the directory
				continue;
			}

			if (dirPath == FAIL_TIMBUK_AUT_DIR)
			{	// in case it is the bad directory
				continue;
			}

			for (auto dirIt = fs::directory_iterator(dirPath);
				dirIt != fs::directory_iterator(); ++dirIt)
			{	// for each entry in the lower directory
				if (fs::is_regular_file(*dirIt))
				{	// if it is a file
					if (dirIt->path().filename().string()[0] != '.')
					{	// in case the file is not hidden
						result.push_back(dirIt->path().string());
					}
				}
			}
		}
	}

	return result;
}


LogFixture::ConfFileContentType LogFixture::ParseTestFile(
	const std::string& filename) const
{
	if (!fs::exists(filename) || !fs::is_regular_file(filename))
	{
		BOOST_FAIL("Cannot find the " + filename + " file");
	}

	std::string confStr = VATA::Util::ReadFile(filename);

	std::vector<std::string> splitConfStr;
	boost::algorithm::split(splitConfStr, confStr,
		boost::algorithm::is_any_of("\n"), boost::algorithm::token_compress_on);

	ConfFileContentType result;
	for (auto confLine : splitConfStr)
	{
		std::vector<std::string> splitConfLine;
		boost::algorithm::split(splitConfLine, confLine,
			boost::algorithm::is_space(), boost::algorithm::token_compress_on);

		if (splitConfLine.empty() || splitConfLine[0].empty())
		{	// in case there is nothing
			continue;
		}
		else
		{
			result.push_back(splitConfLine);
		}
	}

	return result;
}
