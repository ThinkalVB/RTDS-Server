#include "rtds_settings.h"
#include "cmd_processor.h"
#include <iostream>

int Error::mWarnings = 0;
int Error::mError_memmory = 0;
int Error::mError_socket = 0;
int Error::mError_io = 0;
int Error::mError_code = 0;

unsigned short Settings::mRtdsPortNo = RDTS_DEF_PORT;
short Settings::mRtdsThreadCount = MIN_THREAD_COUNT;

void Settings::mFindPortNumber(std::string portNStr)
{
	if (!CmdProcessor::isPortNumber(portNStr, mRtdsPortNo))
	{
		std::cerr << "Invalid Port Number as argument";
		exit(0);
	}
}

void Settings::mFindThreadCount(std::string threadCStr)
{
	if (!CmdProcessor::isThreadCount(threadCStr, mRtdsThreadCount))
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
	else
	{
		std::cerr << "Invalid argument";
		exit(0);
	}
}

void Error::mResetErrorCounts()
{
	mWarnings = 0;
	mError_memmory = 0;
	mError_socket = 0;
	mError_io = 0;
	mError_code = 0;
}
