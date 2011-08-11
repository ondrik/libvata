/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file of a fixture class that enables logging shared among test
 *    suites.
 *
 *****************************************************************************/

#ifndef _VATA_LOG_FIXTURE_HH_
#define _VATA_LOG_FIXTURE_HH_


// Log4cpp headers
#include <log4cpp/Category.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/BasicLayout.hh>


/**
 * @brief   Test fixture that enables logging
 *
 * This test fixture enables Log4cpp logging for proper categories.
 */
class LogFixture
{
private:  // Private data members

	/**
	 * @brief  Static initialization flag
	 *
	 * Static flag that is initialized when logging is enabled (only once for
	 * the whole program.
	 */
	static bool logInitialized_;


public:   // Public methods

	/**
	 * @brief  Constructor
	 *
	 * Fixture constructor that initializes the logging feature (only once for
	 * all fixtures).
	 */
	LogFixture();

	/**
	 * @brief  Destructor
	 *
	 * Virtual destructor
	 */
	virtual ~LogFixture()
	{
	}

};

#endif
