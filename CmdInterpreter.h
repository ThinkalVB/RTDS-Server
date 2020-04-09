#pragma once
#include "Log.h"
#include "Peer.h"

class CmdInterpreter
{
	static void ping(Peer&);
public:
	static void processCommand(Peer&);



};

