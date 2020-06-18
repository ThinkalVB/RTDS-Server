#ifndef RTDS_SETTINGS_H
#define RTDS_SETTINGS_H

#define REGISTER_WARNING Error::m_warnings++;
#define REGISTER_MEMMORY_ERR Error::m_error_memmory++;
#define REGISTER_SOCKET_ERR Error::m_error_socket++;
#define REGISTER_IO_ERR Error::m_error_io++;
#define REGISTER_CODE_ERROR Error::m_error_code++;

#define WARNINGS Error::m_warnings
#define MEMMORY_ERR Error::m_error_memmory
#define SOCKET_ERR Error::m_error_socket
#define IO_ERR Error::m_error_io
#define CODE_ERR Error::m_error_code

#define RTDS_PORT Settings::m_rtdsPortNo
#define RTDS_START_THREAD Settings::m_rtdsThreadCount

#include <string>
struct Error
{
	static int m_warnings;
	static int m_error_memmory;
	static int m_error_socket;
	static int m_error_io;
	static int m_error_code;
};

class Settings
{
/*******************************************************************************************
* @brief Find port number.
*
* @details
* std::err will display the error in argument and exit if the arguments are incorrect.
********************************************************************************************/
	static void m_findPortNumber(std::string portNStr);
/*******************************************************************************************
* @brief Find Thread count.
*
* @details
* std::err will display the error in argument and exit if the arguments are incorrect.
********************************************************************************************/
	static void m_findThreadCount(std::string threadCStr);
public:
	static unsigned short m_rtdsPortNo;
	static short m_rtdsThreadCount;
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
