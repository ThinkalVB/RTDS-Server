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

std::ofstream Log::logFile;
std::mutex Log::writeLock;
bool Log::goodToLog = false;

void Log::_printTime()
{
	auto localTime = posix_time::second_clock::local_time();
	logFile << "[" << localTime.time_of_day().hours();
	logFile << ":" << localTime.time_of_day().minutes() << ":";
	logFile << ":" << localTime.time_of_day().fractional_seconds() << ":";
	logFile << localTime.time_of_day().seconds() << "\t]";
}

void Log::startLog(std::string fileName)
{
	std::lock_guard<std::mutex> lock(writeLock);
	try {
		logFile.open(fileName, std::ios::out);
	}catch(...)
	{
		RTDS_CLI(std::cout << "Logging failed";)
	}
	goodToLog = true;
}

void Log::log(std::string message, const std::runtime_error& ec)
{
	std::lock_guard<std::mutex> lock(writeLock);
	if (goodToLog)
	{
		try {
			_printTime();
			logFile << message << " " << ec.what() << std::endl;
		}catch (...)
		{
			RTDS_CLI(std::cout << "Logging failed";)
		}
	}
}

void Log::log(std::string message, const system::error_code& ec)
{
	std::lock_guard<std::mutex> lock(writeLock);
	if (goodToLog)
	{
		try {
			_printTime();
			logFile << message << " " << ec.value() << " " << ec.message() << std::endl;
		}catch (...)
		{
			RTDS_CLI(std::cout << "Logging failed";)
		}
	}
}

void Log::log(std::string message, const asio::ip::tcp::socket* peerSocket)
{
	std::lock_guard<std::mutex> lock(writeLock);
	if (goodToLog)
	{
		try {
			auto remoteEp = peerSocket->remote_endpoint();
			auto IPaddress = remoteEp.address();
			auto portNumber = remoteEp.port();
			_printTime();
			logFile << message << " " << IPaddress.to_string() << " " << portNumber << std::endl;
		}catch (...)
		{
			RTDS_CLI(std::cout << "Logging failed";)
		}
	}
}

void Log::log(std::string message)
{
	std::lock_guard<std::mutex> lock(writeLock);
	if (goodToLog)
	{
		try {
			_printTime();
			logFile << message << std::endl;
		}catch (...)
		{
			RTDS_CLI(std::cout << "Logging failed";)
		}
	}
}

void Log::stopLog()
{
	std::lock_guard<std::mutex> lock(writeLock);
	goodToLog = false;
	logFile.close();
}
