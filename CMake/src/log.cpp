#include "log.h"
#include "rtds_settings.h"

std::atomic_bool Log::m_canLog;
std::mutex Log::m_writeLock;
std::ofstream Log::m_logFile;

void Log::startLog()
{
	std::lock_guard<std::mutex> lock(m_writeLock);
	try {
		m_logFile.open("log.txt", std::ios::out);
	}
	catch (...)
	{
		std::cerr << "Logging failed";
		REGISTER_IO_ERR
		m_canLog = false;
	}
	m_canLog = true;
}

void Log::stopLog()
{
	if (m_canLog)
	{
		m_canLog = false;
		m_logFile.close();
	}
}

bool Log::isLogging()
{
	return m_canLog;
}