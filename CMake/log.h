#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <asio.hpp>
#include <iostream>

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
    template<typename T>
    static void log(T, const std::runtime_error);
/*******************************************************************************************
* @brief Print the message
*
* @param[in]        Messages (Variadic template function)
********************************************************************************************/
    template<typename T, typename... Args>
    static void log(T, Args...);
/*******************************************************************************************
* @brief Print details about the socket error
*
* @param[in]        Message
* @param[in]        Pointer to the socket
* @param[in]        Asio socket error
********************************************************************************************/
    template<typename T>
    static void log(T, const asio::ip::tcp::socket*, const asio::error_code);
/*******************************************************************************************
* @brief Print asio error code details
*
* @param[in]        Message from RTDS
* @param[in]        Runtime error
********************************************************************************************/
    template<typename T>
    static void log(T, const asio::error_code);
/*******************************************************************************************
* @brief Print details about the socket
*
* @param[in]        Message
* @param[in]        Pointer to the socket
********************************************************************************************/
    template<typename T>
    static void log(T, const asio::ip::tcp::socket*);

private:
    static std::atomic_bool _needLog;           // True if needs logs
    static std::mutex _consoleWriteLock;        // Mutex for thread safety

/*******************************************************************************************
* @brief Print details about the socket (Ipaddress and Port number)
*
* @param[in]        Pointer to the socket
* [Not Thread Safe]
********************************************************************************************/
    static void _printSocketInfo(const asio::ip::tcp::socket*);
};

template<typename T>
inline void Log::log(T message, const std::runtime_error ec)
{
    std::lock_guard<std::mutex> lock(_consoleWriteLock);
    if (_needLog)
    {
        std::cout << "RTDS : ";
        std::cout << message << " " << ec.what() << std::endl;
    }
}

template<typename T, typename... Args>
inline void Log::log(T message, Args... messages)
{
    std::lock_guard<std::mutex> lock(_consoleWriteLock);
    if (_needLog)
    {
        std::cout << "RTDS : ";
        std::cout << message;
        (std::cout << ... << std::forward<Args>(messages));
        std::cout << std::endl;
    }
}

template<typename T>
inline void Log::log(T message, const asio::ip::tcp::socket* socketPtr, const asio::error_code ec)
{
    std::lock_guard<std::mutex> lock(_consoleWriteLock);
    if (_needLog)
    {
        std::cout << "RTDS : ";
        std::cout << message << " ";
        _printSocketInfo(socketPtr);
        std::cout << " " << ec.message() << std::endl;
    }
}

template<typename T>
inline void Log::log(T message, const asio::error_code ec)
{
    std::lock_guard<std::mutex> lock(_consoleWriteLock);
    if (_needLog)
    {
        std::cout << "RTDS : ";
        std::cout << message << " " << ec.message() << std::endl;
    }
}

template<typename T>
inline void Log::log(T message, const asio::ip::tcp::socket* socketPtr)
{
    std::lock_guard<std::mutex> lock(_consoleWriteLock);
    if (_needLog)
    {
        std::cout << "RTDS : ";
        std::cout << message << " ";
        _printSocketInfo(socketPtr);
        std::cout << std::endl;
    }
}

#endif