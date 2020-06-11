#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string.h>
#include <asio.hpp>
#include <atomic>

#define START_LOG Log::startLog();
#define STOP_LOG Log::stopLog();

#ifdef PRINT_DEBUG_LOG
#define DEBUG_LOG(x) x
#else
#define DEBUG_LOG(x)
#endif

#ifdef PRINT_LOG
#define LOG(x) x
#else
#define LOG(x)
#endif

class Log
{
public:
/*******************************************************************************************
* @brief Enable logging
********************************************************************************************/
    static void startLog();
/*******************************************************************************************
* @brief Disable Logging
********************************************************************************************/
    static void stopLog();
/*******************************************************************************************
* @brief Check if the logging is enbaled or disabled
*
* @return                       True if the logging is enabled
********************************************************************************************/
    static bool isLogging();
/*******************************************************************************************
* @brief Print OS runtime_error code details
*
* @param[in]         Message
* @param[in]         Runtime error
********************************************************************************************/
    static void log(const std::string, const std::runtime_error);
/*******************************************************************************************
* @brief Print the message
*
* @param[in]        Message
********************************************************************************************/
    static void log(const std::string);
/*******************************************************************************************
* @brief Print details about the socket error
*
* @param[in]        Message
* @param[in]        Pointer to the socket
* @param[in]        Asio socket error
********************************************************************************************/
    static void log(const std::string, const asio::ip::tcp::socket*, const asio::error_code);
/*******************************************************************************************
* @brief Print asio error code details
*
* @param[in]        Message from RTDS
* @param[in]        Runtime error
********************************************************************************************/
    static void log(const std::string, const asio::error_code);
/*******************************************************************************************
* @brief Print details about the socket
*
* @param[in]        Message
* @param[in]        Pointer to the socket
********************************************************************************************/
    static void log(const std::string, const asio::ip::tcp::socket*);

private:
    static std::atomic_bool _needLog;           // True if needs logs
    static std::mutex _consoleWriteLock;        // Mutex for thread safety

/*******************************************************************************************
* @brief Print the system time
* [Not Thread Safe]
********************************************************************************************/
    static void _printTimeStamp();
/*******************************************************************************************
* @brief Print details about the socket (Ipaddress and Port number)
*
* @param[in]        Pointer to the socket
* [Not Thread Safe]
********************************************************************************************/
    static void _printSocketInfo(const asio::ip::tcp::socket*);
};

#endif
