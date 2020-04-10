#pragma once
#include "Peer.h"

class CmdInterpreter
{
	static void ping(Peer&);
public:
	static void processCommand(Peer&);

	static void makeSourcePairV4(asio::ip::tcp::endpoint&, unsigned short, uint8_t (&sourcePair)[18]);
	static void makeSourcePairV6(asio::ip::tcp::endpoint&, unsigned short, uint8_t (&sourcePair)[18]);
	static bool validIPaddress(std::string, unsigned short = 0);
};

template <typename T>
void byteSwap(T& portNumber)
{
	char* startIndex = static_cast<char*>((void*)&portNumber);
	char* endIndex = startIndex + sizeof(T);
	std::reverse(startIndex, endIndex);
}