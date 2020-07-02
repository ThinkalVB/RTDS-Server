#include "log.h"
#include <chrono>
#include <ctime>
#include <iomanip>

std::atomic_bool Log::mCanLog;
std::mutex Log::mWriteLock;
std::ofstream Log::mLogFile;

void Log::startLog()
{
	std::lock_guard<std::mutex> lock(mWriteLock);
	try {
		mLogFile.open("log.txt", std::ios::out);

		const auto currTime = std::time(NULL);
		auto strTime = std::put_time(std::localtime(&currTime), "%F %T");
		mLogFile << "Log started at : " << strTime << std::endl;
	}
	catch (...)
	{
		std::cerr << "Logging failed";
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