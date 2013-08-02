/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2013  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for the "unimplemented feature" exception class.
 *
 *****************************************************************************/

#ifndef _VATA_NOTIMPL_EXCEPT_H_
#define _VATA_NOTIMPL_EXCEPT_H_

#include <string>
#include <stdexcept>

/**
 * @file notimpl_except.hh
 * NotImplementedException class declaration (and definition)
 */

namespace VATA
{
	class NotImplementedException;
}

/**
 * @brief  An exception class for unimplemented features
 *
 * This exception class is used for reporting unimplemented features.
 */
class VATA::NotImplementedException : public std::exception
{
private:  // data members

	/// Error message
	std::string msg_;

	/// Error message prefix
	static constexpr const char* STRING_PREFIX = "Not implemented: ";

public:

	/**
	 * @brief  Constructor
	 *
	 * Constructs and assigns value to a new object.
	 *
	 * @param[in]  msg  The error message
	 */
	NotImplementedException(
		const std::string&                msg) :
		msg_(STRING_PREFIX + msg)
	{ }

	/**
	 * @brief  Copy constructor
	 */
	NotImplementedException(const NotImplementedException& ex) :
		msg_(ex.msg_)
	{ }

	/**
	 * @brief  Assignment operator
	 */
	NotImplementedException& operator=(const NotImplementedException& ex)
	{
		if (this != &ex)
		{
			msg_ = ex.msg_;
		}

		return *this;
	}

	/**
	 * @brief  Destructor
	 *
	 * Virtual destructor.
	 */
	virtual ~NotImplementedException() throw()
	{ }

	/**
	 * @brief  Description of error
	 *
	 * Retrieves the description of the error (overrides the
	 * std::exception::what() method)
	 *
	 * @returns  The description of the error
	 */
	virtual const char* what() const throw()
	{
		return msg_.c_str();
	}
};

#endif
