#ifndef LOG_H
#define LOG_H

#include <boost/asio.hpp>
#include <fstream>
#include <mutex>
#include <string.h>
/* #define goes here since all of the cpp file contains Log.h */
#define DEBUG
/* #define RTDS_DUAL_STACK */

#ifdef DEBUG
#define PRINT_LOG
#define RTDS_CLI_MODE
#else
#define PRINT_ERROR
#endif

using namespace boost;

class Log
{
    static std::ofstream logFile;               //!< File stream
    static bool goodToLog;                      //!< Mutex for thread safety
    static std::mutex writeLock;                //!< True if the file is open for writing

/*******************************************************************************************
* @brief Print the system time 
********************************************************************************************/
    static void _printTime();

public:
/*******************************************************************************************
* @brief Start the logging process
*
* @param[in] fileName           Name of the log file
*
* @details
* Open the file in writting mode clearing the previous contents
* Name of the logging file can be changed using this function [call once only]
********************************************************************************************/
    static void startLog(std::string);
/*******************************************************************************************
* @brief Print OS runtime_error code details
*
* @param[in] message            Message from RTDS
* @param[in] ec                 Runtime error
********************************************************************************************/
    static void log(std::string, const std::runtime_error&);
/*******************************************************************************************
* @brief Print boost system error code details
*
* @param[in] message            Message from RTDS
* @param[in] ec                 Runtime error
********************************************************************************************/
    static void log(std::string, const system::error_code&);
/*******************************************************************************************
* @brief Print details about the socket error
*
* @param[in] message            Message from RTDS
* @param[in] socketPtr          Pointer to the socket
********************************************************************************************/
    static void log(std::string, const asio::ip::tcp::socket*);
/*******************************************************************************************
* @brief Print details about the socket error
*
* @param[in] message            Message from RTDS
* @param[in] socketPtr          Pointer to the socket
* @param[in] ec                 Runtime error
********************************************************************************************/
    static void log(std::string, const asio::ip::tcp::socket*, const std::error_code& ec);
/*******************************************************************************************
* @brief Print the message
*
* @param[in] message            Message from RTDS
********************************************************************************************/
    static void log(std::string);
/*******************************************************************************************
* @brief Stpo logging
********************************************************************************************/
    static void stopLog();
};

#endif