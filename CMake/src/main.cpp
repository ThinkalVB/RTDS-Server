#include "rtds.h"

int main()
{
	RTDS rtdsServer;
	rtdsServer.startTCPserver();
	rtdsServer.addThisThread();
}