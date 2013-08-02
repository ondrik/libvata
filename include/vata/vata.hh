/*****************************************************************************
 *  VATA Tree Automata Library
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

// Standard library headers
#include <cassert>
#include <iostream>

//#define NDEBUG

#ifdef NDEBUG
	#define DEBUG 0
#endif

#define VATA_LOG_PREFIX (std::string(__FILE__ ":" + VATA::Util::Convert::ToString(__LINE__) + ": "))

#define VATA_LOG_MESSAGE(severity) (std::clog << #severity << ": " << (VATA_LOG_PREFIX))

#define VATA_LOG_DEBUG    (VATA_LOG_MESSAGE(debug))
#define VATA_LOG_INFO     (VATA_LOG_MESSAGE(info))
#define VATA_LOG_NOTICE   (VATA_LOG_MESSAGE(notice))
#define VATA_LOG_WARN     (VATA_LOG_MESSAGE(warning))
#define VATA_LOG_ERROR    (VATA_LOG_MESSAGE(error))
#define VATA_LOG_CRIT     (VATA_LOG_MESSAGE(critical))
#define VATA_LOG_ALERT    (VATA_LOG_MESSAGE(alert))
#define VATA_LOG_FATAL    (VATA_LOG_MESSAGE(fatal))

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
