#include "RTDS.h"
#include "Log.h"

int main()
{
	#ifdef PRINT_LOG
	Log::startLog("log.txt");
	#endif
	{
		RTDS RTDSserver;
		RTDSserver.addThread();
		RTDSserver.startTCPserver();
		RTDSserver.startTCPaccepting();


		std::this_thread::sleep_for(std::chrono::milliseconds(20000));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	#ifdef PRINT_LOG
	Log::stopLog();
	#endif
	return 0;
}


