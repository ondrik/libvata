/*****************************************************************************
 *  VATA Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Header file for a class faking a FILE* structure
 *
 *****************************************************************************/

#ifndef _VATA_FAKE_FILE_HH_
#define _VATA_FAKE_FILE_HH_

// Standard library headers
#include <cstdio>
#include <string>


// insert the class into proper namespace
namespace VATA
{
	namespace Util
	{
		class FakeFile;
	}
}


/**
 * @brief  Class that mocks a file and saves input to string
 *
 * This class provides a @c FILE* pointer that mocks an open file but saves
 * the input into a string that can be later read out.
 */
class VATA::Util::FakeFile
{
private:  // Private data members

	/**
	 * @brief  Buffer
	 *
	 * Pointer to the buffer that stores input data
	 */
	char* ptrBuffer_;

	/**
	 * @brief  Size of buffer
	 *
	 * The size of buffer for input data
	 */
	size_t bufferSize_;

	/**
	 * @brief  @c FILE* pointer for the mock file
	 *
	 * @c FILE* pointer for the mock file
	 */
	FILE* ptrFile_;

	/**
	 * @brief  Denotes whether the mock file has been opened
	 *
	 * A Boolean value that denotes whether the mock file represented by the
	 * object has been opened.
	 */
	bool hasBeenOpened_;

	/**
	 * @brief  Denotes whether the mock file has been closed
	 *
	 * A Boolean value that denotes whether the mock file represented by the
	 * object has been closed (after being open).
	 */
	bool isClosed_;

	/**
	 * @brief  Denotes whether the mock file has been opened in write mode
	 *
	 * A Boolean value that denotes whether the mock file represented by the
	 * object has been opened in write mode.
	 */
	bool writeMode_;


private:  // Private methods


	/**
	 * @brief  Copy constructor
	 *
	 * Private copy constructor.
	 *
	 * @param[in]  ff  The object to be copied
	 */
	FakeFile(const FakeFile& ff);

	/**
	 * @brief  Assignment operator
	 *
	 * Private assignment operator.
	 *
	 * @param[in]  ff  The object to be assigned
	 *
	 * @returns  Assigned object
	 */
	FakeFile& operator=(const FakeFile& ff);


public:   // Public methods


	/**
	 * @brief  Constructor
	 *
	 * The constructor of the class
	 */
	FakeFile();


	/**
	 * @brief  The method that opens the mock stream for writing
	 *
	 * Opens the mock stream and returns the @c FILE* pointer so that it can be
	 * handled using a UNIX file pointer in the common way. The stream is opened
	 * only for writing.
	 *
	 * @returns  UNIX @c FILE* pointer
	 */
	FILE* OpenWrite();


	/**
	 * @brief  The method that opens the mock stream for reading
	 *
	 * Opens the mock stream and returns the @c FILE* pointer so that it can be
	 * handled using a UNIX file pointer in the common way. The stream is opened
	 * only for reading.
	 *
	 * @param[in]  str  The string to be inserted into the stream
	 *
	 * @returns  UNIX @c FILE* pointer
	 */
	FILE* OpenRead(std::string str);


	/**
	 * @brief  Closes the stream
	 *
	 * This method closes a previously opened mock file. Note that the file
	 * needs to have been opened and haven't been closed yet.
	 */
	void Close();


	/**
	 * @brief  Gets the data written to the file
	 *
	 * Retuns a string that contains the data that has been written to the mock
	 * file.
	 *
	 * @returns  Data written to the mock file
	 */
	std::string GetContent() const;


	/**
	 * @brief  Destructor
	 *
	 * The destuctor
	 */
	~FakeFile();
};

#endif
