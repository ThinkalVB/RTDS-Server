#include "rtds.h"

int main()
{
	RTDS RTDSserver;
	RTDSserver.addThread(5);
	RTDSserver.startTCPserver();
	RTDSserver.addThisThread();
	return 0;
}