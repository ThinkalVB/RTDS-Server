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

void Log::_printSocketInfo(const asio::ip::tcp::socket* socketPtr)
{
	const auto remoteEp = socketPtr->remote_endpoint();
	const auto IPaddress = remoteEp.address();
	const auto portNumber = remoteEp.port();
	std::cout << IPaddress.to_string() << " " << portNumber;
}