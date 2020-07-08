#include "rtds_settings.h"
#include "cmd_processor.h"
#include <iostream>

unsigned short Settings::mRTDSportNo = RDTS_DEF_PORT;
unsigned short Settings::mRTDSccmPortNo = RTDS_DEF_CCM_PORT;
short Settings::mRTDSthreadCount = MIN_THREAD_COUNT;
bool Settings::mNeedToAbort = false;

void Settings::mFindPortNumber(std::string portNStr)
{
	if (!CmdProcessor::isPortNumber(portNStr, mRTDSportNo))
	{
		std::cerr << "Invalid Port Number as argument";
		exit(0);
	}
}

void Settings::mFindccmPortNumber(std::string portNStr)
{
	if (!CmdProcessor::isPortNumber(portNStr, mRTDSccmPortNo))
	{
		std::cerr << "Invalid Port Number as argument";
		exit(0);
	}
}

void Settings::mFindThreadCount(std::string threadCStr)
{
	if (!CmdProcessor::isThreadCount(threadCStr, mRTDSthreadCount))
	{
		std::cerr << "Invalid Thread count as argument (Must be [4-28])";
		exit(0);
	}
}

void Settings::processArgument(std::string arg)
{
	if (arg.rfind("-p", 0) == 0)
		mFindPortNumber(arg.substr(2));
	else if (arg.rfind("-t", 0) == 0)
		mFindThreadCount(arg.substr(2));
	else if (arg.rfind("-c", 0) == 0)
		mFindccmPortNumber(arg.substr(2));
	else
	{
		std::cerr << "Invalid argument";
		exit(0);
	}
}

std::string Settings::generateStatus()
{
	std::string statusStr;
	statusStr = std::to_string(RTDS_MAJOR) + "." + std::to_string(RTDS_MINOR) + "." + std::to_string(RTDS_PATCH) + "\t";
	statusStr += std::to_string(mRTDSportNo) + "\t";
	statusStr += std::to_string(mRTDSccmPortNo) + "\t";
	return statusStr;
}
