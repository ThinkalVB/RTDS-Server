#include "rtds.h"
#include "log.h"

int main()
{
	RTDS RTDSserver;
	RTDSserver.addThread(5);
	RTDSserver.startTCPserver();
	RTDSserver.addThisThread();

	return 0;
}