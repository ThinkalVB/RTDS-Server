#include "log.h"
#include <iostream>

std::atomic_bool Log::_canLog;
std::mutex Log::_writeLock;
std::ofstream Log::_logFile;

void Log::startLog()
{
	std::lock_guard<std::mutex> lock(_writeLock);
	try {
		_logFile.open("log.txt", std::ios::out);
	}
	catch (...)
	{
		std::cerr << "Logging failed";
		_canLog = false;
	}
	_canLog = true;
}

void Log::stopLog()
{
	if (_canLog)
	{
		_canLog = false;
		_logFile.close();
	}
}

bool Log::isLogging()
{
	return _canLog;
}