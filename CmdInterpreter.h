#pragma once
#include "Log.h"
#include "Peer.h"

class CmdInterpreter
{
	static void ping(Peer&);
public:
	static void processCommand(Peer&);


	static void makeSourcePairV4(asio::ip::tcp::endpoint&, unsigned short, unsigned char(&sourcePair)[18]);
	static void makeSourcePairV6(asio::ip::tcp::endpoint&, unsigned short, unsigned char(&sourcePair)[18]);
	static bool validIPaddress(std::string, unsigned short = 0);
};

