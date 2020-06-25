#include "log.h"
#include "rtds_settings.h"

std::atomic_bool Log::mCanLog;
std::mutex Log::mWriteLock;
std::ofstream Log::mLogFile;

void Log::startLog()
{
	std::lock_guard<std::mutex> lock(mWriteLock);
	try {
		mLogFile.open("log.txt", std::ios::out);
	}
	catch (...)
	{
		std::cerr << "Logging failed";
		REGISTER_IO_ERR
		mCanLog = false;
	}
	mCanLog = true;
}

void Log::stopLog()
{
	if (mCanLog)
	{
		mCanLog = false;
		mLogFile.close();
	}
}

bool Log::isLogging()
{
	return mCanLog;
}