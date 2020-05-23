#include "log.h"
#include <boost/date_time.hpp>

#ifdef RTDS_CLI_MODE
#define RTDS_CLI(x) x
#else
#define RTDS_CLI(x)
#endif

#ifdef RTDS_CLI_MODE
#include <iostream>
#endif
using namespace boost;

std::ofstream Log::_logFile;
std::mutex Log::_writeLock;
bool Log::_goodToLog = false;

void Log::_printTime()
{
	auto localTime = posix_time::microsec_clock::local_time();
	_logFile << "[" << posix_time::to_simple_string(localTime) << "]  ";
}

void Log::startLog(std::string fileName)
{
	std::lock_guard<std::mutex> lock(_writeLock);
	if (!_goodToLog)
	{
		try {
			_logFile.open(fileName, std::ios::out);
		}
		catch (...)
		{
			RTDS_CLI(std::cout << "Logging failed";)
		}
		_goodToLog = true;
	}
	else
		return;
}

void Log::log(std::string message, const std::runtime_error& ec)
{
	std::lock_guard<std::mutex> lock(_writeLock);
	if (_goodToLog)
	{
		_printTime();
		try {
			_logFile << message << " " << ec.what() << std::endl;
		}catch (...)
		{
			RTDS_CLI(std::cout << "Logging failed";)
		}
	}
}

void Log::log(std::string message, const system::error_code& ec)
{
	std::lock_guard<std::mutex> lock(_writeLock);
	if (_goodToLog)
	{
		try {
			_printTime();
			_logFile << message << " " << ec.value() << " " << ec.message() << std::endl;
		}catch (...)
		{
			RTDS_CLI(std::cout << "Logging failed";)
		}
	}
}

void Log::log(std::string message, const asio::ip::tcp::socket* socketPtr)
{
	std::lock_guard<std::mutex> lock(_writeLock);
	if (_goodToLog)
	{
		try {
			auto remoteEp = socketPtr->remote_endpoint();
			auto IPaddress = remoteEp.address();
			auto portNumber = remoteEp.port();
			_printTime();
			_logFile << message << " " << IPaddress.to_string() << " " << portNumber << std::endl;
		}
		catch (...)
		{
			RTDS_CLI(std::cout << "Logging failed";)
		}
	}
}

void Log::log(std::string message, const asio::ip::tcp::socket* socketPtr, const std::error_code& ec)
{
	std::lock_guard<std::mutex> lock(_writeLock);
	if (_goodToLog)
	{
		try {
			auto remoteEp = socketPtr->remote_endpoint();
			auto IPaddress = remoteEp.address();
			auto portNumber = remoteEp.port();
			_printTime();
			_logFile << message << " " << IPaddress.to_string() << " " << portNumber << " ";
			_logFile << ec.message() << std::endl;
		}
		catch (...)
		{
			RTDS_CLI(std::cout << "Logging failed";)
		}
	}
}

void Log::log(std::string message)
{
	std::lock_guard<std::mutex> lock(_writeLock);
	if (_goodToLog)
	{
		try {
			_printTime();
			_logFile << message << std::endl;
		}catch (...)
		{
			RTDS_CLI(std::cout << "Logging failed";)
		}
	}
}

void Log::stopLog()
{
	std::lock_guard<std::mutex> lock(_writeLock);
	_goodToLog = false;
	_logFile.close();
}