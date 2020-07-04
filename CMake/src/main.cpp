#include "rtds.h"
#include <thread>
#include "rtds_settings.h"

int main(int argCount, const char* args[])
{
	for (auto i = 1; i < argCount; i++)
	{
		auto argument = std::string(args[i]);
		Settings::processArgument(argument);
	}

	RTDS rtdsServer(RTDS_PORT, RTDS_CCM, RTDS_START_THREAD);
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (NEED_TO_ABORT)
			break;
	}
	return 0;
}