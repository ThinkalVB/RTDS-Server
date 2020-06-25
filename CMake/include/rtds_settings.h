#ifndef RTDS_SETTINGS_H
#define RTDS_SETTINGS_H

#define REGISTER_WARNING Error::mWarnings++;
#define REGISTER_MEMMORY_ERR Error::mError_memmory++;
#define REGISTER_SOCKET_ERR Error::mError_socket++;
#define REGISTER_IO_ERR Error::mError_io++;
#define REGISTER_CODE_ERROR Error::mError_code++;

#define WARNINGS Error::mWarnings
#define MEMMORY_ERR Error::mError_memmory
#define SOCKET_ERR Error::mError_socket
#define IO_ERR Error::mError_io
#define CODE_ERR Error::mError_code

#define RTDS_PORT Settings::mRtdsPortNo
#define RTDS_START_THREAD Settings::mRtdsThreadCount
#define RESET_ERROR_COUNTER Error::mResetErrorCounts();

#include <string>
struct Error
{
	static int mWarnings;						// Number of Warnings
	static int mError_memmory;					// Number of Memmory Errors
	static int mError_socket;					// Number of Socket Errors
	static int mError_io;						// Number of IO Error
	static int mError_code;						// Number of Code Error

	static void mResetErrorCounts();
};

class Settings
{
/*******************************************************************************************
* @brief Find port number.
*
* @param[in]		Port number as string
*
* @details
* std::err will display the error in argument and exit if the arguments are incorrect.
********************************************************************************************/
	static void mFindPortNumber(std::string portNStr);
/*******************************************************************************************
* @brief Find Thread count.
*
* @param[in]		Thread cound as string
*
* @details
* std::err will display the error in argument and exit if the arguments are incorrect.
********************************************************************************************/
	static void mFindThreadCount(std::string threadCStr);
public:
	static unsigned short mRtdsPortNo;			// RTDS port number
	static short mRtdsThreadCount;				// Number of RTDS threads
/*******************************************************************************************
* @brief Process Arguments string
*
* @param[in]		Argument string
*
* @details
* std::err will display the error in argument and exit if the arguments are incorrect.
********************************************************************************************/
	static void processArgument(std::string);
};
#endif
