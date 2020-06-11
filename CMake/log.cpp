#include "log.h"

std::atomic_bool Log::_needLog;
std::mutex Log::_consoleWriteLock;

void Log::startLog()
{
	_needLog = true;
}

void Log::stopLog()
{
	_needLog = false;
}

bool Log::isLogging()
{
	return _needLog;
}

void Log::log(const std::string message, const std::runtime_error ec)
{
	std::lock_guard<std::mutex> lock(_consoleWriteLock);
	if (_needLog)
	{
		std::cout << "RTDS: ";
		std::cout << message << " " << ec.what() << std::endl;
	}
}

void Log::log(const std::string message, const asio::ip::tcp::socket* socketPtr, const std::error_code ec)
{
	std::lock_guard<std::mutex> lock(_consoleWriteLock);
	if (_needLog)
	{
		std::cout << "RTDS: ";
		std::cout << message << " ";
		_printSocketInfo(socketPtr);
		std::cout << " " << ec.message() << std::endl;
	}
}

void Log::log(std::string message, const asio::error_code ec)
{
	std::lock_guard<std::mutex> lock(_consoleWriteLock);
	if (_needLog)
	{
		std::cout << "RTDS: ";
		std::cout << message << " " << ec.message() << std::endl;
	}
}

void Log::log(const std::string message, const asio::ip::tcp::socket* socketPtr)
{
	std::lock_guard<std::mutex> lock(_consoleWriteLock);
	if (_needLog)
	{
		std::cout << "RTDS: ";
		std::cout << message << " ";
		_printSocketInfo(socketPtr);
		std::cout << std::endl;
	}
}

void Log::_printSocketInfo(const asio::ip::tcp::socket* socketPtr)
{
	const auto remoteEp = socketPtr->remote_endpoint();
	const auto IPaddress = remoteEp.address();
	const auto portNumber = remoteEp.port();
	std::cout << IPaddress.to_string() << " " << portNumber;
}