/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of a fixture class that enables logging shared among test
 *    suites.
 *
 *****************************************************************************/

// Boost headers
#include <boost/test/unit_test.hpp>

// testing header files
#include "log_fixture.hh"

// initialization of the static variable
bool LogFixture::logInitialized_ = false;


LogFixture::LogFixture()
{
	boost::unit_test::unit_test_log.set_threshold_level(boost::unit_test::log_messages);

	if (!logInitialized_)
	{	// if the logging has not been initialized yet
		logInitialized_ = true;

		// Create the appender
		log4cpp::Appender* app1  = new log4cpp::OstreamAppender("ClogAppender", &std::clog);

		std::string cat_name = "VATA";

		log4cpp::Category::getInstance(cat_name).setAdditivity(false);
		log4cpp::Category::getInstance(cat_name).addAppender(app1);
		log4cpp::Category::getInstance(cat_name).setPriority(log4cpp::Priority::INFO);
	}
}
