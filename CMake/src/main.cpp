#include "rtds.h"
#include <iostream>
#include "common.h"
#include "rtds_settings.h"

void processCommand(RTDS& rtdsServer, std::string command)
{
	if (command == "exit")
	{
		std::cout << "RTDS : RTDS exiting";
		exit(0);
	}
	else if (command == "stop")
	{
		rtdsServer.stopServer();
		std::cout << "RTDS : RTDS stopped" << std::endl;
	}
	else if (command == "status")
	{
		rtdsServer.printStatus();
	}
	else if (command == "start")
	{
		if (rtdsServer.isActive())
			std::cout << "RTDS : RTDS is already running" << std::endl;
		else
		{
			rtdsServer.startServer();
			std::cout << "RTDS : RTDS started" << std::endl;
		}
	}
	else if (command == "version")
	{
		std::cout << "RTDS : Version " << RTDS_MAJOR << "." << RTDS_MINOR
			<< "." << RTDS_PATCH << std::endl;
	}
	else if (command == "reset")
	{
		RESET_ERROR_COUNTER
		std::cout << "RTDS : RTDS error counters reset OK" << std::endl;
	}
	else
		std::cout << "RTDS : Invalid command !!" << std::endl;
}

int main(int argCount, const char* args[])
{
	for (auto i = 1; i < argCount; i++)
	{
		auto argument = std::string(args[i]);
		Settings::processArgument(argument);
	}

	RTDS rtdsServer(RTDS_PORT, RTDS_START_THREAD);
	rtdsServer.startServer();

	while (true)
	{
		std::string command;
		std::cout << "RTDS : ";
		std::cin >> command;
		processCommand(rtdsServer, command);
	}
	return 0;
}