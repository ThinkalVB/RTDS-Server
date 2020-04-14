#pragma once
#include "peer.h"
#include "common.hpp"

class CmdInterpreter
{
	static void ping(Peer&);
public:
	static void processCommand(Peer&);

	static void makeSourcePair(const asio::ip::address_v4&, unsigned short, sourcePairV4&);
	static void makeSourcePair(const asio::ip::address_v6&, unsigned short, sourcePairV6&);
};

template <typename T>
void byteSwap(T& portNumber)
{
	char* startIndex = static_cast<char*>((void*)&portNumber);
	char* endIndex = startIndex + sizeof(T);
	std::reverse(startIndex, endIndex);
}