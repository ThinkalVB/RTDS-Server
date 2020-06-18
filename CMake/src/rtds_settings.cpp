#include "rtds_settings.h"
#include "cmd_processor.h"
#include <iostream>

int Error::m_warnings = 0;
int Error::m_error_memmory = 0;
int Error::m_error_socket = 0;
int Error::m_error_io = 0;
int Error::m_error_code = 0;

unsigned short Settings::m_rtdsPortNo = 389;
short Settings::m_rtdsThreadCount = 4;

void Settings::m_findPortNumber(std::string portNStr)
{
	if (!CmdProcessor::isPortNumber(portNStr, m_rtdsPortNo))
	{
		std::cerr << "Invalid Port Number as argument";
		exit(0);
	}
}

void Settings::m_findThreadCount(std::string threadCStr)
{
	if (!CmdProcessor::isThreadCount(threadCStr, m_rtdsThreadCount))
	{
		std::cerr << "Invalid Thread count as argument (Must be [1-28])";
		exit(0);
	}
}

void Settings::processArgument(std::string arg)
{
	if (arg.rfind("-p", 0) == 0)
		m_findPortNumber(arg.substr(2));
	else if (arg.rfind("-t", 0) == 0)
		m_findThreadCount(arg.substr(2));
	else
	{
		std::cerr << "Invalid argument";
		exit(0);
	}
}
