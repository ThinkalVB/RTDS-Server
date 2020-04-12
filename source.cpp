#include "rtds.h"
#include "log.h"

int main()
{
	#ifdef PRINT_LOG
	Log::startLog("logs.txt");
	#endif
	{
		RTDS RTDSserver;
		RTDSserver.addThread(5);
		RTDSserver.startTCPserver();
		RTDSserver.stopTCPserver();
		RTDSserver.startTCPserver();


		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20000));
		}
	}
	#ifdef PRINT_LOG
	Log::stopLog();
	#endif
	return 0;
}
