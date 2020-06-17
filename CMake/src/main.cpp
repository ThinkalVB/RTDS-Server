#include "rtds.h"
#include <iostream>
#include "cmd_processor.h"

unsigned short portNumber = RTDS_PORT;
short threadCount = DEF_THREAD_COUNT;

void findPortNumber(std::string portNStr)
{
	if (!CmdProcessor::isPortNumber(portNStr, portNumber))
	{
		std::cerr << "Invalid Port Number as argument";
		exit(0);
	}
}

void findThreadCount(std::string threadCStr)
{
	if (!CmdProcessor::isThreadCount(threadCStr, threadCount))
	{
		std::cerr << "Invalid Thread count as argument (Must be [1-28])";
		exit(0);
	}
}

void processArgs(std::string arg)
{
	if (arg.rfind("-p", 0) == 0)
		findPortNumber(arg.substr(2));
	else if (arg.rfind("-t", 0) == 0)
		findThreadCount(arg.substr(2));
	else
	{
		std::cerr << "Invalid argument";
		exit(0);
	}
}

void processCommand(RTDS& rtdsServer, std::string command)
{
	if (command == "exit")
	{
		rtdsServer.stopAccepting();
		rtdsServer.stopTCPserver();
		exit(0);
	}
}

int main(int argCount, const char* args[])
{
	for (auto i = 1; i < argCount; i++)
	{
		auto argument = std::string(args[i]);
		processArgs(argument);
	}

	RTDS rtdsServer(portNumber);
	rtdsServer.startTCPserver(threadCount);

	while (true)
	{
		std::string command;
		std::cin >> command;
		processCommand(rtdsServer, command);
	}
}