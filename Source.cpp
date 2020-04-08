#include "RTDS.h"
#include "Log.h"

int main()
{
	#ifdef PRINT_LOG
	Log::startLog("log.txt");
	#endif
	{
		RTDS RTDSserver(500);
		RTDSserver.addThread(5);
		RTDSserver.startTCPserver();


		std::this_thread::sleep_for(std::chrono::milliseconds(20000));
	}
	#ifdef PRINT_LOG
	Log::stopLog();
	#endif
	return 0;
}


