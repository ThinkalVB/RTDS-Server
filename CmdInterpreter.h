#pragma once
#include "Peer.h"
#include "Enums.h"

class CmdInterpreter
{
	static void ping(Peer&);
public:
	static void processCommand(Peer&);

	static void makeSourcePairV4(const asio::ip::address_v4&, unsigned short, uint8_t(&sourcePair)[6]);
	static void makeSourcePairV6(const asio::ip::address_v6&, unsigned short, uint8_t(&sourcePair)[18]);
	static bool validIPaddress(std::string, unsigned short = 0);
};

template <typename T>
void byteSwap(T& portNumber)
{
	char* startIndex = static_cast<char*>((void*)&portNumber);
	char* endIndex = startIndex + sizeof(T);
	std::reverse(startIndex, endIndex);
}