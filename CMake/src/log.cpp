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