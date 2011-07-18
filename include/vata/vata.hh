/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file with global declarations. It contains:
 *      * macros for easy logging
 *      * macro for suppressing certain GCC warnings
 *
 *****************************************************************************/

#ifndef _VATA_VATA_HH_
#define _VATA_VATA_HH_

// Log4cpp headers
#include <log4cpp/Category.hh>

//#define DEBUG 0
//#define NDEBUG

#define VATA_LOG_CATEGORY_NAME "VATA"

#define VATA_LOGGER_PREFIX (std::string(__FILE__ ":" + VATA::Convert::ToString(__LINE__) + ": "))

#define VATA_LOGGER_LOG_MESSAGE(severity, msg) (log4cpp::Category::getInstance(VATA_LOG_CATEGORY_NAME).severity((VATA_LOGGER_PREFIX) + (msg)))

#define VATA_LOGGER_DEBUG(msg)  (VATA_LOGGER_LOG_MESSAGE(debug, msg))
#define VATA_LOGGER_INFO(msg)   (VATA_LOGGER_LOG_MESSAGE(info, msg))
#define VATA_LOGGER_NOTICE(msg) (VATA_LOGGER_LOG_MESSAGE(notice, msg))
#define VATA_LOGGER_WARN(msg)   (VATA_LOGGER_LOG_MESSAGE(warn, msg))
#define VATA_LOGGER_ERROR(msg)  (VATA_LOGGER_LOG_MESSAGE(error, msg))
#define VATA_LOGGER_CRIT(msg)   (VATA_LOGGER_LOG_MESSAGE(crit, msg))
#define VATA_LOGGER_ALERT(msg)  (VATA_LOGGER_LOG_MESSAGE(alert, msg))
#define VATA_LOGGER_FATAL(msg)  (VATA_LOGGER_LOG_MESSAGE(fatal, msg))

#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x,y) GCC_DIAG_STR(x ## y)
# define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
# define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
# if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#  define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
	         GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
#  define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
# else
#  define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W,x))
#  define GCC_DIAG_ON(x)  GCC_DIAG_PRAGMA(warning GCC_DIAG_JOINSTR(-W,x))
# endif
#else
# define GCC_DIAG_OFF(x)
# define GCC_DIAG_ON(x)
#endif

#endif
